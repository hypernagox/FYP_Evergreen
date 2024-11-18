#include "ServerCorePch.h"
#include "ClusterUpdateQueue.h"
#include "Field.h"
#include "Cluster.h"

namespace ServerCore
{
	Cluster* const GetCluster(const ClusterInfo info) noexcept
	{
		return Mgr(FieldMgr)->GetCluster(info);
	}
}