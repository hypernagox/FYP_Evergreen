#pragma once

#include "pch.h"

namespace udsdx
{
	class SceneObject;
	class RendererBase;
	class GUIElement;
	class Camera;
	class LightDirectional;

	class Scene
	{
	public:
		Scene();
		~Scene();

	public:
		virtual void Update(const Time& time);
		virtual void PostUpdate(const Time& time);
		void Render(RenderParam& param);
		void UpdateGUIElementEvent(const Time& time);

		void AddObject(std::shared_ptr<SceneObject> object);

	public:
		void EnqueueRenderCamera(Camera* camera);
		void EnqueueRenderLight(LightDirectional* light);
		void EnqueueRenderObject(RendererBase* object, RenderGroup group);
		void EnqueueRenderShadowObject(RendererBase* object);
		void EnqueueRenderGUIObject(GUIElement* object);

		void RenderShadowSceneObjects(RenderParam& param, int instances = 1);
		void RenderSceneObjects(RenderParam& param, RenderGroup group, int instances = 1);
		void RenderGUIObjects(RenderParam& param, int instances = 1);

	private:
		void PassRenderShadow(RenderParam& param, Camera* camera, LightDirectional* light);
		void PassRenderSSAO(RenderParam& param, Camera* camera);
		void PassRenderMain(RenderParam& param, Camera* camera, D3D12_GPU_VIRTUAL_ADDRESS cameraCbv);
		void PassRenderHUD(RenderParam& param);

	protected:
		std::shared_ptr<SceneObject> m_rootObject;

		std::vector<Camera*> m_renderCameraQueue;
		std::vector<LightDirectional*> m_renderLightQueue;
		std::array<std::vector<RendererBase*>, 2> m_renderObjectQueues;
		std::vector<RendererBase*> m_renderShadowObjectQueue;
		std::vector<GUIElement*> m_renderGUIObjectQueue;
	};
}

