#include "pch.h"
#include "GizmoSphereRenderer.h"

using namespace udsdx;

void GizmoSphereRenderer::OnDrawGizmos(const udsdx::Camera* target)
{
	static Vector3 vertexData[] =
	{
		Vector3(-1.0f, 0.0f, 0.0f),
		Vector3(1.0f, 0.0f, 0.0f),
		Vector3(0.0f, -1.0f, 0.0f),
		Vector3(0.0f, 1.0f, 0.0f),
		Vector3(0.0f, 0.0f, -1.0f),
		Vector3(0.0f, 0.0f, 1.0f),
		Vector3(-0.707f, -0.707f, 0.0f),
		Vector3(0.707f, -0.707f, 0.0f),
		Vector3(-0.707f, 0.707f, 0.0f),
		Vector3(0.707f, 0.707f, 0.0f),
		Vector3(-0.707f, 0.0f, -0.707f),
		Vector3(0.707f, 0.0f, -0.707f),
		Vector3(-0.707f, 0.0f, 0.707f),
		Vector3(0.707f, 0.0f, 0.707f),
		Vector3(0.0f, -0.707f, -0.707f),
		Vector3(0.0f, 0.707f, -0.707f),
		Vector3(0.0f, -0.707f, 0.707f),
		Vector3(0.0f, 0.707f, 0.707f)
	};

	static unsigned int indexData[] =
	{
		0, 10, 10, 4, 4, 11, 11, 1, 1, 13, 13, 5, 5, 12, 12, 0,
		0, 6, 6, 2, 2, 7, 7, 1, 1, 9, 9, 3, 3, 8, 8, 0,
		2, 14, 14, 4, 4, 15, 15, 3, 3, 17, 17, 5, 5, 16, 16, 2
	};

	std::array<Vector2, _countof(vertexData)> vertexScreen;
	bool isVisible = true;
	for (size_t i = 0; i < vertexScreen.size() && isVisible; ++i)
	{
		Vector3 worldPosition = Vector3::Transform(vertexData[i] * m_radius + m_offset, GetTransform()->GetWorldSRTMatrix());

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