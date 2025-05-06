#include "pch.h"
#include "QuestRoom.h"
#include "EntityFactory.h"
#include "Regenerator.h"
#include "PartyQuestSystem.h"
#include "ClientSession.h"
#include "TaskTimerMgr.h"
#include "Navigator.h"
#include "NavigationMesh.h"
#include "PathNPC.h"
#include "PositionComponent.h"

std::atomic_int aaaa;
QuestRoom::QuestRoom() noexcept
{
	m_field_x_scale = 1024;
	m_field_y_scale = 1024;

	m_cluster_x_scale = m_field_x_scale;
	m_cluster_y_scale = m_field_y_scale;

	InitFieldGlobal();
	InitFieldTLS();
	std::cout << ++aaaa << std::endl;

}

QuestRoom::~QuestRoom() noexcept
{
	const auto row = (size_t)GetNumOfClusterRow();

	for (int i = 0; i < NagiocpX::NUM_OF_THREADS; ++i)
	{
		const std::span<XVector<NagiocpX::Cluster*>> clusters{ tl_vecClusters[i], row };

		for (const auto cluster : clusters | std::views::join)
		{
			for (auto& entities : cluster->GetEntitesExceptSession())
			{
				for (auto* entity : entities.GetItemListRef())
				{
					entity->TryOnDestroy();
				}
			}
			NagiocpX::xdelete<NagiocpX::Cluster>(cluster);
		}
		NagiocpX::DeleteJEMallocArray(clusters);
	}
	for (const auto entity : m_members)
	{
		if (!entity)continue;
		entity->DecRef();
	}
	std::cout << "퀘스트룸 소멸\n";
	std::cout << --aaaa << std::endl;
	m_ownerPartrySystem->EndFlag();
}

void QuestRoom::NotifyQuestClear(NagiocpX::ContentsEntity* const entity) const noexcept
{
	entity->GetSession()->SendAsync(Create_s2c_PARTY_QUEST_CLEAR(m_ownerPartrySystem->m_curQuestID));
}

void QuestRoom::NotifyQuestFail(NagiocpX::ContentsEntity* const entity) const noexcept
{
	// TODO: 알리기

}

void QuestRoom::InitFieldGlobal() noexcept
{
	m_fieldID = -1;
}

void QuestRoom::InitFieldTLS() noexcept
{
	const auto row = GetNumOfClusterRow();
	const auto col = GetNumOfClusterCol();

	for (int i = 0; i < NagiocpX::NUM_OF_THREADS; ++i)
	{
		const auto clusters = NagiocpX::CreateJEMallocArray<XVector<NagiocpX::Cluster*>>(row);
		tl_vecClusters[i] = clusters.data();
		for (uint8 r = 0; r < row; ++r)
		{
			tl_vecClusters[i][r].reserve(col);
			for (uint8 c = 0; c < col; ++c)
			{
				tl_vecClusters[i][r].emplace_back(NagiocpX::xnew<NagiocpX::Cluster>(
					NUM_OF_GROUPS,
					NagiocpX::ClusterInfo{ m_fieldID, c, r },
					this
				));
			}
		}
	}
}

void QuestRoom::DestroyFieldTLS() noexcept
{
	const auto row = (size_t)GetNumOfClusterRow();

	for (int i = 0; i < NagiocpX::NUM_OF_THREADS; ++i)
	{
		const std::span<XVector<NagiocpX::Cluster*>> clusters{ tl_vecClusters[i], row };

		for (const auto  cluster : clusters | std::views::join)
		{
			for (auto& entities : cluster->GetEntitesExceptSession())
			{
				for (auto* entity : entities.GetItemListRef())
				{
					entity->TryOnDestroy();
				}
			}
		}
	}
}

void QuestRoom::MigrationAfterBehavior(Field* const prev_field) noexcept
{
	std::cout << "멤버카운트 " << (int)m_numOfMember << std::endl;
	std::cout << "성공\n";
}

void QuestRoom::DecMemberCount() noexcept
{
	const auto count = m_numOfMember.fetch_sub(1);
	std::cout << (int)count << std::endl;
	if (1 == count)
	{
		std::cout << "삭제시작\n";
		DestroyFieldTLS();
		m_ownerPartrySystem->m_curQuestRoomInstance.reset();
	}
}

void QuestRoom::CheckPartyQuestState()noexcept
{
	// TODO: 락 고려
	if (ProcessPartyQuest())
	{
		// TODO 근본적인 해결책
		Mgr(TaskTimerMgr)->ReserveAsyncTask(1000,[this, owner = m_ownerPartrySystem->m_member[0]]() {
			for (const auto& players : owner->m_party_quest_system->m_member)
			{
				if (!players)continue;
				NotifyQuestClear(players->GetOwnerEntity());
			}
			m_isClear.store(true);
			//m_ownerPartrySystem->GetPartyLeader()->m_cur_my_party_system.load()->MissionEnd();
			});
		return;
	}
	else if (IsFailPartyQuest())
	{
		for (const auto& players : m_ownerPartrySystem->m_member)
		{
			if (!players)continue;
			NotifyQuestFail(players->GetOwnerEntity());
		}
		return;
	}
}

void QuestRoom::RegisterMember(const uint32_t idx, ContentsEntity* const entity) noexcept
{
	const auto id = entity->GetObjectID();
	m_id2idx_table.try_emplace(id, idx);
	m_members[idx] = entity;
	entity->IncRef();
}

void FoxQuest::InitQuestField() noexcept
{
	for (int i = 0; i < 500; ++i)
	{
		EntityBuilder b;
		b.group_type = Nagox::Enum::GROUP_TYPE::GROUP_TYPE_MONSTER;
		b.obj_type = MONSTER_TYPE_INFO::FOX;
		const auto m = EntityFactory::CreateMonster(b);
		static_cast<Regenerator*>(m->GetDeleter())->m_targetField = SharedFromThis<NagiocpX::Field>();
		const auto pos = m->GetComp<PositionComponent>()->pos;
		EnterFieldWithFloatXYNPC(pos.x + 512.f, pos.z + 512.f, m);
		//EnterFieldNPC(m);
	}
}

void GoblinQuest::InitQuestField() noexcept
{
	for (int i = 0; i < 500; ++i)
	{
		EntityBuilder b;
		b.group_type = Nagox::Enum::GROUP_TYPE_NPC;
		b.obj_type = 0;
		const auto m = EntityFactory::CreateRangeMonster(b);
		static_cast<Regenerator*>(m->GetDeleter())->m_targetField = SharedFromThis<NagiocpX::Field>();

		const auto pos = m->GetComp<PositionComponent>()->pos;
		EnterFieldWithFloatXYNPC(pos.x + 512.f, pos.z + 512.f, m);
		//EnterFieldNPC(m);
	}
}

void NPCGuardQuest::InitQuestField() noexcept
{
	EntityBuilder b;
	b.group_type = Nagox::Enum::GROUP_TYPE_MONSTER;
	b.obj_type = 0;
	const auto m = EntityFactory::CreatePathNPC(b);
	const auto m2 = m;

	const Vector3 begin = { -19.601448f,  72.97739f,  0.74976814f };
	const Vector3 end = { -119.499115f,75,13.64f }; // 마을 중앙

	EnterFieldWithFloatXYNPC(begin.x + 512.f, begin.z + 512.f, m);
	//EnterFieldNPC(m);
	// TODO: 위험
	m2->GetComp<PathNPC>()->m_owner_system = GetOwnerSystem();
	m2->GetComp<PathNPC>()->InitPathNPC();
}

