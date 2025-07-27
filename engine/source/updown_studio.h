#pragma once

#include "pch.h"

#include "resource_load.h"
#include "resource_object.h"
#include "material.h"
#include "mesh.h"
#include "rigged_mesh.h"
#include "texture.h"
#include "shader.h"
#include "texture.h"
#include "audio_clip.h"
#include "font.h"

#include "shadow_map.h"
#include "screen_space_ao.h"
#include "deferred_renderer.h"
#include "shader_compile.h"
#include "animation_clip.h"
#include "gui_image.h"
#include "gui_text.h"
#include "gui_button.h"

#include "debug_console.h"
#include "time_measure.h"
#include "input.h"
#include "audio.h"
#include "core.h"

#include "scene.h"
#include "scene_object.h"
#include "transform.h"
#include "component.h"
#include "mesh_renderer.h"
#include "rigged_mesh_renderer.h"
#include "rigged_prop_renderer.h"
#include "inline_mesh_renderer.h"
#include "camera.h"
#include "light_directional.h"

namespace udsdx
{
	class UpdownStudio
	{
	private:
		static std::wstring m_className;
		static std::wstring m_windowName;

		static HINSTANCE m_hInstance;
		static HWND m_hWnd;

		static std::function<void()> m_ioUpdateCallback;

	public:
		static int Initialize(HINSTANCE hInstance, std::wstring_view sWindowName = std::wstring());
		static int Run(std::shared_ptr<Scene> beginScene, int nCmdShow = SW_SHOWNORMAL);
		static void Quit();
		static LRESULT CALLBACK ProcessMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
		static void RegisterIOUpdateCallback(std::function<void()> callback);
		static void RegisterUpdateCallback(std::function<void(const Time&)> callback);
		static inline const auto GetMainHWND()noexcept { return m_hWnd; }
	};
}