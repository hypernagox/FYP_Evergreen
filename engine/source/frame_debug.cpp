#include "pch.h"
#include "frame_debug.h"

namespace udsdx
{
	FrameDebug::FrameDebug(HINSTANCE hInstance)
	{
		m_hWnd = CreateWindowW(L"UpdownStudioClass", L"Updown Studio Frame Debug", WS_OVERLAPPED,
						100, 100, WindowWidth, WindowHeight, nullptr, nullptr, hInstance, nullptr);

		if (!m_hWnd)
		{
			return;
		}

		ShowWindow(m_hWnd, SW_SHOW);
		UpdateWindow(m_hWnd);

		m_hDC = GetDC(m_hWnd);
		m_hMemDC = CreateCompatibleDC(m_hDC);
		m_hBitmapDC = CreateCompatibleDC(m_hDC);

		// Create pens
		m_hMaskPen = CreatePen(PS_SOLID, 1, RGB(255, 0, 255));
		m_hBlackPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
		m_hWhitePen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));

		// Create bitmap
		m_graphBitmap = CreateCompatibleBitmap(m_hDC, WindowWidth, WindowHeight);
		m_memBitmap = CreateCompatibleBitmap(m_hDC, WindowWidth, WindowHeight);
	}

	FrameDebug::~FrameDebug()
	{
		if (m_hWnd)
		{
			DeleteObject(m_memBitmap);
			DeleteObject(m_graphBitmap);
			DeleteDC(m_hBitmapDC);
			DeleteDC(m_hMemDC);
			ReleaseDC(m_hWnd, m_hDC);
			DestroyWindow(m_hWnd);

			DeletePen(m_hMaskPen);
			DeletePen(m_hBlackPen);
			DeletePen(m_hWhitePen);
		}
	}

	void FrameDebug::Update(const Time& time)
	{
		if (!m_hWnd)
		{
			return;
		}

		int height = static_cast<int>(time.deltaTime * 5'000);
		RECT clientRect;
		GetClientRect(m_hWnd, &clientRect);
		int bottom = clientRect.bottom;

		HPEN oldPen = (HPEN)SelectObject(m_hBitmapDC, m_hBlackPen);

		SelectBitmap(m_hMemDC, m_memBitmap);
		MoveToEx(m_hBitmapDC, currentFrameX, 0, nullptr);
		LineTo(m_hBitmapDC, currentFrameX, bottom);

		SelectObject(m_hBitmapDC, m_hWhitePen);
		LineTo(m_hBitmapDC, currentFrameX, bottom - height);

		SelectObject(m_hBitmapDC, oldPen);

		SelectBitmap(m_hBitmapDC, m_graphBitmap);
		BitBlt(m_hMemDC, 0, 0, WindowWidth, WindowHeight, m_hBitmapDC, 0, 0, SRCCOPY);

		SetBkMode(m_hMemDC, TRANSPARENT);
		SetTextColor(m_hMemDC, RGB(255, 255, 255));

		std::wstring text = std::format(L"{:4} fps", static_cast<int>(1.0f / time.deltaTime));
		TextOutW(m_hMemDC, 0, 0, text.c_str(), (int)text.size());
		text = std::format(L"{:.1f} ms", time.deltaTime * 1000);
		TextOutW(m_hMemDC, 0, 16, text.c_str(), (int)text.size());

		SelectBitmap(m_hDC, m_memBitmap);
		BitBlt(m_hDC, 0, 0, WindowWidth, WindowHeight, m_hMemDC, 0, 0, SRCCOPY);

		currentFrameX = (currentFrameX + 1) % WindowWidth;
	}
}