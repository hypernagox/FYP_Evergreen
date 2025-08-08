#include "pch.h"
#include "scene.h"
#include "light_directional.h"
#include "shadow_map.h"
#include "screen_space_ao.h"
#include "renderer_base.h"
#include "frame_resource.h"
#include "scene_object.h"
#include "transform.h"
#include "time_measure.h"
#include "camera.h"
#include "shader.h"
#include "core.h"
#include "input.h"
#include "deferred_renderer.h"
#include "motion_blur.h"
#include "post_process_bloom.h"
#include "post_process_fxaa.h"
#include "post_process_outline.h"
#include "gui_element.h"
#include "debug_console.h"
#include "audio.h"

namespace udsdx
{
	extern unsigned long long g_localMatrixRecalculateCounter;
	extern unsigned long long g_worldMatrixRecalculateCounter;

	Scene::Scene()
	{
		m_rootObject = SceneObject::MakeShared();
		m_rootObject->m_sceneRoot = this;
		m_rootObjectSub = SceneObject::MakeShared();
	}

	Scene::~Scene()
	{

	}

	void Scene::OnAttach()
	{
	}

	void Scene::Update(const Time& time)
	{ ZoneScoped;
	    UpdateGUIElementEvent(time);
		SceneObject::Enumerate(m_rootObjectSub, [&time, this](const std::shared_ptr<SceneObject>& sceneObject) { sceneObject->Update(time, *this); });
	}

	void Scene::PostUpdate(const Time& time)
	{
		m_renderCameraQueue.clear();
		m_renderLightQueue.clear();
		for (auto& queue : m_renderObjectQueues)
		{
			queue.clear();
		}
		m_renderShadowObjectQueue.clear();
		m_renderGUIObjectQueue.clear();

		SceneObject::Enumerate(m_rootObjectSub, [&time, this](const std::shared_ptr<SceneObject>& sceneObject) { sceneObject->PostUpdate(time, *this); });
		SceneObject::Enumerate(m_rootObjectSub, [&time, this](const std::shared_ptr<SceneObject>& sceneObject) { sceneObject->GetTransform()->ValidateSRTMatrices(); });
	}

	void Scene::OnDrawGizmos()
	{
		const char* className = typeid(*this).name();

		ImGui::Begin(className);

		unsigned int activeObjectsCount = 0;
		Camera* renderCamera = m_renderCameraQueue[0];
		SceneObject::Enumerate(m_rootObjectSub, [&activeObjectsCount, renderCamera](const std::shared_ptr<SceneObject>& object) {
			activeObjectsCount++;
			object->OnDrawGizmos(renderCamera);
			});
		
		// Add Category for draw calls
		ImGui::SetNextItemOpen(true, ImGuiCond_Once);
		ImGui::Text("Active Objects Count: %u", activeObjectsCount);
		ImGui::Text("Local Matrix Recalculation Count: %zu", g_localMatrixRecalculateCounter);
		ImGui::Text("World Matrix Recalculation Count: %zu", g_worldMatrixRecalculateCounter);
		g_localMatrixRecalculateCounter = 0;
		g_worldMatrixRecalculateCounter = 0;
		if (ImGui::TreeNode("Draw Calls"))
		{
			ImGui::Text("Render Camera Count: %zu", m_renderCameraQueue.size());
			ImGui::Text("Render Light Count: %zu", m_renderLightQueue.size());
			ImGui::Text("Render Shadow Object Count: %zu", m_renderShadowObjectQueue.size());

			for (size_t i = 0; i < m_renderObjectQueues.size(); ++i)
			{
				const auto& group = m_renderObjectQueues[i];
				ImGui::Text("Render Group %zu:", i);
				for (const auto& [defferedPipelineState, objectGroups] : group)
				{
					ImGui::Text("  Deferred Pipeline State: %p", defferedPipelineState);
					for (const auto& [pipelineState, objects] : objectGroups)
					{
						ImGui::Text("    Object Count: %zu", objects.size());
					}
				}
			}
			ImGui::TreePop();
		}

		Matrix4x4 viewMatrix = renderCamera->GetViewMatrix(false);

		Vector3 viewForward = Vector3::TransformNormal(Vector3::UnitZ, viewMatrix);
		Vector3 viewUp = Vector3::TransformNormal(Vector3::UnitY, viewMatrix);
		Vector3 viewRight = viewUp.Cross(viewForward);

		std::array<std::pair<Vector3, ImColor>, 3> lines = {
			std::make_pair(viewRight,	ImColor(1.0f, 0.0f, 0.0f, 1.0f)),		// Red for right
			std::make_pair(viewUp,		ImColor(0.0f, 1.0f, 0.0f, 1.0f)),		// Green for up
			std::make_pair(viewForward, ImColor(0.0f, 0.0f, 1.0f, 1.0f))		// Blue for forward
		};
		std::sort(lines.begin(), lines.end(), [](const auto& a, const auto& b) { return a.first.z > b.first.z; });

		float lineLength = 40.0f;
		float lineThickness = 4.0f;
		ImVec2 screenPosition = ImVec2(ImGui::GetIO().DisplaySize.x - 50.0f, 50.0f);
		ImDrawList* drawList = ImGui::GetBackgroundDrawList();
		for (const auto& line : lines)
		{
			drawList->AddLine(screenPosition,
				screenPosition + ImVec2(line.first.x, -line.first.y) * lineLength,
				line.second, lineThickness);
		}

		const char* text = "View Gizmos";
		ImVec2 textSize = ImGui::CalcTextSize(text);
		drawList->AddText(screenPosition - textSize * 0.5f + ImVec2(0.0f, 50.0f), ImColor(1.0f, 1.0f, 1.0f, 1.0f), text);

		ImGui::End();
	}

