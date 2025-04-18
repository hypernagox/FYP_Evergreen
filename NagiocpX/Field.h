#pragma once
#include "Cluster.h"

namespace NagiocpX
{
	class Cluster;

	class Field
		:public RefCountable
	{
	public:
		virtual ~Field() = default;
	public:
		virtual void InitFieldGlobal()noexcept = 0;
		virtual void InitFieldTLS()noexcept = 0;
		virtual void DestroyFieldTLS()noexcept;
		virtual void MigrationAfterBehavior(Field* const prev_field)noexcept = 0;
	public:
		void EnterFieldNPC(const S_ptr<ContentsEntity>& pEntity_)noexcept {
			const auto entity_ptr = pEntity_.get();
			Mgr(FieldMgr)->RegisterNPC(const_cast<S_ptr<ContentsEntity>&>(pEntity_));
			if (pEntity_)return;
			EnterField(entity_ptr);
		}
		void EnterFieldWithXYNPC(const uint8_t start_x, const uint8_t start_y, const S_ptr<ContentsEntity>& pEntity_)noexcept {
			const auto entity_ptr = pEntity_.get();
			Mgr(FieldMgr)->RegisterNPC(const_cast<S_ptr<ContentsEntity>&>(pEntity_));
			if (pEntity_)return;
			EnterFieldWithXY(start_x, start_y, entity_ptr);
		}
		void EnterField(ContentsEntity* const pEntity_)noexcept {
			const auto cluster = GetCluster(m_start_x, m_start_y);
			pEntity_->IncRefEnterCluster();
			pEntity_->SetClusterFieldInfoUnsafe(cluster->GetClusterFieldInfo());
			std::atomic_thread_fence(std::memory_order_release);
			cluster->EnterEnqueue(pEntity_);
		}
		void EnterFieldWithXY(const uint8_t start_x, const uint8_t start_y, ContentsEntity* const pEntity_)noexcept {
			const auto cluster = GetCluster(start_x, start_y);
			pEntity_->IncRefEnterCluster();
			pEntity_->SetClusterFieldInfoUnsafe(cluster->GetClusterFieldInfo());
			std::atomic_thread_fence(std::memory_order_release);
			cluster->EnterEnqueue(pEntity_);
		}
	public:
		template <typename T = Cluster>
		T* const GetCluster(const Point2D sectorXY)const noexcept { 
			return static_cast<T* const>(tl_vecClusters[GetCurThreadIdx()][sectorXY.y][sectorXY.x]);
		}
		template <typename T = Cluster>
		T* const GetCluster(const uint8_t x, const uint8_t y)const noexcept { 
			return static_cast<T* const>(tl_vecClusters[GetCurThreadIdx()][y][x]);
		}
		template <typename T = Cluster>
		T* const GetStartCluster()const noexcept {
			return GetCluster<T>(m_start_x, m_start_y);
		}
		inline const Point2D CalculateClusterXY(const float x, const float y)const noexcept {
			return Point2D{ static_cast<const uint8_t>(static_cast<const int32_t>(x) / cluster_x)
				, static_cast<const uint8_t>(static_cast<const int32_t>(y) / cluster_y) };
		}
		const auto GetNumOfClustersInField()const noexcept { return m_numOfClusters; }
		const auto GetFieldID()const noexcept { return m_fieldID; }
		const bool IsRunning()const noexcept { return m_bIsRunning; }
		void FinishField()noexcept { m_bIsRunning = false; }	
	private:
		static Field* const GetFieldInternal(const uint8_t fieldID)noexcept;
	public:
		template <typename T = Field>
		static inline T* GetField(const uint8_t fieldID)noexcept {
			return static_cast<T* const>(Field::GetFieldInternal(fieldID));
		}
	protected:
		XVector<Cluster*>* tl_vecClusters[NUM_OF_THREADS] = { nullptr };
		uint32_t m_numOfClusters = 0;
		int8_t m_fieldID = 0;
		bool m_bIsRunning = true;
		const uint8_t m_start_x = 0;
		const uint8_t m_start_y = 0;
		int field_x = 1;
		int field_y = 1;
		int cluster_x = 1;
		int cluster_y = 1;
	};
}