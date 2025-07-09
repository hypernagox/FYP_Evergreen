#include "pch.h"
#include "scene_object.h"
#include "transform.h"
#include "component.h"
#include "camera.h"
#include "core.h"

namespace udsdx
{
	void SceneObject::Enumerate(const std::shared_ptr<SceneObject>& root, std::function<void(const std::shared_ptr<SceneObject>&)> callback)
	{
		static std::stack<std::shared_ptr<SceneObject>> s;
		size_t sBase = s.size();
		std::shared_ptr<SceneObject> node = root;

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
				if (node->m_active)
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

	void SceneObject::EnumerateUpdate(const std::shared_ptr<SceneObject>& root, const Time& time, Scene& scene)
	{
		Enumerate(root, [&](const std::shared_ptr<SceneObject>& node) { node->Update(time, scene); });
	}

	void SceneObject::EnumeratePostUpdate(const std::shared_ptr<SceneObject>& root, const Time& time, Scene& scene)
	{
		static std::stack<std::pair<std::shared_ptr<SceneObject>, bool>> s;
		size_t sBase = s.size();
		std::pair<std::shared_ptr<SceneObject>, bool> node = std::make_pair(root, false);

		// Perform in-order traversal (sibiling-node-child)
		// Since the order of the siblings is reversed, it needs to visit the siblings first
		while (s.size() > sBase || node.first != nullptr)
		{
			if (node.first != nullptr)
			{
				s.emplace(node);
				node = std::make_pair(node.first->m_sibling, node.second);
			}
			else
			{
				node = s.top();
				s.pop();

				// SceneObject::PostUpdate() returns true if the node is still valid and active
				if (node.first->PostUpdate(time, scene, node.second))
				{
					node = std::make_pair(node.first->m_child, node.second);
				}
				else
				{
					node = std::make_pair(nullptr, node.second);
				}
			}
		}
	}

	SceneObject::SceneObject()
	{

	}

	SceneObject::~SceneObject()
	{

	}

	Transform* SceneObject::GetTransform()
	{
		return &m_transform;
	}

	void SceneObject::RemoveAllComponents()
	{
		m_components.clear();
	}

	void SceneObject::DetachFromHierarchy()
	{
		INSTANCE(Core)->FlushCommandQueue();

		if (m_sibling != nullptr)
		{
			m_sibling->m_parent = m_parent;
		}
		if (m_parent->m_sibling.get() == this)
		{
			m_parent->m_sibling = m_sibling;
		}
		if (m_parent->m_child.get() == this)
		{
			m_parent->m_child = m_sibling;
		}

		m_parent = nullptr;
		m_sibling = nullptr;

		m_transform.SetParent(nullptr);

		m_detachDirty = false;
	}

	void SceneObject::Update(const Time& time, Scene& scene)
	{
		// Update components
		for (auto& component : m_components)
		{
			if (component->GetActive())
			{
				component->Update(time, scene);
			}
		}
	}

	bool SceneObject::PostUpdate(const Time& time, Scene& scene, bool& forceValidate)
	{
		if (m_detachDirty)
		{
			DetachFromHierarchy();
			return false;
		}
		if (!m_active)
		{
			return false;
		}

		// Validate SRT matrix
		m_transform.ValidateSRTMatrices(true);

		// Update components
		for (auto& component : m_components)
		{
			if (component->GetActive())
			{
				component->PostUpdate(time, scene);
			}
		}

		return true;
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
		Matrix4x4 viewMatrix = target->GetViewMatrix();
		Matrix4x4 projMatrix = target->GetProjMatrix(screenRatio);

		// Draw transform gizmos
		Vector3 worldPosition = m_transform.GetWorldPosition();
		Quaternion worldRotation = m_transform.GetWorldRotation();
		Vector3 worldForward = Vector3::Transform(Vector3::UnitZ, worldRotation);
		Vector3 worldUp = Vector3::Transform(Vector3::UnitY, worldRotation);

		Vector3 viewPosition = Vector3::Transform(worldPosition, viewMatrix);
		Vector3 viewForward = Vector3::TransformNormal(worldForward, viewMatrix);
		Vector3 viewUp = Vector3::TransformNormal(worldUp, viewMatrix);
		Vector3 viewRight = viewUp.Cross(viewForward);

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
			ImGui::Text(std::format("Transform Validation State: {}", static_cast<int>(m_transform.m_validationState)).c_str());

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
			Vector3 screenRight = viewPosition + viewRight * worldLength;
			Vector3 screenUp = viewPosition + viewUp * worldLength;
			Vector3 screenForward = viewPosition + viewForward * worldLength;

			Vector3 screenPosition = Vector3::Transform(viewPosition, projMatrix);
			screenRight = Vector3::Transform(screenRight, projMatrix);
			screenUp = Vector3::Transform(screenUp, projMatrix);
			screenForward = Vector3::Transform(screenForward, projMatrix);

			Matrix4x4 screenMatrix = Matrix4x4::Identity;
			screenMatrix.m[0][0] = 0.5f * screenSize.x;
			screenMatrix.m[1][1] = -0.5f * screenSize.y;
			screenMatrix.m[3][0] = 0.5f * screenSize.x;
			screenMatrix.m[3][1] = 0.5f * screenSize.y;

			screenPosition = Vector3::Transform(screenPosition, screenMatrix);
			screenRight = Vector3::Transform(screenRight, screenMatrix);
			screenUp = Vector3::Transform(screenUp, screenMatrix);
			screenForward = Vector3::Transform(screenForward, screenMatrix);

			std::array<std::pair<Vector3, ImColor>, 3> lines = {
				std::make_pair(screenRight, ImColor(1.0f, 0.0f, 0.0f, 1.0f)),	// Red for right
				std::make_pair(screenUp, ImColor(0.0f, 1.0f, 0.0f, 1.0f)),		// Green for up
				std::make_pair(screenForward, ImColor(0.0f, 0.0f, 1.0f, 1.0f))  // Blue for forward
			};

			std::sort(lines.begin(), lines.end(), [](const auto& a, const auto& b) { return a.first.z > b.first.z; });

			ImDrawList* drawList = ImGui::GetBackgroundDrawList();
			for (const auto& line : lines)
			{
				drawList->AddLine(ImVec2(screenPosition.x, screenPosition.y),
					ImVec2(line.first.x, line.first.y),
					line.second, lineThickness);
			}
			drawList->AddCircleFilled(ImVec2(screenPosition.x, screenPosition.y), lineThickness, ImColor(1.0f, 1.0f, 1.0f, 1.0f));

			ImVec2 cursorWinPos = cursorPos - ImGui::GetWindowPos();
			float regionHeight = ImGui::GetWindowHeight();

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

	void SceneObject::AddChild(std::shared_ptr<SceneObject> child)
	{
		if (m_child != nullptr)
		{
			m_child->m_parent = child.get();
		}

		child->m_sibling = m_child;
		child->m_parent = this;
		m_child = child;

		child->m_transform.SetParent(&m_transform);
	}

	void SceneObject::RemoveFromParent()
	{
		m_detachDirty = true;
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
		while (node != nullptr)
		{
			if (!node->m_active)
			{
				return false;
			}
			node = node->m_parent;
		}
		return true;
	}

	void SceneObject::SetActive(bool active)
	{
		m_active = active;
	}

	void SceneObject::ComponentDeleter::operator()(Component* component) const
	{
		delete component;
	}
}