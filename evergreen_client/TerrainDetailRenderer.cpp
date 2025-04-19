#include "pch.h"
#include "TerrainDetailRenderer.h"
#include "TerrainDetail.h"

using namespace udsdx;

TerrainDetailRenderer::TerrainDetailRenderer(const std::shared_ptr<udsdx::SceneObject>& object) : RendererBase(object)
{
	m_castShadow = false;
}

void TerrainDetailRenderer::Render(udsdx::RenderParam& param, int instances)
{
	ObjectConstants objectConstants;
	objectConstants.World = m_transformCache.Transpose();
	objectConstants.PrevWorld = m_prevTransformCache.Transpose();

	param.CommandList->SetGraphicsRoot32BitConstants(RootParam::PerObjectCBV, sizeof(ObjectConstants) / 4, &objectConstants, 0);

	Mesh* mesh = m_terrainDetail->GetMesh();
	UINT indexBase = m_terrainDetail->GetIndexBase(8, 8);
	UINT indexCount = m_terrainDetail->GetIndexCount(8, 8);

	D3D12_VERTEX_BUFFER_VIEW vbv = mesh->VertexBufferView();
	D3D12_INDEX_BUFFER_VIEW ibv = mesh->IndexBufferView();

	param.CommandList->IASetVertexBuffers(0, 1, &vbv);
	param.CommandList->IASetIndexBuffer(&ibv);
	param.CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);

	Vector3 cameraPosition = param.TargetCamera->GetTransform()->GetWorldPosition();
	Vector3 cameraTerrainPosition = (cameraPosition - GetTransform()->GetLocalPosition()) / GetTransform()->GetLocalScale();
	UINT segmentation = m_terrainDetail->GetSegmentation();
	int cameraSegmentX = static_cast<int>(cameraTerrainPosition.x * segmentation);
	int cameraSegmentY = static_cast<int>(cameraTerrainPosition.z * segmentation);

	const udsdx::Texture* texture = m_materials[0]->GetSourceTexture(0);
	param.CommandList->SetGraphicsRootDescriptorTable(RootParam::SrcTexSRV_0, texture->GetSrvGpu());

	BoundingBox boundingBox;
	boundingBox.Extents = Vector3(0.5f / segmentation, 0.5f, 0.5f / segmentation) * GetTransform()->GetLocalScale();
	for (int segmentX = std::max<int>(cameraSegmentX - 1, 0); segmentX <= std::min<int>(cameraSegmentX + 1, segmentation - 1); ++segmentX)
	{
		for (int segmentY = std::max<int>(cameraSegmentY - 1, 0); segmentY <= std::min<int>(cameraSegmentY + 1, segmentation - 1); ++segmentY)
		{
			boundingBox.Center = Vector3((segmentX + 0.5f) / segmentation, 0.5f, (segmentY + 0.5f) / segmentation) * GetTransform()->GetLocalScale() + GetTransform()->GetLocalPosition();
			if (!param.ViewFrustumWorld->Contains(boundingBox))
				continue;
			UINT indexBase = m_terrainDetail->GetIndexBase(segmentX, segmentY);
			UINT indexCount = m_terrainDetail->GetIndexCount(segmentX, segmentY);
			if (indexCount > 0)
				param.CommandList->DrawIndexedInstanced(indexCount, instances, indexBase, 0, 0);
		}
	}
}
