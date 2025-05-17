#include "Session.h"
#include "pch.h"
#include "ClientSession.h"
#include "Queueabler.h"
#include "TaskTimerMgr.h"
#include "MoveBroadcaster.h"
#include "Cluster.h"
#include "ClusterInfoHelper.h"
#include "HP.h"
#include "Death.h"
#include "QuestSystem.h"
#include "TimerRoutine.h"
#include "Inventory.h"
#include "StatusSystem.h"
#include "PartyQuestSystem.h"

std::atomic_int cnt = 0;
static NagoxAtomic::Atomic<int> g_connect_count{ 0 };

ClientSession::ClientSession() noexcept
{
	std::cout << ++cnt << '\n';
}

ClientSession::~ClientSession()
{
	NagiocpX::PrintLogEndl("BYE");
	std::cout << --cnt << '\n';
}

void ClientSession::OnConnected()
{
	//std::cout << "Connect !" << std::endl;
	const auto pOwner = GetOwnerEntity();

	pOwner->AddIocpComponent<NagiocpX::Queueabler>();
	pOwner->AddComp<HP>()->InitHP(5); // TODO 매직넘버
	pOwner->AddComp<PlayerDeath>();
	pOwner->AddComp<QuestSystem>();
	pOwner->AddComp<NagiocpX::TimerHandler>();

	const auto inv = pOwner->AddComp<Inventory>();

	pOwner->AddComp<StatusSystem>()->m_equipSystem = inv->GetEquipmentSystem();

	std::cout << ++g_connect_count << '\n';
	NagiocpX::PrintKoreaRealTime("Connect !", GetAddress().GetIpAddress());
}

void ClientSession::OnDisconnected(const NagiocpX::Cluster* const curCluster_)noexcept
{
	GetOwnerEntity()->GetComp<NagiocpX::ClusterInfoHelper>()->BroadcastCluster(Create_s2c_REMOVE_OBJECT(GetSessionID()));
	//if (const auto sector_ptr = curSectorInfo_.GetPtr())
	//{
	//	
	//	const NagiocpX::Vector<NagiocpX::Sector*> temp{ sector_ptr };
	//	const auto ptr = SharedFromThis<PacketSession>();
	//	sector_ptr->BroadCastParallel(Create_s2c_REMOVE_OBJECT(GetSessionID()), temp, GetOwnerEntity());
	//}
	std::cout << --g_connect_count << '\n';
	NagiocpX::PrintKoreaRealTime("DisConnect !", GetAddress().GetIpAddress());
	std::cout << cnt << std::endl;
	//Mgr(TaskTimerMgr)->ReserveAsyncTask(1000 + NagiocpX::my_rand() % 1000, [e = GetOwnerEntity()->SharedFromThis()]() {
	//	const auto ee = e->UseCount();
	//	if (ee != 1)std::cout <<"왜?: "<< ee << std::endl;
	//	});
	
	//m_party_quest_system->ResetPartyQuestSystem();
	if (const auto p = m_cur_my_party_system.load())
	{
		const auto q_room = p->m_curQuestRoomInstance;
		p->OutMember(GetSessionID());
		if (q_room)
		{
			q_room->DecMemberCount();
		}
	}
}

std::shared_ptr<PartyQuestSystem> ClientSession::CreatePartySystem()
{
	if (m_cur_my_party_system.load())return {};
	auto sys = NagiocpX::MakeSharedSTD<PartyQuestSystem>();
	m_cur_my_party_system.store(
		sys
	);
	sys->m_member[0] = GetOwnerEntity()->SharedFromThis();
	return sys;
}

std::shared_ptr<PartyQuestSystem> ClientSession::QueryPartyLeader() const noexcept
{
	const auto owner = GetOwnerEntity();
	auto sys = m_cur_my_party_system.load();
	if (!sys)return {};
	if (sys->QueryPartyLeader(owner))
	{
		return sys;
	}
	else
	{
		return {};
	}
}

S_ptr<ContentsEntity> ClientSession::GetCurPartySystemLeader() const noexcept
{
	const auto party = m_cur_my_party_system.load();
	if (!party)return nullptr;
	return party->GetPartyLeader();
}

PARTY_ACCEPT_RESULT ClientSession::AcceptNewPlayer(const S_ptr<PacketSession>& session)
{
	return AcceptNewPlayer(session->GetOwnerEntity());
}

PARTY_ACCEPT_RESULT ClientSession::AcceptNewPlayer(ContentsEntity* const other)
{
	const auto cur_party = m_cur_my_party_system.load();
	if(!cur_party)return PARTY_ACCEPT_RESULT::INVALID;
	const auto leader = cur_party->GetPartyLeader();
	if(GetOwnerEntity() != leader.get())return PARTY_ACCEPT_RESULT::INVALID;
	const auto other_session = other->GetClientSession();
	std::shared_ptr<PartyQuestSystem> expected{ nullptr };
	if (!other_session->m_cur_my_party_system.compare_exchange_strong(
		expected,
		cur_party
	)) {
		// 2개의 동시 신청 건에 대하여 동시승인 할 경우 CAS로 승부를가린다.
		// + 이미 파티에있을경우도 실패
		return PARTY_ACCEPT_RESULT::INVALID;
	}
	return cur_party->AcceptNewMember(other->SharedFromThis());
}
