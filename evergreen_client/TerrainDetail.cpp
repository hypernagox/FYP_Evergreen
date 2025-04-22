#include "pch.h"
#include "TerrainDetail.h"
#include "HeightMap.h"

static float GetValue(const std::vector<BYTE>& heightData, LONG sourceWidth, LONG sourceHeight, float x, float y, int channel)
{
	// Linear interpolation
	const LONG x0 = static_cast<LONG>(x);
	const LONG x1 = x0 + 1;
	const LONG y0 = static_cast<LONG>(y);
	const LONG y1 = y0 + 1;
	const float dx = x - x0;
	const float dy = y - y0;
	const BYTE h00 = heightData[(y0 * sourceWidth + x0) * 3 + channel];
	const BYTE h01 = heightData[(y1 * sourceWidth + x0) * 3 + channel];
	const BYTE h10 = heightData[(y0 * sourceWidth + x1) * 3 + channel];
	const BYTE h11 = heightData[(y1 * sourceWidth + x1) * 3 + channel];
	const float h0 = h00 * (1.0f - dx) + h10 * dx;
	const float h1 = h01 * (1.0f - dx) + h11 * dx;
	const float h = h0 * (1.0f - dy) + h1 * dy;
	return h / 255.0f;
}

TerrainDetail::TerrainDetail(HeightMap* heightMap, std::wstring_view filename, LONG sourceWidth, LONG sourceHeight, UINT segmentation, ID3D12Device* device, ID3D12GraphicsCommandList* commandList) : m_segmentation(segmentation)
{
	LONG width = heightMap->GetPixelWidth();
	LONG height = heightMap->GetPixelHeight();

	std::uniform_real_distribution<float> distribution(0.0f, 1.0f);
	std::default_random_engine generator;

	std::vector<BYTE> heightData(sourceWidth * sourceHeight * 3);
	{
		HANDLE file = CreateFile(filename.data(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_READONLY, nullptr);
		if (file == INVALID_HANDLE_VALUE)
		{
			throw std::exception("Failed to open file");
		}

		DWORD bytesRead = 0;
		if (!ReadFile(file, heightData.data(), static_cast<DWORD>(heightData.size() * sizeof(BYTE)), &bytesRead, nullptr))
		{
			throw std::exception("Failed to read file");
		}

		CloseHandle(file);
	}

	const int sampleCount = (1 << 22) / (segmentation * segmentation);
	std::vector<udsdx::Vector2> positions;
	positions.reserve(sampleCount);
	for (int i = 0; i < sampleCount; i++)
	{
		float x = distribution(generator);
		float y = distribution(generator);
		positions.emplace_back(x, y);
	}

	std::vector<udsdx::Vertex> vertices;
	std::vector<UINT> indices;

	m_submeshes.resize(segmentation);
	for (int segmentX = 0; segmentX < segmentation; ++segmentX)
	{
		m_submeshes[segmentX].resize(segmentation);
		for (int segmentY = 0; segmentY < segmentation; ++segmentY)
		{
			m_submeshes[segmentX][segmentY].first = static_cast<UINT>(indices.size());
			for (int type = 0; type < 3; type++)
			{
				for (int i = 0; i < sampleCount; i++)
				{
					float x = positions[i].x * (1.0f / segmentation) + (static_cast<float>(segmentX) / segmentation);
					float y = positions[i].y * (1.0f / segmentation) + (static_cast<float>(segmentY) / segmentation);

					float r = GetValue(heightData, sourceWidth, sourceHeight, x * sourceWidth, y * sourceHeight, type);
					if (r <= distribution(generator))
						continue;

					float h = heightMap->GetHeight(x * width, y * height);
					udsdx::Vertex vertex{};
					vertex.position = Vector3(x, h, y);
					vertex.uv = udsdx::Vector2(type % 2, type / 2) * 0.5f;
					vertices.emplace_back(vertex);
					indices.emplace_back(static_cast<UINT>(indices.size()));
				}
			}
			m_submeshes[segmentX][segmentY].second = static_cast<UINT>(indices.size()) - m_submeshes[segmentX][segmentY].first;
		}
	}

	m_detailMesh = std::make_unique<udsdx::Mesh>(vertices, indices);
	m_detailMesh->UploadBuffers(device, commandList);
}
