#include "pch.h"
#include "scene_object.h"
#include "transform.h"
#include "component.h"
#include "camera.h"
#include "core.h"

namespace udsdx
{
	unsigned long long g_sceneObjectCount = 0ULL;

	int SceneObject::GarbageCollector::m_currentFrameResourceIndex = 1;
	std::array<std::vector<SceneObject*>, FrameResourceCount + 1> SceneObject::GarbageCollector::m_garbage;

	void SceneObject::GarbageCollector::Stash(SceneObject* object)
	{
		m_garbage[m_currentFrameResourceIndex].emplace_back(object);
	}

	void SceneObject::GarbageCollector::Collect(int frameResourceIndex)
	{
		m_currentFrameResourceIndex = FrameResourceCount;
		for (SceneObject* object : m_garbage[frameResourceIndex])
		{
			// Caution: This operation will stash other objects into the garbage collector
			delete object;
		}
		m_garbage[frameResourceIndex] = m_garbage[FrameResourceCount];
		m_garbage[FrameResourceCount].clear();
		m_currentFrameResourceIndex = frameResourceIndex;
	}

	void SceneObject::Enumerate(const std::shared_ptr<SceneObject>& root, std::function<void(const std::shared_ptr<SceneObject>&)> callback, bool onlyActive)
	{
		static std::stack<std::shared_ptr<SceneObject>> s;
		size_t sBase = s.size();
		std::shared_ptr<SceneObject> node = root->m_child;
		if (!onlyActive || root->m_active)
		{
			callback(root);

			// Perform in-order traversal (sibiling-node-child)
			// Since the order of the siblings is reversed, it needs to visit the siblings first
			while (s.size() > sBase || node != nullptr)
			{
				if (node != nullptr)
				{
					s.emplace(node);
					node = node->m_sibling;
				}
				else
				{
					// node is guaranteed to have an instance
					node = s.top();
					s.pop();
					if (!onlyActive || node->m_active)
					{
						callback(node);
						node = node->m_child;
					}
					else
					{
						node = nullptr;
					}
				}
			}
		}
	}

	std::shared_ptr<SceneObject> SceneObject::MakeShared()
	{
		return std::shared_ptr<SceneObject>(new SceneObject(), SceneObjectDeleter());
	}

	SceneObject::SceneObject()
	{
		++g_sceneObjectCount;
	}

	SceneObject::~SceneObject()
	{
		--g_sceneObjectCount;
	}

	Transform* SceneObject::GetTransform()
	{
		return &m_transform;
	}

	void SceneObject::RemoveAllComponents()
	{
		decltype(m_components) componentsToDelete;
		componentsToDelete.swap(m_components);

		bool activeInScene = GetActiveInScene();
		bool attachedInScene = GetAttachedInScene();

		if (activeInScene)
		{
			for (auto& component : componentsToDelete)
			{
				if (component->GetActive())
				{
					component->OnInactive();
				}
			}
		}
		if (attachedInScene)
		{
			for (auto& component : componentsToDelete)
			{
				component->OnDetach();
			}
		}
	}

	void SceneObject::Update(const Time& time, Scene& scene)
	{
		// Update components
		for (const auto& component : m_components)
		{
			if (component->GetActive())
			{
				if (component->m_isBegin)
				{
					component->Begin();
					component->m_isBegin = false;
				}
				component->Update(time, scene);
			}
		}
	}

	void SceneObject::PostUpdate(const Time& time, Scene& scene)
	{
		// Update components
		for (const auto& component : m_components)
		{
			if (component->GetActive())
			{
				if (component->m_isBegin)
				{
					component->Begin();
					component->m_isBegin = false;
				}
				component->PostUpdate(time, scene);
			}
		}
	}

