#include "pch.h"
#include "QuestRoom.h"
#include "EntityFactory.h"
#include "Regenerator.h"
#include "PartyQuestSystem.h"
#include "ClientSession.h"

std::atomic_int aaaa;
QuestRoom::QuestRoom() noexcept
{
	InitFieldGlobal();
	InitFieldTLS();
	std::cout << ++aaaa << std::endl;
}

QuestRoom::~QuestRoom() noexcept
{
	for (int i = 0; i < NagiocpX::NUM_OF_THREADS; ++i)
	{
		const std::span< XVector<NagiocpX::Cluster*>> clusters{ tl_vecClusters[i],m_numOfClusters };
		for (const auto cluster : clusters | std::views::join)
		{
			for (auto& entities : cluster->GetAllEntites())
			{
				for (const auto entity : entities.GetItemListRef())
				{
					entity->ResetDeleter();
					entity->TryOnDestroy();
				}
			}
			NagiocpX::xdelete<NagiocpX::Cluster>(cluster);
		}
		NagiocpX::DeleteJEMallocArray(clusters);
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
	m_numOfClusters = 1;
	m_fieldID = -1;
}

void QuestRoom::InitFieldTLS() noexcept
{
	for (int i = 0; i < NagiocpX::NUM_OF_THREADS; ++i) 
	{
		const auto clusters = NagiocpX::CreateJEMallocArray<XVector<NagiocpX::Cluster*>>(m_numOfClusters);
		tl_vecClusters[i] = clusters.data();
		tl_vecClusters[i][0].emplace_back(NagiocpX::xnew<NagiocpX::Cluster>(NUM_OF_GROUPS, NagiocpX::ClusterInfo{ m_fieldID, 0, 0 }, this));
	}
}

void QuestRoom::DestroyFieldTLS() noexcept
{
	//for (int i = 0; i < NagiocpX::NUM_OF_THREADS; ++i)
	{
		const std::span< XVector<NagiocpX::Cluster*>> clusters{ tl_vecClusters[NagiocpX::GetCurThreadIdx()],m_numOfClusters};
		for (const auto cluster : clusters | std::views::join)
		{
			for (auto& entities : cluster->GetEntitesExceptSession())
			{
				for (const auto entity : entities.GetItemListRef())
				{
					entity->TryOnDestroy();
				}
			}
			// 이전 필드에서는 겜 끝나기 직전 지웠었다.
			//NagiocpX::xdelete<NagiocpX::Cluster>(cluster);
		}
		//NagiocpX::DeleteJEMallocArray(clusters);
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
			for (const auto& players : owner->m_party_quest_system.m_member)
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

void FoxQuest::InitQuestField() noexcept
{
	for (int i = 0; i < 500; ++i)
	{
		EntityBuilder b;
		b.group_type = Nagox::Enum::GROUP_TYPE::GROUP_TYPE_MONSTER;
		b.obj_type = MONSTER_TYPE_INFO::FOX;
		const auto m = EntityFactory::CreateMonster(b);
		static_cast<Regenerator*>(m->GetDeleter())->m_targetField = SharedFromThis<NagiocpX::Field>();
		EnterFieldNPC(m);
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
		EnterFieldNPC(m);
	}
}
