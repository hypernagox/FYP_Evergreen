#pragma once
#include "ContentsComponent.h"

namespace NagiocpX
{
	class Cluster;

	class ClusterInfoHelper
		: public ContentsComponent
	{
	public:
		CONSTRUCTOR_CONTENTS_COMPONENT(ClusterInfoHelper)
	public:
		static void FillterSessionEntities(XVector<const ContentsEntity*>& vec_, const ContentsEntity* const pEntity_)noexcept;
		static void BroadcastWithID(const XVector<uint32_t>& id, const S_ptr<SendBuffer>& pkt_)noexcept;
		void BroadcastCluster(const S_ptr<SendBuffer>& pkt_)noexcept;
		// TODO: ������ �˻��� ���͸� �� ���� �޸���ƽ �����
		// TODO: �Լ������� ����� �������
		static const XVector<Cluster*> GetAdjClusters(const ContentsEntity* const pEntity_)noexcept;
		//static void SetSectorFillterHeuristic(const auto fp)noexcept { g_sectorFillterFunc = fp; }
	public:
		bool AdjustCluster(const float new_x, const float new_z)noexcept;
		bool AdjustCluster(const std::pair<float, float> xy)noexcept { return AdjustCluster(xy.first, xy.second); }

	private:
		using ClusterFillter = XVector<Cluster*>(*)(const ContentsEntity* const, const Field* const)noexcept;
		constinit static inline ClusterFillter g_clusterFillterFunc = {};

	public:
		static void RegisterClusterFilter(const ClusterFillter filter)noexcept { g_clusterFillterFunc = filter; }
	};
}

