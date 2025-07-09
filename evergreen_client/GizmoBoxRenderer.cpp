#include "pch.h"
#include "GizmoBoxRenderer.h"

using namespace udsdx;

void GizmoBoxRenderer::OnDrawGizmos(const udsdx::Camera* target)
{
	static Vector3 vertexData[] =
	{
		Vector3(-0.5f, -0.5f, -0.5f),
		Vector3(-0.5f, -0.5f, 0.5f),
		Vector3(-0.5f, 0.5f, -0.5f),
		Vector3(-0.5f, 0.5f, 0.5f),
		Vector3(0.5f, -0.5f, -0.5f),
		Vector3(0.5f, -0.5f, 0.5f),
		Vector3(0.5f, 0.5f, -0.5f),
		Vector3(0.5f, 0.5f, 0.5f)
	};

	static unsigned int indexData[] =
	{
		0, 1, 1, 3, 3, 2, 2, 0,
		4, 5, 5, 7, 7, 6, 6, 4,
		0, 4, 1, 5, 2, 6, 3, 7
	};

	std::array<Vector2, _countof(vertexData)> vertexScreen;
	bool isVisible = true;
	for (size_t i = 0; i < vertexScreen.size() && isVisible; ++i)
	{
		Vector3 worldPosition = Vector3::Transform(vertexData[i] * m_size + m_offset, GetTransform()->GetWorldSRTMatrix());

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
