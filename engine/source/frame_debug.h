#pragma once

#include "pch.h"

namespace udsdx
{
	class FrameDebug
	{
	public:
		FrameDebug(HINSTANCE hInstance);
		~FrameDebug();

		void Update(const Time& time);

	private:
		static constexpr int WindowWidth = 480;
		static constexpr int WindowHeight = 160;

	private:
		HWND m_hWnd = 0;
		HDC m_hDC = 0;
		HDC m_hMemDC = 0;
		HDC m_hBitmapDC = 0;
		HBITMAP m_graphBitmap = 0;
		HBITMAP m_memBitmap = 0;
		int currentFrameX = 0;

		HPEN m_hMaskPen;
		HPEN m_hBlackPen;
		HPEN m_hWhitePen;
	};
}