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

namespace udsdx
{
	Scene::Scene()
	{
		m_rootObject = std::make_unique<SceneObject>();
	}

	Scene::~Scene()
	{

	}

	void Scene::Update(const Time& time)
	{ ZoneScoped;
		m_renderCameraQueue.clear();
		m_renderLightQueue.clear();
		m_renderObjectQueue.clear();
		m_renderShadowObjectQueue.clear();

		m_rootObject->Update(time, *this, false);
	}

	void Scene::Render(RenderParam& param)
	{ ZoneScoped;
	
		// Shadow map rendering pass
		if (!m_renderLightQueue.empty() && !m_renderCameraQueue.empty())
		{
			PassRenderShadow(param, m_renderCameraQueue.front(), m_renderLightQueue[0]);
		}

		for (const auto& camera : m_renderCameraQueue)
		{
			PassRenderMain(param, camera);
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

	void Scene::EnqueueRenderObject(RendererBase* object)
	{
		m_renderObjectQueue.emplace_back(object);
	}

	void Scene::EnqueueRenderShadowObject(RendererBase* object)
	{
		m_renderShadowObjectQueue.emplace_back(object);
	}

	void Scene::PassRenderShadow(RenderParam& param, Camera* camera, LightDirectional* light)
	{
		ZoneScopedN("Shadow Render Pass");
		TracyD3D12Zone(*param.TracyQueueContext, param.CommandList, "Shadow Render Pass");
		param.RenderShadowMap->Pass(param, this, camera, light);
	}

	void Scene::PassRenderNormal(RenderParam& param, Camera* camera)
	{
		ZoneScopedN("Normal / Depth Render Pass");
		TracyD3D12Zone(*param.TracyQueueContext, param.CommandList, "Normal / Depth Render Pass");
		param.RenderScreenSpaceAO->PassNormal(param, this, camera);
	}

	void Scene::PassRenderSSAO(RenderParam& param, Camera* camera)
	{
		ZoneScopedN("SSAO Render Pass");
		TracyD3D12Zone(*param.TracyQueueContext, param.CommandList, "SSAO Render Pass");
		param.RenderScreenSpaceAO->UpdateSSAOConstants(param, camera);
		param.RenderScreenSpaceAO->PassSSAO(param);
		param.RenderScreenSpaceAO->PassBlur(param);
	}

	void Scene::PassRenderMain(RenderParam& param, Camera* camera)
	{
		ZoneScopedN("Main Pass");
		TracyD3D12Zone(*param.TracyQueueContext, param.CommandList, "Main Pass");

		param.Renderer->PassBufferPreparation(param);

		Matrix4x4 viewMat = camera->GetViewMatrix();
		Matrix4x4 projMat = camera->GetProjMatrix(param.AspectRatio);
		param.ViewFrustumWorld = camera->GetViewFrustumWorld(param.AspectRatio);

		CameraConstants cameraConstants;
		cameraConstants.View = viewMat.Transpose();
		cameraConstants.Proj = projMat.Transpose();

		Matrix4x4 worldMat = camera->GetSceneObject()->GetTransform()->GetWorldSRTMatrix();
		cameraConstants.CameraPosition = Vector4::Transform(Vector4::UnitW, worldMat);

		param.CommandList->SetGraphicsRoot32BitConstants(RootParam::PerCameraCBV, sizeof(CameraConstants) / 4, &cameraConstants, 0);

		RenderSceneObjects(param, 1);

		param.Renderer->PassBufferPostProcess(param);

		PassRenderSSAO(param, camera);

		param.CommandList->SetGraphicsRoot32BitConstants(0, sizeof(CameraConstants) / 4, &cameraConstants, 0);
		param.Renderer->PassRender(param);
	}

	void Scene::RenderShadowSceneObjects(RenderParam& param, int instances)
	{
		for (const auto& object : m_renderObjectQueue)
		{
			param.CommandList->SetPipelineState(object->GetShadowPipelineState());
			object->Render(param, instances);
		}
	}

	void Scene::RenderSceneObjects(RenderParam& param, int instances)
	{
		for (const auto& object : m_renderObjectQueue)
		{
			param.CommandList->SetPipelineState(object->GetPipelineState());
			object->Render(param, instances);
		}
	}
}