#include "NagiocpXPch.h"
#include "ImGUIMgr.h"

namespace NagiocpX
{
	ImGUIMgr::ImGUIMgr()
	{
	}

	ImGUIMgr::~ImGUIMgr()
	{
        ShutDown();
	}

	void ImGUIMgr::Init() noexcept
	{
        if (!glfwInit())
        {
            fprintf(stderr, "[ImGUIMgr] GLFW init failed.\n");
            return;
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        m_window = glfwCreateWindow(1280, 720, "NagiocpX Window", nullptr, nullptr);

        if (!m_window)
        {
            fprintf(stderr, "[ImGuiManager] GLFW window creation failed.\n");
            return;
        }

        glfwMakeContextCurrent(m_window);
        glfwSwapInterval(1);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsDark();
        
        if (!ImGui_ImplGlfw_InitForOpenGL(m_window, true)) return;
        if (!ImGui_ImplOpenGL3_Init("#version 330")) return;

	}

    void ImGUIMgr::ShutDown() noexcept
    {
        if (!m_window)return;

        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        glfwDestroyWindow(m_window);
        m_window = nullptr;

        glfwTerminate();
    }

    void ImGUIMgr::Update() noexcept
    {
        while (!glfwWindowShouldClose(m_window))
        {
            Render();
            Sleep(100);
        }
    }

    void ImGUIMgr::Render() noexcept
    {
        BeginFrame();
        m_renderFp();
        EndFrame();
    }

	void ImGUIMgr::BeginFrame() noexcept
	{
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
	}

	void ImGUIMgr::EndFrame() noexcept
	{
      ImGui::Render();
      int w, h;
      glfwGetFramebufferSize(m_window, &w, &h);
      glViewport(0, 0, w, h);
      glClearColor(0.1f, 0.1f, 0.1f, 1.f);
      glClear(GL_COLOR_BUFFER_BIT);
      ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
      glfwSwapBuffers(m_window);
	}
}