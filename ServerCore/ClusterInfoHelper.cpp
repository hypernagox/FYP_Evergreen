#include "ServerCorePch.h"
#include "ClusterInfoHelper.h"
#include "Service.h"
#include "Cluster.h"
#include "MoveBroadcaster.h"

namespace ServerCore
{
	void ClusterInfoHelper::FillterSessionEntities(Vector<const ContentsEntity*>& vec_, const ContentsEntity* const pEntity_) noexcept
	{
		vec_.clear();
		const auto clusters = ClusterInfoHelper::GetAdjClusters(pEntity_);
		const auto huristic = MoveBroadcaster::g_huristic[0];

		for (const auto cluster : clusters)
		{
			const auto& sessions = cluster->GetSessions();
			auto b = sessions.data();
			const auto e = b + sessions.size();
			while (e != b) {
				const auto s = *b++;
				if (huristic(pEntity_, s))vec_.emplace_back(s);
			}
		}
	}

	void ClusterInfoHelper::BroadcastWithID(const Vector<uint32_t>& id, const S_ptr<SendBuffer>& pkt_) noexcept
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
	void ClusterInfoHelper::BroadcastCluster(const S_ptr<SendBuffer>& pkt_) noexcept
	{
		const auto clusters = ClusterInfoHelper::GetAdjClusters(GetOwnerEntityRaw());
		for (const auto cluster : clusters)
		{
			cluster->Broadcast(pkt_);
		}
	}
	const Vector<Cluster*> ClusterInfoHelper::GetAdjClusters(const ContentsEntity* const pEntity_) noexcept
	{
		return Vector<Cluster*>{pEntity_->GetCurCluster()};
	}
}