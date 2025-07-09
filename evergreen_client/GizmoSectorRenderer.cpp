#include "pch.h"
#include "GizmoSectorRenderer.h"

using namespace udsdx;

void GizmoSectorRenderer::OnDrawGizmos(const udsdx::Camera* target)
{
	// TODO: 시야가 반대로 렌더링 된다.
	// 이 문제는 NPC, 캐릭터, 몬스터 등 모든 오브젝트가 Z- 방향으로 바라보고 있는 상태로 import 되었기 때문이다.
	// 모든 모델을 180도 회전시키는것으로 해결하는 것이나, 고쳐야 할 코드가 많으므로 우선순위를 낮게 둔다.

	float radian = m_angle * DEG2RAD;

	Vector3 vertexData[20];

	vertexData[0] = Vector3(0.0f, 0.0f, 0.0f);
	vertexData[10] = Vector3(0.0f, 1.0f, 0.0f);
	for (int index = 1; index < 10; ++index)
	{
		float angle = (index - 5) * 0.125f * radian;
		vertexData[index] = Vector3(sin(angle), 0.0f, cos(angle));
		vertexData[index + 10] = Vector3(sin(angle), 1.0f, cos(angle));
	}

	static unsigned int indexData[] =
	{
		0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 0,
		10, 11, 11, 12, 12, 13, 13, 14, 14, 15, 15, 16, 16, 17, 17, 18, 18, 19, 19, 10,
		0, 10, 1, 11, 2, 12, 3, 13, 4, 14, 5, 15, 6, 16, 7, 17, 8, 18, 9, 19
	};

	std::array<Vector2, _countof(vertexData)> vertexScreen;
	bool isVisible = true;
	for (size_t i = 0; i < vertexScreen.size() && isVisible; ++i)
	{
		Vector3 worldPosition = Vector3::Transform(vertexData[i] * Vector3(m_radius, 1.0f, m_radius), GetTransform()->GetWorldSRTMatrix());

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
