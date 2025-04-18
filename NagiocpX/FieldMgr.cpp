#include "NagiocpXPch.h"
#include "FieldMgr.h"
#include "Field.h"

namespace NagiocpX
{
	FieldMgr::FieldMgr()
	{
	}

	FieldMgr::~FieldMgr()
	{
		for (const auto [key, val] : m_mapField)
		{
			xdelete<Field>(val);
		}
	}

	void FieldMgr::ClearField()const noexcept
	{
		{
			auto b = m_arrNPC.data();
			const auto e = b + m_arrNPC.size();
			while (e != b)
			{
				auto& atomic_ptr = (*b++).ptr;
				if (const auto target = atomic_ptr.load())
				{
					target->ResetDeleter();
					target->TryOnDestroy();
				}
			}
		}
	}
	void FieldMgr::RegisterNPC(S_ptr<ContentsEntity>& pNPC) noexcept
	{
		const uint32_t obj_id = static_cast<c_uint32>(pNPC->GetObjectID());
		int32 idx;
		if (!m_idxQueue.try_pop(idx))
			return;
		m_id2Index.emplace(static_cast<c_uint32>(obj_id), static_cast<c_uint16>(idx));
		m_arrNPC[idx].ptr.store(std::move(pNPC));
	}
	void FieldMgr::ReleaseNPC(const ContentsEntity* const pNPC) noexcept
	{
		const auto idx = m_id2Index[pNPC->GetObjectID()];
		if (0 == idx)
			return;
		
		m_arrNPC[idx].ptr.reset();
		m_idxQueue.emplace(idx);
	}
	void FieldMgr::InitTLSinField()
	{
		//NAGOX_ASSERT_LOG(false == m_mapField.empty(), "Cluster Empty");
		for (const auto field : m_mapField | std::views::values)
		{
			NAGOX_ASSERT_LOG(0 != field->GetNumOfClustersInField(), "Num of Clusters cannot be Zero");
			field->InitFieldTLS();
		}
	}
	void FieldMgr::DestroyTLSinField()
	{
		for (const auto field : m_mapField | std::views::values)
		{ 
			field->FinishField();
			field->DestroyFieldTLS();
		}
	}
	void FieldMgr::ShrinkToFitBeforeStart() noexcept
	{
		for (const auto& entity : m_arrNPC) {
			if (const auto entity_ptr = entity.ptr.load())
				const_cast<ComponentSystem*>(entity_ptr->GetComponentSystem())->ShrinkToFit();
		}
	}
	S_ptr<ContentsEntity> FieldMgr::GetNPC(const uint32_t npc_id) const noexcept
	{
		const uint16_t idx = m_id2Index[static_cast<c_uint32>(npc_id)];
		auto target = m_arrNPC[idx].ptr.load();
		if (target && target->GetObjectID() == npc_id)
			return target;
		else
			return nullptr;
	}
}