	void Scene::Render(RenderParam& param)
	{ ZoneScoped;
		std::vector<D3D12_GPU_VIRTUAL_ADDRESS> cameraCbvs(m_renderCameraQueue.size());
		for (size_t i = 0; i < m_renderCameraQueue.size(); ++i)
		{
			cameraCbvs[i] = m_renderCameraQueue[i]->UpdateConstantBuffer(param.FrameResourceIndex, param.Viewport.Width, param.Viewport.Height);
		}

		param.CommandList->SetGraphicsRootSignature(param.RootSignature);

		if (!m_renderCameraQueue.empty())
		{
			Transform* listenerTransform = m_renderCameraQueue[0]->GetTransform();
			INSTANCE(Audio)->UpdateAudioListener(listenerTransform->GetWorldPosition(), listenerTransform->GetWorldRotation());
		}

		// Shadow map rendering pass
		if (!m_renderLightQueue.empty() && !m_renderCameraQueue.empty())
		{
			param.CommandList->SetGraphicsRootConstantBufferView(RootParam::PerCameraCBV, cameraCbvs[0]);
			PassRenderShadow(param, m_renderCameraQueue.front(), m_renderLightQueue[0]);
		}

		for (size_t i = 0; i < m_renderCameraQueue.size(); ++i)
		{
			param.TargetCamera = m_renderCameraQueue[i];
			param.CommandList->SetGraphicsRootConstantBufferView(RootParam::PerCameraCBV, cameraCbvs[i]);
			PassRenderMain(param, m_renderCameraQueue[i], cameraCbvs[i]);
		}

		PassRenderHUD(param);
	}

	void Scene::OnDetach()
	{
	}

	void Scene::UpdateGUIElementEvent(const Time& time)
	{
		auto elements = m_rootObjectSub->GetComponentsInChildren<GUIElement>();
		int mx = INSTANCE(Input)->GetMouseX();
		int my = INSTANCE(Input)->GetMouseY();

		GUIElement* hoveredElement = nullptr;
		for (auto iter = elements.rbegin(); iter != elements.rend(); ++iter)
		{
			GUIElement* element = *iter;
			if (hoveredElement == nullptr && element->GetRaycastTarget())
			{
				RECT rect = element->GetScreenRect();
				if (rect.left <= mx && mx <= rect.right && rect.top <= my && my <= rect.bottom)
				{
					hoveredElement = element;
				}
			}
			element->UpdateEvent(element == hoveredElement);
		}
	}

	void Scene::AddObject(std::shared_ptr<SceneObject> object)
	{
		m_rootObjectSub->AddChild(object);
	}

	void Scene::HandleAttach()
	{
		OnAttach();

		m_rootObject->AddChild(m_rootObjectSub);
	}

	void Scene::HandleDetach()
	{
		OnDetach();

		m_rootObjectSub->RemoveFromParent();
	}

	void Scene::EnqueueRenderCamera(Camera* camera)
	{
		m_renderCameraQueue.emplace_back(camera);
	}

	void Scene::EnqueueRenderLight(LightDirectional* light)
	{
		m_renderLightQueue.emplace_back(light);
	}

	void Scene::EnqueueRenderObject(RendererBase* object, RenderGroup group, ID3D12PipelineState* pipelineState, ID3D12PipelineState* deferredPipelineState, int parameter)
	{
		m_renderObjectQueues[group][deferredPipelineState][pipelineState].emplace_back(object, parameter);
	}

	void Scene::EnqueueRenderShadowObject(RendererBase* object, ID3D12PipelineState* pipelineState, int parameter)
	{
		m_renderShadowObjectQueue[pipelineState].emplace_back(object, parameter);
	}

	void Scene::EnqueueRenderGUIObject(GUIElement* object)
	{
		m_renderGUIObjectQueue.emplace_back(object);
	}

	void Scene::PassRenderShadow(RenderParam& param, Camera* camera, LightDirectional* light)
	{
		ZoneScopedN("Shadow Render Pass");
		TracyD3D12Zone(*param.TracyQueueContext, param.CommandList, "Shadow Render Pass");
		param.RenderShadowMap->Pass(param, this, camera, light);
	}

