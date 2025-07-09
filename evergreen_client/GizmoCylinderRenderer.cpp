#include "pch.h"
#include "GizmoCylinderRenderer.h"

static constexpr char g_psoResource[] = R"(
	cbuffer cbPerObject : register(b0)
	{
		float4x4 gWorld;
		float4x4 gPrevWorld;
	};

	cbuffer cbPerCamera : register(b1)
	{
		float4x4 gView;
		float4x4 gProj;
		float4x4 gViewProj;
		float4x4 gViewInverse;
		float4x4 gProjInverse;
		float4x4 gViewProjInverse;
		float4x4 gPrevViewProj;
		float4 gEyePosW;
	};

	static float3 gVertexData[16] = 
	{
        float3(-1.0f, 0.0f, 0.0f),
		float3(-0.707f, 0.0f, -0.707f),
		float3(0.0f, 0.0f, -1.0f),
		float3(0.707f, 0.0f, -0.707f),
		float3(1.0f, 0.0f, 0.0f),
		float3(0.707f, 0.0f, 0.707f),
		float3(0.0f, 0.0f, 1.0f),
		float3(-0.707f, 0.0f, 0.707f),
        float3(-1.0f, 1.0f, 0.0f),
		float3(-0.707f, 1.0f, -0.707f),
		float3(0.0f, 1.0f, -1.0f),
		float3(0.707f, 1.0f, -0.707f),
		float3(1.0f, 1.0f, 0.0f),
		float3(0.707f, 1.0f, 0.707f),
		float3(0.0f, 1.0f, 1.0f),
		float3(-0.707f, 1.0f, 0.707f)
	};

	static uint gIndexData[40] = 
	{
        0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 0,
        8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13, 14, 14, 15, 15, 8,
		0, 8, 2, 10, 4, 12, 6, 14,
	};

	float4 VS(uint vid : SV_VertexID) : SV_POSITION
	{
		float4 worldPos = mul(float4(gVertexData[gIndexData[vid]], 1.0f), gWorld);
		float4 projPos = mul(worldPos, gViewProj);
		return projPos;
	}

	float4 PS(float4 pos : SV_POSITION) : SV_TARGET
	{
		return float4(1.0f, 1.0f, 1.0f, 1.0f);
	}
)";

using namespace udsdx;

void GizmoCylinderRenderer::OnDrawGizmos(const udsdx::Camera* target)
{
	static Vector3 vertexData[] =
	{
		Vector3(-1.0f, 0.0f, 0.0f),
		Vector3(-0.707f, 0.0f, -0.707f),
		Vector3(0.0f, 0.0f, -1.0f),
		Vector3(0.707f, 0.0f, -0.707f),
		Vector3(1.0f, 0.0f, 0.0f),
		Vector3(0.707f, 0.0f, 0.707f),
		Vector3(0.0f, 0.0f, 1.0f),
		Vector3(-0.707f, 0.0f, 0.707f),
		Vector3(-1.0f, 1.0f, 0.0f),
		Vector3(-0.707f, 1.0f, -0.707f),
		Vector3(0.0f, 1.0f, -1.0f),
		Vector3(0.707f, 1.0f, -0.707f),
		Vector3(1.0f, 1.0f, 0.0f),
		Vector3(0.707f, 1.0f, 0.707f),
		Vector3(0.0f, 1.0f, 1.0f),
		Vector3(-0.707f, 1.0f, 0.707f)
	};

	static unsigned int indexData[] =
	{
		0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 0,
		8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13, 14, 14, 15, 15, 8,
		0, 8, 2, 10, 4, 12, 6, 14,
	};

	std::array<Vector2, _countof(vertexData)> vertexScreen;
	bool isVisible = true;
	for (size_t i = 0; i < vertexScreen.size() && isVisible; ++i)
	{
		Vector3 worldPosition = Vector3::Transform(vertexData[i] * Vector3(m_radius, m_height, m_radius) + m_offset, GetTransform()->GetWorldSRTMatrix());

		isVisible &= target->ToViewPosition(worldPosition).z > 1e-2f;
		vertexScreen[i] = target->ToScreenPosition(worldPosition);
	}

	ImDrawList* drawList = ImGui::GetBackgroundDrawList();
	ImColor drawColor(1.0f, 1.0f, 1.0f, 1.0f);

	for (size_t i = 0; i < _countof(indexData) >> 1; ++i)
	{
		int start = indexData[i << 1];
		int end = indexData[i << 1 | 1];
		drawList->AddLine(
			ImVec2(vertexScreen[start].x, vertexScreen[start].y),
			ImVec2(vertexScreen[end].x, vertexScreen[end].y),
			drawColor);
	}
}
