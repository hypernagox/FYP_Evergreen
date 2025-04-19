#pragma once

#include "pch.h"

class HeightMap
{
public:
	using Format = unsigned short;

public:
	HeightMap(std::wstring_view filename, LONG width, LONG height);

	float GetHeight(LONG x, LONG y) const;
	float GetHeight(float x, float y) const;

	LONG GetPixelWidth() const;
	LONG GetPixelHeight() const;

private:
	LONG m_width = 0;
	LONG m_height = 0;

	std::vector<Format> m_heightData;
};