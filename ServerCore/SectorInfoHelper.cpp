#include "ServerCorePch.h"
#include "SectorInfoHelper.h"
#include "World.h"

namespace ServerCore
{
	void SectorInfoHelper::ImmigrationSector(const float x, const float y) noexcept
	{
		const auto pOwnerEntity = GetOwnerEntityRaw();
		const auto obj_type = pOwnerEntity->GetObjectType();
		const auto obj_id = pOwnerEntity->GetObjectID();
		const auto cur_sector = pOwnerEntity->GetCurSector();
		const auto cur_world = cur_sector->GetParentWorld();
		const Point2D next_sector = cur_world->CalculateSectorXY(x, y);

		if (next_sector != m_cur_sectorXY)
		{
			cur_sector->MigrationEnqueue(cur_world->GetSector(next_sector)->SharedFromThis(), obj_type, obj_id);
			m_cur_sectorXY = next_sector;
		}
	}

	Vector<uint32_t> SectorInfoHelper::FillterSessionEntities(const ContentsEntity* const pEntity_) noexcept
	{
		const auto sectors = GetAdjSectors(pEntity_);
		Vector<uint32_t> id_vec;
		const auto huristic_func = MoveBroadcaster::g_huristic[0];
		for (const auto sector : sectors)
		{
			const auto& session_vecHash = sector->GetSessionVectorHash();
			const auto& session_lock = session_vecHash.GetSRWLock();
			const auto& session_list = session_vecHash.GetItemListRef();
			session_lock.lock_shared();
			auto b = session_list.data();
			const auto e = b + session_list.size();
			while (e != b) {
				const ContentsEntity* const __restrict pSessionEntity = *b++;
				if (huristic_func(pSessionEntity, pEntity_))
					id_vec.emplace_back(pSessionEntity->GetObjectID());
			}
			session_lock.unlock_shared();
		}
		return id_vec;
	}
	void SectorInfoHelper::BroadcastWithID(const Vector<uint32_t>& id, const S_ptr<SendBuffer>& pkt_) noexcept
	{
		auto b = id.data();
		const auto e = b + id.size();
		while (e != b)
		{
			if (const auto session = GetSession((*b++)))
				session->GetSession()->SendAsync(pkt_);
		}
	}
	const Vector<Sector*> SectorInfoHelper::GetAdjSectors(const ContentsEntity* const pEntity_) noexcept
	{
		return Vector<Sector*>{pEntity_->GetCurSector()};
	}
}