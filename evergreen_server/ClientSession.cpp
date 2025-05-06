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
	m_party_quest_system = NagiocpX::xnew<PartyQuestSystem>();
}

ClientSession::~ClientSession()
{
	NagiocpX::PrintLogEndl("BYE");
	std::cout << --cnt << '\n';
	NagiocpX::xdelete<PartyQuestSystem>(m_party_quest_system);
}

void ClientSession::OnConnected()
{
	//std::cout << "Connect !" << std::endl;
	const auto pOwner = GetOwnerEntity();
	pOwner->SetDetailType(PLAYER_TYPE_INFO::WARRIOR);
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
	curCluster_->Broadcast(Create_s2c_REMOVE_OBJECT(GetSessionID()));
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
	m_party_quest_system->m_curQuestRoomInstance.reset();
	//m_party_quest_system->ResetPartyQuestSystem();
	if (const auto p = m_cur_my_party_system.load())
	{
		p->OutMember(GetSessionID());
	}
}

bool ClientSession::CreatePartySystem()
{
	if (m_cur_my_party_system.load())return false;
	IncRef();
	m_party_quest_system->m_member[0] = this;
	m_cur_my_party_system.store(m_party_quest_system);
	return true;
}

ClientSession* ClientSession::GetCurPartySystemLeader() const noexcept
{
	const auto party = m_cur_my_party_system.load();
	if (!party)return nullptr;
	return party->GetPartyLeader();
}

PARTY_ACCEPT_RESULT ClientSession::AcceptNewPlayer(ClientSession* const other)
{
	if (!IsPartyLeader())return PARTY_ACCEPT_RESULT::INVALID;
	if (other->HasParty())return PARTY_ACCEPT_RESULT::INVALID;
	other->m_cur_my_party_system.store(m_party_quest_system);
	return m_party_quest_system->AcceptNewMember(other->SharedFromThis<ClientSession>());
}
