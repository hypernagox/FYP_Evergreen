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
#include "gui_image.h"

namespace udsdx
{
	Scene::Scene()
	{
		m_rootObject = std::make_shared<SceneObject>();
	}

	Scene::~Scene()
	{

	}

	void Scene::Update(const Time& time)
	{ ZoneScoped;
		SceneObject::EnumerateUpdate(m_rootObject, time, *this);
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

		SceneObject::EnumeratePostUpdate(m_rootObject, time, *this);
	}

	void Scene::Render(RenderParam& param)
	{ ZoneScoped;
		std::vector<D3D12_GPU_VIRTUAL_ADDRESS> cameraCbvs(m_renderCameraQueue.size());
		for (size_t i = 0; i < m_renderCameraQueue.size(); ++i)
		{
			cameraCbvs[i] = m_renderCameraQueue[i]->UpdateConstantBuffer(param.FrameResourceIndex, param.Viewport.Width, param.Viewport.Height);
		}

		param.CommandList->SetGraphicsRootSignature(param.RootSignature);

		// Shadow map rendering pass
		if (!m_renderLightQueue.empty() && !m_renderCameraQueue.empty())
		{
			param.CommandList->SetGraphicsRootConstantBufferView(RootParam::PerCameraCBV, cameraCbvs[0]);
			PassRenderShadow(param, m_renderCameraQueue.front(), m_renderLightQueue[0]);
		}

		for (size_t i = 0; i < m_renderCameraQueue.size(); ++i)
		{
			param.CommandList->SetGraphicsRootConstantBufferView(RootParam::PerCameraCBV, cameraCbvs[i]);
			PassRenderMain(param, m_renderCameraQueue[i], cameraCbvs[i]);
		}
	}

	void Scene::AddObject(std::shared_ptr<SceneObject> object)
	{
		m_rootObject->AddChild(object);
	}

	void Scene::EnqueueRenderCamera(Camera* camera)
	{
		m_renderCameraQueue.emplace_back(camera);
	}

	void Scene::EnqueueRenderLight(LightDirectional* light)
	{
		m_renderLightQueue.emplace_back(light);
	}

	void Scene::EnqueueRenderObject(RendererBase* object, RenderGroup group)
	{
		m_renderObjectQueues[group].emplace_back(object);
	}

	void Scene::EnqueueRenderShadowObject(RendererBase* object)
	{
		m_renderShadowObjectQueue.emplace_back(object);
	}

	void Scene::EnqueueRenderGUIObject(GUIImage* object)
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
		param.RenderScreenSpaceAO->UpdateSSAOConstants(param, camera);
		param.RenderScreenSpaceAO->PassSSAO(param);
		param.RenderScreenSpaceAO->PassBlur(param);
	}

	void Scene::PassRenderMain(RenderParam& param, Camera* camera, D3D12_GPU_VIRTUAL_ADDRESS cameraCbv)
	{
		ZoneScopedN("Main Pass");
		TracyD3D12Zone(*param.TracyQueueContext, param.CommandList, "Main Pass");

		auto pCommandList = param.CommandList;

		// Deferred rendering pass
		param.Renderer->PassBufferPreparation(param);
		param.Renderer->ClearRenderTargets(pCommandList);

		std::unique_ptr<BoundingCamera> boundingCamera = camera->GetViewFrustumWorld(param.AspectRatio);
		param.ViewFrustumWorld = boundingCamera.get();

		RenderSceneObjects(param, RenderGroup::Deferred, 1);

		param.Renderer->PassBufferPostProcess(param);

		PassRenderSSAO(param, camera);

		param.Renderer->PassRender(param, cameraCbv);

		// Forward rendering pass
		param.Renderer->PassBufferPreparation(param);

		pCommandList->SetGraphicsRootSignature(param.RootSignature);
		pCommandList->OMSetRenderTargets(1, &param.RenderTargetView, true, &param.Renderer->GetDepthBufferDsv());

		pCommandList->RSSetViewports(1, &param.Viewport);
		pCommandList->RSSetScissorRects(1, &param.ScissorRect);

		pCommandList->SetGraphicsRootConstantBufferView(RootParam::PerCameraCBV, cameraCbv);
		RenderSceneObjects(param, RenderGroup::Forward, 1);

		param.Renderer->PassBufferPostProcess(param);

		// Motion blur pass
		param.RenderMotionBlur->Pass(param, cameraCbv);

		RenderGUIObjects(param, 1);
	}

	void Scene::RenderShadowSceneObjects(RenderParam& param, int instances)
	{
		for (const auto& object : m_renderShadowObjectQueue)
		{
			param.CommandList->SetPipelineState(object->GetShadowPipelineState());
			object->Render(param, instances);
		}
		param.RenderStageIndex++;
	}
	
	void Scene::RenderSceneObjects(RenderParam& param, RenderGroup group, int instances)
	{
		for (const auto& object : m_renderObjectQueues[group])
		{
			param.CommandList->SetPipelineState(object->GetPipelineState());
			object->Render(param, instances);
		}
		param.RenderStageIndex++;
	}

	void Scene::RenderGUIObjects(RenderParam& param, int instances)
	{
		param.CommandList->SetPipelineState(GUIImage::GetPipelineState());
		param.CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		param.CommandList->IASetVertexBuffers(0, 0, nullptr);
		param.CommandList->IASetIndexBuffer(nullptr);

		for (const auto& object : m_renderGUIObjectQueue)
		{
			object->Render(param, instances);
		}
	}
}