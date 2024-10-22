#include "ServerCorePch.h"
#include "ClusterUpdateQueue.h"
#include "Field.h"
#include "Cluster.h"
#include "WorldMgr.h"

namespace ServerCore
{
	Cluster* GetCluster(const ClusterInfo info) noexcept
	{
		return Mgr(WorldMgr)->GetCluster(info.fieldID, info.clusterID);
	}
}