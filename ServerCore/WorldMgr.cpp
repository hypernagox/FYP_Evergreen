#include "ServerCorePch.h"
#include "WorldMgr.h"
#include "World.h"

namespace ServerCore
{
	WorldMgr::WorldMgr()
	{
	}

	WorldMgr::~WorldMgr()
	{
		
		ClearWorld();
	}

	void WorldMgr::ClearWorld()const noexcept
	{
		{
			auto b = m_arrNPC.data();
			const auto e = b + m_arrNPC.size();
			while (e != b)
			{
				auto& atomic_ptr = (*b++).ptr;
				if (const auto target = atomic_ptr.load())
				{
					target->TryOnDestroy();
					atomic_ptr.reset();
				}
			}
		}
		{
			auto b = m_mapWorld.cbegin();
			const auto e = m_mapWorld.cend();
			while (e != b) { (*b++).second->EndWorldEnqueue(); }
		}
	}
	void WorldMgr::RegisterNPC(S_ptr<ContentsEntity>& pNPC) noexcept
	{
		const uint32_t obj_id = static_cast<c_uint32>(pNPC->GetObjectID());
		int32 idx;
		if (!m_idxQueue.try_pop(idx))
			return;
		m_id2Index.emplace(static_cast<c_uint32>(obj_id), static_cast<c_uint16>(idx));
		m_arrNPC[idx].ptr.store(std::move(pNPC));
	}
	void WorldMgr::ReleaseNPC(const ContentsEntity* const pNPC) noexcept
	{
		const uint16_t idx = m_id2Index[pNPC->GetObjectID()];
		if (0 == idx)
			return;
		m_arrNPC[idx].ptr.reset();
		m_idxQueue.emplace(idx);
	}
	S_ptr<ContentsEntity> WorldMgr::GetNPC(const uint32_t npc_id) const noexcept
	{
		const uint16_t idx = m_id2Index[static_cast<c_uint32>(npc_id)];
		auto target = m_arrNPC[idx].ptr.load();
		if (target && target->GetObjectID() == npc_id)
			return target;
		else
			return nullptr;
	}
}