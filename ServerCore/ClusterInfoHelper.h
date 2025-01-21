#pragma once
#include "ContentsComponent.h"

namespace ServerCore
{
	class Cluster;

	class ClusterInfoHelper
		: public ContentsComponent
	{
	public:
		CONSTRUCTOR_CONTENTS_COMPONENT(ClusterInfoHelper)
	public:
		void SetClusterXY(const uint8_t x, const uint8_t y)noexcept { m_cur_sectorXY = { x,y }; }
		const Point2D GetCurXY()const noexcept { return m_cur_sectorXY; }
	public:
		static void FillterSessionEntities(XVector<const ContentsEntity*>& vec_, const ContentsEntity* const pEntity_)noexcept;
		static void BroadcastWithID(const XVector<uint32_t>& id, const S_ptr<SendBuffer>& pkt_)noexcept;
		void BroadcastCluster(const S_ptr<SendBuffer>& pkt_)noexcept;
		// TODO: ������ �˻��� ���͸� �� ���� �޸���ƽ �����
		// TODO: �Լ������� ����� �������
		static const XVector<Cluster*> GetAdjClusters(const ContentsEntity* const pEntity_)noexcept;
		//static void SetSectorFillterHeuristic(const auto fp)noexcept { g_sectorFillterFunc = fp; }
	private:
		Point2D m_cur_sectorXY;

	private:
		//using SectorFillter = XVector<Sector*>(*)(const ContentsEntity* const)noexcept;

		//static inline SectorFillter g_sectorFillterFunc = {};
	};
}

