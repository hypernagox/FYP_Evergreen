#include "pch.h"
#include "Input.h"
#include "debug_console.h"

namespace udsdx
{
	Input::Input()
	{
		m_keyboard = std::make_unique<Keyboard>();
		m_mouse = std::make_unique<Mouse>();
	}

	Input::~Input()
	{

	}

	LRESULT Input::ProcessMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		switch (message)
		{
		case WM_MOUSEACTIVATE:
			return MA_ACTIVATEANDEAT;
		case WM_ACTIVATEAPP:
			m_mouse->ProcessMessage(message, wParam, lParam);
			m_keyboard->ProcessMessage(message, wParam, lParam);
			break;
		case WM_ACTIVATE:
		case WM_INPUT:
		case WM_MOUSEMOVE:
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		case WM_MOUSEWHEEL:
		case WM_XBUTTONDOWN:
		case WM_XBUTTONUP:
		case WM_MOUSEHOVER:
			m_mouse->ProcessMessage(message, wParam, lParam);
			break;
		case WM_KEYDOWN:
		case WM_KEYUP:
			m_keyboard->ProcessMessage(message, wParam, lParam);
			break;
		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
			m_keyboard->ProcessMessage(message, wParam, lParam);
			return 0; // Prevent the system from processing this message (It freezes the window if the default WindowProc is returned in this case)
		}

		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	void Input::Initialize(HWND hWnd)
	{
		m_mouse->SetWindow(hWnd);
	}

	void Input::Reset()
	{
		m_mouse->ResetScrollWheelValue();
		m_keyboard->Reset();

		m_mouseTracker.Reset();
		m_keyboardTracker.Reset();
	}

	void Input::Update()
	{
		auto& mState = m_mouse->GetState();
		auto& kState = m_keyboard->GetState();

		m_mouseX = mState.x;
		m_mouseY = mState.y;

		m_mouseTracker.Update(mState);
		m_keyboardTracker.Update(kState);
	}

	Mouse::Mode Input::GetMouseMode() const
	{
		return m_mouse->GetState().positionMode;
	}

	void Input::SetRelativeMouse(bool value)
	{
		m_mouse->SetMode(value ? Mouse::MODE_RELATIVE : Mouse::MODE_ABSOLUTE);

		// Force the cursor to be hidden or shown based on the mode
		// ShowCursor behaviour is based on increment / decrement counting; some operations may not count correctly
		// So we need to loop until the cursor is in the desired state
		// This is a bit of a hack, but it works

		// The maximum number of times to try ShowCursor is restricted due to the potential infinite loop
		int timeoutCount = 16;
		while (timeoutCount > 0 && value)
		{
			if (ShowCursor(FALSE) < 0)
			{
				break;
			}
			--timeoutCount;
		}
		while (timeoutCount > 0 && !value)
		{
			if (ShowCursor(TRUE) >= 0)
			{
				break;
			}
			--timeoutCount;
		}
	}

	bool Input::GetKey(Keyboard::Keys key) const
	{
		return m_keyboard->GetState().IsKeyDown(key);
	}

	bool Input::GetKeyDown(Keyboard::Keys key) const
	{
		return m_keyboardTracker.IsKeyPressed(key);
	}

	bool Input::GetKeyUp(Keyboard::Keys key) const
	{
		return m_keyboardTracker.IsKeyReleased(key);
	}

	bool Input::GetMouseLeftButton() const
	{
		return m_mouse->GetState().leftButton;
	}

	bool Input::GetMouseLeftButtonDown() const
	{
		return m_mouseTracker.leftButton == Mouse::ButtonStateTracker::PRESSED;
	}

	bool Input::GetMouseLeftButtonUp() const
	{
		return m_mouseTracker.leftButton == Mouse::ButtonStateTracker::RELEASED;
	}

	bool Input::GetMouseRightButton() const
	{
		return m_mouse->GetState().rightButton;
	}

	bool Input::GetMouseRightButtonDown() const
	{
		return m_mouseTracker.rightButton == Mouse::ButtonStateTracker::PRESSED;
	}

	bool Input::GetMouseRightButtonUp() const
	{
		return m_mouseTracker.rightButton == Mouse::ButtonStateTracker::RELEASED;
	}

	bool Input::GetMouseMiddleButton() const
	{
		return m_mouse->GetState().middleButton;
	}

	bool Input::GetMouseMiddleButtonDown() const
	{
		return m_mouseTracker.middleButton == Mouse::ButtonStateTracker::PRESSED;
	}

	bool Input::GetMouseMiddleButtonUp() const
	{
		return m_mouseTracker.middleButton == Mouse::ButtonStateTracker::RELEASED;
	}

	bool Input::GetMouseXButton1() const
	{
		return m_mouse->GetState().xButton1;
	}

	bool Input::GetMouseXButton1Down() const
	{
		return m_mouseTracker.xButton1 == Mouse::ButtonStateTracker::PRESSED;
	}

	bool Input::GetMouseXButton1Up() const
	{
		return m_mouseTracker.xButton1 == Mouse::ButtonStateTracker::RELEASED;
	}

	bool Input::GetMouseXButton2() const
	{
		return m_mouse->GetState().xButton2;
	}

	bool Input::GetMouseXButton2Down() const
	{
		return m_mouseTracker.xButton2 == Mouse::ButtonStateTracker::PRESSED;
	}

	bool Input::GetMouseXButton2Up() const
	{
		return m_mouseTracker.xButton2 == Mouse::ButtonStateTracker::RELEASED;
	}

	int Input::GetMouseX() const
	{
		return m_mouseX;
	}

	int Input::GetMouseY() const
	{
		return m_mouseY;
	}

	int Input::GetMouseScroll() const
	{
		return m_mouse->GetState().scrollWheelValue;
	}
}