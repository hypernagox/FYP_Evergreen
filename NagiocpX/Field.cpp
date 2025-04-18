#include "NagiocpXPch.h"
#include "Field.h"
#include "Cluster.h"
#include "FieldMgr.h"

namespace NagiocpX
{
	void Field::DestroyFieldTLS() noexcept
	{
		const std::span< XVector<NagiocpX::Cluster*>> clusters{ tl_vecClusters[NagiocpX::GetCurThreadIdx()],m_numOfClusters };
		for (const auto cluster : clusters | std::views::join)NagiocpX::xdelete<NagiocpX::Cluster>(cluster);
		NagiocpX::DeleteJEMallocArray(clusters);
	}
	Field* const Field::GetFieldInternal(const uint8_t fieldID) noexcept
	{
		const auto& field_table = Mgr(FieldMgr)->GetFieldTable();
		const auto iter = field_table.find(fieldID);
		return field_table.end() != iter ? iter->second : nullptr;
	}
}