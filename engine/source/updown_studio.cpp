#include "pch.h"
#include "updown_studio.h"

#include "core.h"

namespace udsdx
{
    std::wstring UpdownStudio::m_className = L"UpdownStudioClass";
    std::wstring UpdownStudio::m_windowName = L"Updown Studio Application";

    HINSTANCE UpdownStudio::m_hInstance = nullptr;
    HWND UpdownStudio::m_hWnd = nullptr;
    std::function<void()> UpdownStudio::m_ioUpdateCallback = nullptr;


    int UpdownStudio::Initialize(HINSTANCE hInstance, std::wstring_view sWindowName)
    {
        m_hInstance = hInstance;
        if (!sWindowName.empty())
        {
			m_windowName = sWindowName.data();
		}

        WNDCLASSEXW wcex = { 0 };
        wcex.cbSize = sizeof(WNDCLASSEX);
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = UpdownStudio::ProcessMessage;
        wcex.hInstance = hInstance;
        wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wcex.lpszClassName = m_className.c_str();
        RegisterClassExW(&wcex);

        m_hWnd = CreateWindowW(m_className.c_str(), m_windowName.c_str(), WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT, 1280, 720, nullptr, nullptr, hInstance, nullptr);
        if (!m_hWnd)
        {
            return -1;
        }

        // Create console window for debug
#ifdef CONSOLE
        if (AllocConsole())
        {
            FILE* fp;
            freopen_s(&fp, "CONOUT$", "w", stdout);
        }
#endif

        INSTANCE(Core)->Initialize(hInstance, m_hWnd);
        return 0;
    }

    int udsdx::UpdownStudio::Run(std::shared_ptr<Scene> beginScene, int nCmdShow)
    {
        auto core = INSTANCE(Core);

        if (!m_hWnd)
		{
			return -1;
		}

        core->SetScene(beginScene);

        ShowWindow(m_hWnd, nCmdShow);
        UpdateWindow(m_hWnd);

        MSG message{};
        while (message.message != WM_QUIT)
        { ZoneScopedN("Updown Studio Main Loop"); FrameMark;
            if (PeekMessage(&message, NULL, 0, 0, PM_REMOVE))
            {
				TranslateMessage(&message);
				DispatchMessage(&message);
			}
            else
            {
                if (m_ioUpdateCallback != nullptr)
                {
                    m_ioUpdateCallback();
                }
                core->AcquireNextFrameResource();
                core->Update();
                core->Render();
            }
        }

        core->OnDestroy();

        return static_cast<int>(message.wParam);
    }

    void UpdownStudio::Quit()
    {
        PostQuitMessage(0);
    }

    LRESULT CALLBACK UpdownStudio::ProcessMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        if (hWnd != m_hWnd)
        {
			return DefWindowProc(hWnd, message, wParam, lParam);
		}

        switch (message)
        {
        case WM_DESTROY:
			PostQuitMessage(0);
			break;

        default:
            return INSTANCE(Core)->ProcessMessage(hWnd, message, wParam, lParam);
        }
        return 0;
    }

    void UpdownStudio::RegisterIOUpdateCallback(std::function<void()> callback)
	{
        m_ioUpdateCallback = callback;
	}

    void UpdownStudio::RegisterUpdateCallback(std::function<void(const Time&)> callback)
    {
        INSTANCE(Core)->RegisterUpdateCallback(callback);
    }
}