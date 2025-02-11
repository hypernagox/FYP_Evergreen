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
	pOwner->SetDetailType(PLAYER_TYPE_INFO::WARRIOR);
	pOwner->AddIocpComponent<NagiocpX::Queueabler>();
	pOwner->AddComp<HP>()->InitHP(5);
	pOwner->AddComp<PlayerDeath>();
	pOwner->AddComp<QuestSystem>();
	pOwner->AddComp<NagiocpX::TimerHandler>();
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
	//	if (ee != 1)std::cout <<"¿Ö?: "<< ee << std::endl;
	//	});
}