	void SceneObject::OnDrawGizmos(const Camera* target)
	{
		for (const auto& component : m_components)
		{
			if (component->GetActive())
			{
				component->OnDrawGizmos(target);
			}
		}

		ImVec2 screenSize = ImGui::GetIO().DisplaySize;
		const float lineLength = 25.0f;
		const float lineThickness = 4.0f;

		float screenRatio = screenSize.x / screenSize.y;
		Matrix4x4 viewMatrix = target->GetViewMatrix(false);
		Matrix4x4 projMatrix = target->GetProjMatrix(screenRatio);
		Vector3 viewForward = Vector3(viewMatrix.m[2][0], viewMatrix.m[2][1], viewMatrix.m[2][2]);

		// Draw transform gizmos
		Vector3 worldPosition = m_transform.GetWorldPosition();
		Quaternion worldRotation = m_transform.GetWorldRotation();
		Vector3 viewPosition = target->ToViewPosition(worldPosition);

		ImVec2 cursorPos = ImGui::GetCursorScreenPos();
		std::string nodeID = std::to_string(reinterpret_cast<unsigned long long>(this));
		if (ImGui::TreeNode(nodeID.c_str(), "Scene Object (%zu Components) (%.1f, %.1f, %.1f)", m_components.size(), worldPosition.x, worldPosition.y, worldPosition.z))
		{
			// Draw position text
			Vector3 localPosition = m_transform.GetLocalPosition();
			Vector3 localRotationEuler = m_transform.GetLocalRotation().ToEuler() * RAD2DEG;
			Vector3 localScale = m_transform.GetLocalScale();

			ImGui::Text(std::format("Local Position: ({:.2f}, {:.2f}, {:.2f})", localPosition.x, localPosition.y, localPosition.z).c_str());
			ImGui::Text(std::format("Local Rotation: ({:.2f}, {:.2f}, {:.2f})", localRotationEuler.x, localRotationEuler.y, localRotationEuler.z).c_str());
			ImGui::Text(std::format("Local Scale: ({:.2f}, {:.2f}, {:.2f})", localScale.x, localScale.y, localScale.z).c_str());

			ImGui::SetNextItemOpen(true, ImGuiCond_Once);
			if (ImGui::TreeNode((nodeID + "_c").c_str(), "Components"))
			{
				for (const auto& component : m_components)
				{
					ImGui::Text("%s", typeid(*component).name());
				}
				ImGui::TreePop();
			}
			ImGui::TreePop();
		}

		// If the object is in front of the camera, draw a red circle at the position
		if (viewPosition.z > 1e-2f)
		{
			float worldLength = (lineLength / screenSize.y) * (2.0f * viewPosition.z / projMatrix.m[1][1]);

			Vector3 worldForward = Vector3::Transform(Vector3::UnitZ, worldRotation);
			Vector3 worldUp = Vector3::Transform(Vector3::UnitY, worldRotation);
			Vector3 worldRight = worldUp.Cross(worldForward);

			Vector2 screenPosition = target->ToScreenPosition(worldPosition);
			Vector2 screenRight = target->ToScreenPosition(worldPosition + worldRight * worldLength);
			Vector2 screenUp = target->ToScreenPosition(worldPosition + worldUp * worldLength);
			Vector2 screenForward = target->ToScreenPosition(worldPosition + worldForward * worldLength);

			std::array<std::tuple<float, Vector2, ImColor>, 3> lines = {
				std::make_tuple(viewForward.Dot(worldRight), screenRight, ImColor(1.0f, 0.0f, 0.0f, 1.0f)),	// Red for right
				std::make_tuple(viewForward.Dot(worldUp), screenUp, ImColor(0.0f, 1.0f, 0.0f, 1.0f)),		// Green for up
				std::make_tuple(viewForward.Dot(worldForward), screenForward, ImColor(0.0f, 0.0f, 1.0f, 1.0f))  // Blue for forward
			};

			std::sort(lines.begin(), lines.end(), [](const auto& a, const auto& b) { return std::get<0>(a) > std::get<0>(b); });

			ImDrawList* drawList = ImGui::GetBackgroundDrawList();
			for (const auto& [distance, screenPos, color] : lines)
			{
				drawList->AddLine(ImVec2(screenPosition.x, screenPosition.y), ImVec2(screenPos.x, screenPos.y), color, lineThickness);
			}
			drawList->AddCircleFilled(ImVec2(screenPosition.x, screenPosition.y), lineThickness, ImColor(1.0f, 1.0f, 1.0f, 1.0f));

			ImVec2 cursorWinPos = cursorPos - ImGui::GetWindowPos();
			float regionHeight = ImGui::GetWindowHeight();

			if (screenPosition.x >= 0 && screenPosition.x <= screenSize.x && screenPosition.y >= 0 && screenPosition.y <= screenSize.y)
			{
				if (cursorWinPos.y >= 0.0f && cursorWinPos.y + ImGui::GetTextLineHeight() <= regionHeight)
				{
					ImVec2 beginPos = ImVec2(cursorPos.x - 20.0f, cursorPos.y + ImGui::GetTextLineHeight() * 0.5f);
					ImVec2 endPos = ImVec2(screenPosition.x, screenPosition.y);

					drawList->AddCircleFilled(beginPos, 4.0f, ImColor(1.0f, 1.0f, 1.0f, 1.0f));
					if (std::abs(endPos.x - beginPos.x) < 50.0f)
					{
						drawList->AddLine(beginPos, endPos, ImColor(1.0f, 1.0f, 1.0f, 1.0f));
					}
					else
					{
						float sign = (endPos.x - beginPos.x < 0.0f) ? -1.0f : 1.0f;
						ImVec2 midPos1 = ImVec2((endPos.x + beginPos.x - 50.0f * sign) * 0.5f, beginPos.y);
						ImVec2 midPos2 = ImVec2((endPos.x + beginPos.x + 50.0f * sign) * 0.5f, endPos.y);

						drawList->AddLine(beginPos, midPos1, ImColor(1.0f, 1.0f, 1.0f, 1.0f));
						drawList->AddLine(midPos1, midPos2, ImColor(1.0f, 1.0f, 1.0f, 1.0f));
						drawList->AddLine(midPos2, endPos, ImColor(1.0f, 1.0f, 1.0f, 1.0f));
					}
				}
			}
		}
	}

