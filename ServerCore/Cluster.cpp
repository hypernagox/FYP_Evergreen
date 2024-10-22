#include "ServerCorePch.h"
#include "Cluster.h"

namespace ServerCore
{
	void Cluster::Enter(const uint8_t group_type, const uint32_t obj_id, ContentsEntity* const pEntity_) noexcept
	{
		auto& target_vecHash = *(m_vectorHashMapForEntity.data() + group_type);

		if (false == pEntity_->IsValid())
		{
			return;
		}

		if (!target_vecHash.AddItem(obj_id, pEntity_))
		{
			// TODO: 이미 방에 있는데 또 들어오려한거임
			// std::cout << "Alread Exist in Room" << std::endl;
			return;
		}
	}
	void Cluster::LeaveAndDestroy(const uint8_t group_type, const uint32_t obj_id) noexcept
	{
		if (const auto entity = m_vectorHashMapForEntity[group_type].ExtractItem(obj_id))
		{
			if(1 == entity->m_curRefCluster.fetch_sub(1))
				entity->Destroy();
			entity->DecRef();
		}
	}
	void Cluster::Migration(const ClusterInfo info, const uint8_t group_type, const uint32_t obj_id) noexcept
	{
		if (const auto entity = m_vectorHashMapForEntity[group_type].ExtractItem(obj_id))
		{
			GetCluster(info)->Enter(group_type, obj_id, entity);
		}
	}
}