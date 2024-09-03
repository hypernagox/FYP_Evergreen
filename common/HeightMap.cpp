#include "HeightMap.h"

HeightMap::HeightMap(std::wstring_view filename, LONG width, LONG height) : m_width(width), m_height(height)
{
	HANDLE file = CreateFile(filename.data(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_READONLY, nullptr);
	if (file == INVALID_HANDLE_VALUE)
	{
		throw std::exception("Failed to open file");
	}

	m_heightData.resize(m_height * m_width);

	DWORD bytesRead = 0;
	if (!ReadFile(file, m_heightData.data(), static_cast<DWORD>(m_heightData.size() * sizeof(Format)), &bytesRead, nullptr))
	{
		throw std::exception("Failed to read file");
	}

	CloseHandle(file);
}

float HeightMap::GetHeight(LONG x, LONG y) const
{
	if (x < 0 || x >= m_width || y < 0 || y >= m_height)
	{
		return 0.0f;
	}

	return m_heightData[y * m_width + x] / static_cast<float>((1LL << ((sizeof(Format) * 8))) - 1);
}

float HeightMap::GetHeight(float x, float y) const
{
	const LONG x0 = static_cast<LONG>(x);
	const LONG x1 = x0 + 1;
	const LONG y0 = static_cast<LONG>(y);
	const LONG y1 = y0 + 1;

	const float dx = x - x0;
	const float dy = y - y0;

	const float h00 = GetHeight(x0, y0);
	const float h01 = GetHeight(x0, y1);
	const float h10 = GetHeight(x1, y0);
	const float h11 = GetHeight(x1, y1);

	const float h0 = h00 * (1.0f - dx) + h10 * dx;
	const float h1 = h01 * (1.0f - dx) + h11 * dx;

	return h0 * (1.0f - dy) + h1 * dy;
}

LONG HeightMap::GetPixelWidth() const
{
	return m_width;
}

LONG HeightMap::GetPixelHeight() const
{
	return m_height;
}
