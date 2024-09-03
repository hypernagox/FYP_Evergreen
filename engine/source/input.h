#pragma once

#include "pch.h"

namespace udsdx
{
	class Input
	{
	public:
		Input();
		~Input();

		bool ProcessMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
		void Initialize(HWND hWnd);
		void Reset();
		void Update();

		// Set relative mouse mode
		void SetRelativeMouse(bool value);

		// Check if a key is pressed
		bool GetKey(Keyboard::Keys key) const;
		// Check if a key is pressed this frame
		bool GetKeyDown(Keyboard::Keys key) const;
		// Check if a key is released this frame
		bool GetKeyUp(Keyboard::Keys key) const;

		// Check if a mouse button is pressed
		bool GetMouseLeftButton() const;
		// Check if a mouse button is pressed this frame
		bool GetMouseLeftButtonDown() const;
		// Check if a mouse button is released this frame
		bool GetMouseLeftButtonUp() const;

		// Check if a mouse button is pressed
		bool GetMouseRightButton() const;
		// Check if a mouse button is pressed this frame
		bool GetMouseRightButtonDown() const;
		// Check if a mouse button is released this frame
		bool GetMouseRightButtonUp() const;

		// Check if a mouse button is pressed
		bool GetMouseMiddleButton() const;
		// Check if a mouse button is pressed this frame
		bool GetMouseMiddleButtonDown() const;
		// Check if a mouse button is released this frame
		bool GetMouseMiddleButtonUp() const;

		// Check if a mouse button is pressed
		bool GetMouseXButton1() const;
		// Check if a mouse button is pressed this frame
		bool GetMouseXButton1Down() const;
		// Check if a mouse button is released this frame
		bool GetMouseXButton1Up() const;

		// Check if a mouse button is pressed
		bool GetMouseXButton2() const;
		// Check if a mouse button is pressed this frame
		bool GetMouseXButton2Down() const;
		// Check if a mouse button is released this frame
		bool GetMouseXButton2Up() const;

		// Mouse x position in screen space (up-left origin)
		int GetMouseX() const;
		// Mouse y position in screen space (up-left origin)
		int GetMouseY() const;
		// Mouse scroll wheel value
		int GetMouseScroll() const;

	private:
		std::unique_ptr<Keyboard> m_keyboard;
		std::unique_ptr<Mouse> m_mouse;

		Mouse::ButtonStateTracker m_mouseTracker;
		Keyboard::KeyboardStateTracker m_keyboardTracker;

		int m_mouseX = 0;
		int m_mouseY = 0;
	};
}

