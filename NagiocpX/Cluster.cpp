#include "NagiocpXPch.h"
#include "Cluster.h"
#include "Field.h"

namespace NagiocpX
{
	Cluster::~Cluster() noexcept
	{
		DeleteJEMallocArray(m_vectorHashMapForEntity);
	}

	void Cluster::Broadcast(const S_ptr<SendBuffer>& pkt_)const noexcept
	{
		const auto& sessions = m_vectorHashMapForEntity.data()->GetItemListRef();
		auto b = sessions.data();
		const auto e = b + sessions.size();
		while (e != b) { (*b++)->GetSession()->SendAsync(pkt_); }
	}

	Cluster::EntityState Cluster::Enter(const uint8_t group_type, const uint32_t obj_id, ContentsEntity* const pEntity_) noexcept
	{
		auto& target_vecHash = *(m_vectorHashMapForEntity.data() + group_type);
		
		if (false == pEntity_->IsValid())
		{
			pEntity_->DecRef();
			return Cluster::EntityState::FAIL;
		}
		if (!target_vecHash.AddItem(obj_id, pEntity_))
		{
			// TODO: 이미 방에 있는데 또 들어오려한거임
			PrintLogEndl("Alread Exist in Space");
			return Cluster::EntityState::FAIL;
		}

		m_parentField->IncRef();

		return (Cluster::EntityState)pEntity_->RegisterEnterCount();
	}
	void Cluster::LeaveAndDestroy(const uint8_t group_type, const uint32_t obj_id) noexcept
	{
		if (const auto entity = m_vectorHashMapForEntity[group_type].ExtractItem(obj_id))
		{
			entity->DecRef();
			m_parentField->DecRef<Field>();
		}
	}
	void Cluster::Migration(const ClusterInfo info, const uint8_t group_type, const uint32_t obj_id) noexcept
	{
		if (const auto entity = m_vectorHashMapForEntity[group_type].ExtractItem(obj_id))
		{
			//entity->SetOnlyClusterInfo(info);
			GetCluster(info, m_parentField)->Enter(group_type, obj_id, entity);
		}
	}
	void Cluster::MigrationOtherField(Field* const other_field, const uint8_t group_type, const uint32_t obj_id) noexcept
	{
		const auto other_cluster = other_field->GetStartCluster();
		if (const auto entity = m_vectorHashMapForEntity[group_type].ExtractItem(obj_id))
		{
			if (Cluster::EntityState::ENTER == other_cluster->Enter(group_type, obj_id, entity))
			{
				entity->SetClusterFieldInfoUnsafe(other_cluster->GetClusterFieldInfo());
				other_field->MigrationAfterBehavior(m_parentField);
			}
			m_parentField->DecRef<Field>();
		}
	}
}