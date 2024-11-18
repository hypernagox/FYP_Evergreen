#include "ServerCorePch.h"
#include "ClusterInfoHelper.h"
#include "Service.h"
#include "Cluster.h"

namespace ServerCore
{
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