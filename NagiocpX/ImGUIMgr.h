#pragma once
#include "Singleton.hpp"

struct GLFWwindow;

namespace NagiocpX
{
	class ImGUIMgr
		:public Singleton<ImGUIMgr>
	{
		friend class Singleton;
		ImGUIMgr();
		~ImGUIMgr();
	public:
		void Init()noexcept;
		void ShutDown()noexcept;
	public:
		void Update()noexcept;
		void RegisterRenderFp(std::function<void(void)> fp_)noexcept { m_renderFp.swap(fp_); }
	private:
		void Render()noexcept;
		void BeginFrame()noexcept;
		void EndFrame()noexcept;
	private:
		GLFWwindow* m_window = nullptr;
		std::function<void(void)> m_renderFp = nullptr;
	};
}

