#include "NagiocpXPch.h"
#include "ClusterInfoHelper.h"
#include "Service.h"
#include "Cluster.h"
#include "MoveBroadcaster.h"
#include "Field.h"

namespace NagiocpX
{
	void ClusterInfoHelper::FillterSessionEntities(XVector<const ContentsEntity*>& vec_, const ContentsEntity* const pEntity_) noexcept
	{
		// TODO: 몹의 시야
		vec_.clear();
		const auto clusters = ClusterInfoHelper::GetAdjClusters(pEntity_);
		
		for (const auto cluster : clusters)
		{
			const auto& sessions = cluster->GetSessions();
			auto b = sessions.data();
			const auto e = b + sessions.size();
			while (e != b) {
				const auto s = *b++;
				if (MoveBroadcaster::GlobalFilter4Session(pEntity_, s))vec_.emplace_back(s);
			}
		}
	}

	void ClusterInfoHelper::BroadcastWithID(const XVector<uint32_t>& id, const S_ptr<SendBuffer>& pkt_) noexcept
	{
		const auto service = Service::GetMainService();
		auto b = id.data();
		const auto e = b + id.size();
		while (e != b)
		{
			if (const auto session = service->GetSession((*b++)))
				session->GetSession()->SendAsync(pkt_);
		}
	}

	const XVector<Cluster*> ClusterInfoHelper::GetAdjClusters(const ContentsEntity* const pEntity_) noexcept
	{
		// TODO: 몹의 시야, 객체별 시야 (float 인자 하나 더주기)
		return g_clusterFilterFunc(pEntity_, pEntity_->GetCurField());
	}

	const XVector<Cluster*> ClusterInfoHelper::GetAllAdjClusters(const ContentsEntity* const pEntity_) noexcept
	{
		return g_getAllClusterFunc(pEntity_, pEntity_->GetCurField());
	}

	bool ClusterInfoHelper::AdjustCluster(const float new_x, const float new_z) noexcept
	{
		const auto owner = GetOwnerEntityRaw();
		if (owner->IsPendingClusterEntry())return false;
		const auto cur_field = owner->GetCurField();
		const auto cluster_field_info = owner->GetClusterFieldInfo();
		const auto cur_xy = cur_field->CalculateClusterXY(new_x, new_z);
		if (cluster_field_info.clusterInfo.clusterID == cur_xy)
		{
			return false;
		}
		else
		{
			const auto info = ClusterInfo{ cur_field->GetFieldID() ,cur_xy };
			const auto cluster = owner->GetCurCluster();
			owner->SetOnlyClusterInfo(info);
			cluster->MigrationEnqueue(info, owner);
			return true;
		}
	}

	void ClusterInfoHelper::BroadcastCluster(const S_ptr<SendBuffer>& pkt_, const XVector<Cluster*>&& clusters)noexcept
	{
		for (const auto cluster : clusters)
		{
			cluster->Broadcast(pkt_);
		}
	}
}