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
	private:
		using RendererGroup = std::unordered_map<ID3D12PipelineState*, std::unordered_map<ID3D12PipelineState*, std::vector<std::pair<RendererBase*, int>>>>;

	public:
		Scene();
		~Scene();

	public:
		// Called when the scene is attached from the core
		virtual void OnAttach();

		// Called every frame
		virtual void Update(const Time& time);

		// Called every frame after Update()
		virtual void PostUpdate(const Time& time);

		// Called when the component needs to draw ImGUI primitives
		virtual void OnDrawGizmos();

		// Called when the scene needs to draw primitives. The command list already called Reset() and is ready to use.
		virtual void Render(RenderParam& param);

		// Called when the scene is detached from the core
		virtual void OnDetach();

		void UpdateGUIElementEvent(const Time& time);
		void AddObject(std::shared_ptr<SceneObject> object);

		void HandleAttach();
		void HandleDetach();

	public:
		void EnqueueRenderCamera(Camera* camera);
		void EnqueueRenderLight(LightDirectional* light);
		void EnqueueRenderObject(RendererBase* object, RenderGroup group, ID3D12PipelineState* pipelineState, ID3D12PipelineState* deferredPipelineState, int parameter);
		void EnqueueRenderShadowObject(RendererBase* object, ID3D12PipelineState* pipelineState, int parameter);
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
		std::shared_ptr<SceneObject> m_rootObjectSub;

		std::vector<Camera*> m_renderCameraQueue;
		std::vector<LightDirectional*> m_renderLightQueue;
		std::array<RendererGroup, 2> m_renderObjectQueues;
		std::unordered_map<ID3D12PipelineState*, std::vector<std::pair<RendererBase*, int>>> m_renderShadowObjectQueue;
		std::vector<GUIElement*> m_renderGUIObjectQueue;
	};
}