	void Scene::PassRenderSSAO(RenderParam& param, Camera* camera)
	{
		ZoneScopedN("SSAO Render Pass");
		TracyD3D12Zone(*param.TracyQueueContext, param.CommandList, "SSAO Render Pass");
		if (param.RenderOptions->DrawSSAO)
		{
			param.RenderScreenSpaceAO->UpdateSSAOConstants(param, camera);
			param.RenderScreenSpaceAO->PassSSAO(param);
			param.RenderScreenSpaceAO->PassBlur(param);
		}
	}

	void Scene::PassRenderMain(RenderParam& param, Camera* camera, D3D12_GPU_VIRTUAL_ADDRESS cameraCbv)
	{
		ZoneScopedN("Main Pass");
		TracyD3D12Zone(*param.TracyQueueContext, param.CommandList, "Main Pass");

		auto pCommandList = param.CommandList;

		std::vector<ID3D12PipelineState*> defferedPipelineStates;
		for (const auto& [defferedPipelineState, objectGroups] : m_renderObjectQueues[RenderGroup::Deferred])
		{
			defferedPipelineStates.push_back(defferedPipelineState);
		}

		// Deferred rendering pass
		param.Renderer->PassBufferPreparation(param);
		param.Renderer->ClearRenderTargets(pCommandList);

		std::unique_ptr<BoundingCamera> boundingCamera = camera->GetViewFrustumWorld(param.AspectRatio);
		param.ViewFrustumWorld = boundingCamera.get();

		RenderSceneObjects(param, RenderGroup::Deferred, 1);

		param.Renderer->PassBufferPostProcess(param);

		PassRenderSSAO(param, camera);

		param.Renderer->PassRender(param, cameraCbv, defferedPipelineStates);

		// Forward rendering pass
		param.Renderer->PassBufferPreparation(param);

		pCommandList->SetGraphicsRootSignature(param.RootSignature);
		pCommandList->OMSetRenderTargets(1, &param.Renderer->GetRenderTargetRTVView(), true, &param.Renderer->GetDepthBufferDsv());

		pCommandList->RSSetViewports(1, &param.Viewport);
		pCommandList->RSSetScissorRects(1, &param.ScissorRect);

		pCommandList->SetGraphicsRootConstantBufferView(RootParam::PerCameraCBV, cameraCbv);
		RenderSceneObjects(param, RenderGroup::Forward, 1);

		param.Renderer->PassBufferPostProcess(param);

		// Bloom pass
		if (param.RenderOptions->DrawBloom)
		{
			param.RenderPostProcessBloom->Pass(param);
		}

		// Motion blur pass
		if (param.RenderOptions->DrawMotionBlur)
		{
			param.RenderMotionBlur->Pass(param, cameraCbv);
		}

		// FXAA pass
		if (param.RenderOptions->DrawFXAA)
		{
			param.RenderPostProcessFXAA->Pass(param);
		}

		// Post-process outline pass
		if (param.RenderOptions->DrawOutline)
		{
			param.RenderPostProcessOutline->Pass(param);
		}
	}

	void Scene::PassRenderHUD(RenderParam& param)
	{
		param.SpriteBatchNonPremultipliedAlpha->SetViewport(param.Viewport);
		param.SpriteBatchPreMultipliedAlpha->SetViewport(param.Viewport);
		param.SpriteBatchNonPremultipliedAlpha->Begin(param.CommandList);
		//param.SpriteBatchPreMultipliedAlpha->Begin(param.CommandList);

		RenderGUIObjects(param, 1);

		param.SpriteBatchNonPremultipliedAlpha->End();
		//param.SpriteBatchPreMultipliedAlpha->End();
	}

	void Scene::RenderShadowSceneObjects(RenderParam& param, int instances)
	{
		for (const auto& [pipelineState, objects] : m_renderShadowObjectQueue)
		{
			param.CommandList->SetPipelineState(pipelineState);
			for (const auto& [object, parameter] : objects)
			{
				object->ValidateTransformCache();
				object->Render(param, parameter);
			}
		}
		param.RenderStageIndex++;
	}
	
	void Scene::RenderSceneObjects(RenderParam& param, RenderGroup group, int instances)
	{
		UINT pipelineCount = 0;
		for (const auto& [defferedPipelineState, objectGroups] : m_renderObjectQueues[group])
		{
			if (pipelineCount >= 128)
			{
				DebugConsole::LogWarning("Too many deffered pipeline states in render stage: " + std::to_string(group) + ". Limit is 128.");
				break;
			}

			for (const auto& [pipelineState, objects] : objectGroups)
			{
				param.CommandList->SetPipelineState(pipelineState);

				for (const auto& [object, parameter] : objects)
				{
					param.CommandList->OMSetStencilRef(pipelineCount | (static_cast<UINT>(object->GetDrawOutline()) << 7));
					object->ValidateTransformCache();
					object->Render(param, parameter);
				}
			}

			pipelineCount++;
		}

		param.RenderStageIndex++;
	}

	void Scene::RenderGUIObjects(RenderParam& param, int instances)
	{
		for (const auto& object : m_renderGUIObjectQueue)
		{
			object->Render(param);
		}
	}
}