	void SceneObject::AddChild(std::shared_ptr<SceneObject> child)
	{
		if (m_child != nullptr)
		{
			m_child->m_back = child.get();
		}

		child->m_sibling = m_child;
		child->m_parent = this;
		child->m_back = this;
		m_child = child;

		m_transform.m_children.emplace_back(&child->m_transform);
		child->m_transform.m_parent = &m_transform;

		std::vector<Component*> componentsToAttach;
		std::vector<Component*> componentsToActive;

		if (child->GetAttachedInScene())
		{
			Enumerate(child, [&componentsToAttach](const auto& object) {
				for (auto& component : object->m_components)
				{
					componentsToAttach.emplace_back(component.get());
				}
			}, false);
		}
		if (child->GetActiveInScene())
		{
			Enumerate(child, [&componentsToActive](const auto& object) {
				for (auto& component : object->m_components)
				{
					if (component->GetActive())
					{
						componentsToActive.emplace_back(component.get());
					}
				}
			}, true);
		}

		for (auto& component : componentsToAttach)
		{
			component->OnAttach();
		}
		for (auto& component : componentsToActive)
		{
			component->OnActive();
		}
	}

	void SceneObject::RemoveFromParent()
	{
		if (m_parent == nullptr)
		{
			return;
		}

		std::vector<Component*> componentsToDetach;
		std::vector<Component*> componentsToInactive;

		if (GetAttachedInScene())
		{
			Enumerate(shared_from_this(), [&componentsToDetach](const auto& object) {
				for (auto& component : object->m_components)
				{
					componentsToDetach.emplace_back(component.get());
				}
				}, false);
		}
		if (GetActiveInScene())
		{
			Enumerate(shared_from_this(), [&componentsToInactive](const auto& object) {
				for (auto& component : object->m_components)
				{
					if (component->GetActive())
					{
						componentsToInactive.emplace_back(component.get());
					}
				}
				}, true);
		}

		m_transform.m_parent = nullptr;
		if (m_parent != nullptr)
		{
			auto childrenContainer = m_parent->m_transform.m_children;
			auto it = std::find(childrenContainer.begin(), childrenContainer.end(), &m_transform);
			if (it != childrenContainer.end())
			{
				childrenContainer.erase(it);
			}
		}

		if (m_sibling != nullptr)
		{
			m_sibling->m_back = m_back;
		}
		if (m_back->m_sibling.get() == this)
		{
			m_back->m_sibling = m_sibling;
		}
		if (m_back->m_child.get() == this)
		{
			m_back->m_child = m_sibling;
		}

		m_parent = nullptr;
		m_back = nullptr;
		m_sibling = nullptr;

		for (auto& component : componentsToInactive)
		{
			component->OnInactive();
		}
		for (auto& component : componentsToDetach)
		{
			component->OnDetach();
		}
	}

	const SceneObject* SceneObject::GetParent() const
	{
		return m_parent;
	}

	bool SceneObject::GetActive() const
	{
		return m_active;
	}

	bool SceneObject::GetActiveInHierarchy() const
	{
		const SceneObject* node = this;
		while (node->m_parent != nullptr)
		{
			if (!node->m_active)
			{
				return false;
			}
			node = node->m_parent;
		}
		return node->m_active;
	}

	bool SceneObject::GetActiveInScene() const
	{
		const SceneObject* node = this;
		while (node->m_parent != nullptr)
		{
			if (!node->m_active)
			{
				return false;
			}
			node = node->m_parent;
		}
		return node->m_active && node->m_sceneRoot != nullptr;
	}

	Scene* SceneObject::GetScene() const
	{
		const SceneObject* node = this;
		while (node->m_parent != nullptr)
		{
			node = node->m_parent;
		}
		return node->m_sceneRoot;
	}

	bool SceneObject::GetAttachedInScene() const
	{
		return GetScene() != nullptr;
	}

	void SceneObject::SetActive(bool active)
	{
		if (m_active == active)
			return;

		// False -> True transition
		if (active)
		{
			m_active = active;

			if (GetActiveInScene())
			{
				std::vector<Component*> componentsToActive;
				Enumerate(shared_from_this(), [&componentsToActive](const std::shared_ptr<SceneObject>& object) {
					for (auto& component : object->m_components)
					{
						if (component->GetActive())
						{
							componentsToActive.emplace_back(component.get());
						}
					}
				}, true);
				for (auto& component : componentsToActive)
				{
					component->OnActive();
				}
			}
		}
		// True -> False transition
		else
		{
			std::vector<Component*> componentsToInactive;
			if (GetActiveInScene())
			{
				Enumerate(shared_from_this(), [&componentsToInactive](const std::shared_ptr<SceneObject>& object) {
					for (auto& component : object->m_components)
					{
						if (component->GetActive())
						{
							componentsToInactive.emplace_back(component.get());
						}
					}
				}, true);
			}

			m_active = active;
			for (auto& component : componentsToInactive)
			{
				component->OnInactive();
			}
		}
	}

	void SceneObject::SceneObjectDeleter::operator()(SceneObject* object) const
	{
		SceneObject::GarbageCollector::Stash(object);
	}

	void SceneObject::ComponentDeleter::operator()(Component* component) const
	{
		delete component;
	}
}