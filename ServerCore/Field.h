#pragma once

namespace ServerCore
{
	class Cluster;

	class Field
	{
	public:
		virtual void InitField()noexcept = 0;
	public:
		template <typename T = Cluster>
		T* const GetCluster(const Point2D sectorXY)const noexcept { 
			return static_cast<T* const>(m_vecClusters[ThreadMgr::GetCurThreadIdx()][sectorXY.y][sectorXY.x]);
		}
		template <typename T = Cluster>
		T* const GetCluster(const uint8_t x, const uint8_t y)const noexcept { 
			return static_cast<T* const>(m_vecClusters[ThreadMgr::GetCurThreadIdx()][y][x]);
		}
		inline const Point2D CalculateClusterXY(const float x, const float y)const noexcept {
			return Point2D{ static_cast<const uint8_t>(static_cast<const int32_t>(x) / cluster_x)
				, static_cast<const uint8_t>(static_cast<const int32_t>(y) / cluster_y) };
		}
	private:
		Vector<Vector<Cluster*>> m_vecClusters[ThreadMgr::NUM_OF_THREADS];
		const uint8_t m_start_x = 0;
		const uint8_t m_start_y = 0;
		int field_x = 1;
		int field_y = 1;
		int cluster_x = 1;
		int cluster_y = 1;
	};
}