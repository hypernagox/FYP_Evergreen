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
		//void EnterFieldNPC(const S_ptr<ContentsEntity>& pEntity_)noexcept;
		void EnterFieldWithXYNPC(const uint8_t start_x, const uint8_t start_y, const S_ptr<ContentsEntity>& pEntity_)noexcept;
		void EnterFieldWithFloatXYNPC(const float start_x, const float start_y, const S_ptr<ContentsEntity>& pEntity_)noexcept;

		//void EnterField(ContentsEntity* const pEntity_)noexcept;
		void EnterFieldWithXY(const uint8_t start_x, const uint8_t start_y, ContentsEntity* const pEntity_)noexcept;

		void EnterFieldWithFloatXY(const float start_x, const float start_y, ContentsEntity* const pEntity_)noexcept;


		void EnterFieldWithFloatXYNPC(const std::pair<float,float> xy, const S_ptr<ContentsEntity>& pEntity_)noexcept {
			EnterFieldWithFloatXYNPC(xy.first, xy.second, pEntity_);
		}
		void EnterFieldWithFloatXY(const std::pair<float, float> xy, ContentsEntity* const pEntity_)noexcept {
			EnterFieldWithFloatXY(xy.first, xy.second, pEntity_);
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
		inline const Point2D CalculateClusterXY(const float x, const float y)const noexcept {
			return Point2D{ static_cast<const uint8_t>(static_cast<const int32_t>(x) / m_cluster_x_scale)
				, static_cast<const uint8_t>(static_cast<const int32_t>(y) / m_cluster_y_scale) };
		}
		const uint8_t GetNumOfClusterRow()const noexcept { return static_cast<uint8_t>(m_field_y_scale / m_cluster_y_scale); }
		const uint8_t GetNumOfClusterCol()const noexcept { return static_cast<uint8_t>(m_field_x_scale / m_cluster_x_scale); }

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
		const auto GetClusterXScale()const noexcept { return m_cluster_x_scale; }
		const auto GetClusterYScale()const noexcept { return m_cluster_y_scale; }
	protected:
		XVector<Cluster*>* tl_vecClusters[NUM_OF_THREADS] = { nullptr };
		int8_t m_fieldID = 0;
		bool m_bIsRunning = true;
		uint16_t m_field_x_scale = 1;
		uint16_t m_field_y_scale = 1;
		uint16_t m_cluster_x_scale = UINT16_MAX;
		uint16_t m_cluster_y_scale = UINT16_MAX;
	};
}