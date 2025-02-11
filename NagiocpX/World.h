#pragma once
#include "ServerCorePch.h"
#include "TaskQueueable.h"
#include "WorldMgr.h"
#include "SectorInfoHelper.h"

namespace ServerCore
{
	class Sector;
	class ContentsEntity;

	class World
		:public TaskQueueable
	{
	public:
		//World(const uint8_t start_x = 0, const uint8_t start_y = 0)noexcept :m_start_x{ start_x }, m_start_y{ m_start_y } {}
		World()noexcept = default;
		virtual ~World();
	public:
		virtual void InitWorld()noexcept = 0;
		virtual void Dispatch(ServerCore::IocpEvent* const iocpEvent_, const int32_t numOfBytes)noexcept = 0;
		virtual void MigrationWolrdAfterBehavior(const ServerCore::S_ptr<ServerCore::World> prevWorld, ServerCore::ContentsEntity* const pEntity_)noexcept = 0;
	public:
		void MigrationWorld(S_ptr<World> destWorld, ContentsEntity* const pEntity_)noexcept {
			const auto cur_sector = pEntity_->GetCurSector();
			cur_sector->MigrationWorldEnqueue(S_ptr<World>{this}, std::move(destWorld), pEntity_);
		}
		void MigrationWorld(S_ptr<World> destWorld, ContentsEntity* const pEntity_,const uint8_t start_x, const uint8_t start_y)noexcept {
			const auto cur_sector = pEntity_->GetCurSector();
			cur_sector->MigrationWorldEnqueue(S_ptr<World>{this}, std::move(destWorld), pEntity_);
		}
	public:
		template <typename T = World>
		constexpr inline S_ptr<T> SharedFromThis()const noexcept { return S_ptr<T>{this}; }
	public:
		const S_ptr<Sector>& GetStartSector()const noexcept { return m_vecSectors[m_start_y][m_start_x]; }
	public:
		void EnterWorldNPC(const S_ptr<ContentsEntity>& pEntity_)noexcept {
			const auto entity_ptr = pEntity_.get();
			Mgr(WorldMgr)->RegisterNPC(const_cast<S_ptr<ContentsEntity>&>(pEntity_));
			if (pEntity_)return;
			EnterWorld(entity_ptr);
		}
		void EnterWorldWithXYNPC(const uint8_t start_x, const uint8_t start_y, const S_ptr<ContentsEntity>& pEntity_)noexcept { 
			const auto entity_ptr = pEntity_.get();
			Mgr(WorldMgr)->RegisterNPC(const_cast<S_ptr<ContentsEntity>&>(pEntity_));
			if (pEntity_)return;
			EnterWorldWithXY(start_x, start_y, entity_ptr);
		}
		void EnterWorld(ContentsEntity* const pEntity_)noexcept {
			IncRef();
			if (const auto sector = pEntity_->GetCurSector())
			{
				sector->GetParentWorld()->DecRef();
			}
			else
			{
				pEntity_->IncRef();
			}
			pEntity_->GetComp<ServerCore::SectorInfoHelper>()->SetSectorXY(m_start_x, m_start_y);
			m_vecSectors[m_start_y][m_start_x]->EnterEnqueue(pEntity_);
		}
		void EnterWorldWithXY(const uint8_t start_x, const uint8_t start_y, ContentsEntity* const pEntity_)noexcept {
			IncRef();
			if (const auto sector = pEntity_->GetCurSector())
			{
				sector->GetParentWorld()->DecRef();
			}
			else
			{
				pEntity_->IncRef();
			}
			pEntity_->GetComp<SectorInfoHelper>()->SetSectorXY(start_x, start_y);
			m_vecSectors[start_y][start_x]->EnterEnqueue(pEntity_);
		}
		void EndWorldEnqueue()noexcept { EnqueueAsync(&World::EndWorld); }

		void RegisterContentsEntity(ContentsEntity* const pEntity_)noexcept {
			tbb::concurrent_hash_map<uint32_t, S_ptr<ContentsEntity>>::accessor ac;
			m_mapContentsEntity.emplace(ac, pEntity_->GetObjectID(), pEntity_);
		}
		void RemoveContentsEntity(const uint32_t obj_id)noexcept { m_mapContentsEntity.erase(obj_id); }
		S_ptr<ContentsEntity> FindEntity(const uint32_t obj_id)const noexcept {
			tbb::concurrent_hash_map<uint32_t, S_ptr<ContentsEntity>>::const_accessor cac;
			return m_mapContentsEntity.find(cac, obj_id) ? cac->second : nullptr;
		}
		inline const Point2D CalculateSectorXY(const float x, const float y)const noexcept {
			return Point2D{ static_cast<const uint8_t>(static_cast<const int32_t>(x) / sector_x)
				, static_cast<const uint8_t>(static_cast<const int32_t>(y) / sector_y) };
		}

		template <typename T = Sector>
		T* const GetSector(const Point2D sectorXY)const noexcept { return static_cast<T* const>(m_vecSectors[sectorXY.y][sectorXY.x].get()); }
		template <typename T = Sector>
		T* const GetSector(const uint8_t x, const uint8_t y)const noexcept { return static_cast<T* const>(m_vecSectors[y][x].get()); }
	private:
		void EndWorld()noexcept;
	protected:
		Vector<Vector<S_ptr<Sector>>> m_vecSectors;
	private:
		const uint8_t m_start_x = 0;
		const uint8_t m_start_y = 0;
		int world_x = 1;
		int world_y = 1;
		int sector_x = 1;
		int sector_y = 1;
		
		tbb::concurrent_hash_map<uint32_t, S_ptr<ContentsEntity>> m_mapContentsEntity;
	};
}

