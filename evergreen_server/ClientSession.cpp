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

std::atomic_int cnt = 0;

ClientSession::ClientSession() noexcept
	:PacketSession{ ::c2s_PacketHandler::GetPacketHandlerList() }
{
	std::cout << ++cnt << std::endl;
}

ClientSession::~ClientSession()
{
	std::cout << "BYE" << std::endl;
	std::cout << --cnt << std::endl;
}

void ClientSession::OnConnected()
{
	//std::cout << "Connect !" << std::endl;
	const auto pOwner = GetOwnerEntity();
	pOwner->SetDetailType(PLAYER_TYPE_INFO::WARRIOR);
	pOwner->AddIocpComponent<ServerCore::Queueabler>();
	pOwner->AddComp<HP>()->InitHP(5);
	pOwner->AddComp<PlayerDeath>();
	pOwner->AddComp<QuestSystem>();
}

void ClientSession::OnDisconnected(const ServerCore::Cluster* const curCluster_)noexcept
{
	curCluster_->Broadcast(Create_s2c_REMOVE_OBJECT(GetSessionID()));
	//if (const auto sector_ptr = curSectorInfo_.GetPtr())
	//{
	//	
	//	const ServerCore::Vector<ServerCore::Sector*> temp{ sector_ptr };
	//	const auto ptr = SharedFromThis<PacketSession>();
	//	sector_ptr->BroadCastParallel(Create_s2c_REMOVE_OBJECT(GetSessionID()), temp, GetOwnerEntity());
	//}
	std::cout << "DisConnect !" << std::endl;
	//std::cout << cnt << std::endl;
	//Mgr(TaskTimerMgr)->ReserveAsyncTask(1000 + ServerCore::my_rand() % 1000, [e = GetOwnerEntity()->SharedFromThis()]() {
	//	const auto ee = e->UseCount();
	//	if (ee != 1)std::cout <<"¿Ö?: "<< ee << std::endl;
	//	});
}
