#include "pch.h"
#include "Service.h"
#include "ClientSession.h"
#include "SectorPredicate.h"
#include "ContentsWorld.h"

using namespace ServerCore;
constexpr const int32_t NUM_OF_NPC = 1024;

int main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	Mgr(CoreGlobal)->Init();
	c2s_PacketHandler::Init();
	
	Mgr(WorldMgr)->SetNumOfNPC<NUM_OF_NPC>();
	Mgr(WorldMgr)->RegisterWolrd<ContentsWorld>(0);

	ServerCore::MoveBroadcaster::RegisterHuristicFunc2Session(SectorPredicate::SectorHuristicFunc2Session);
	ServerCore::MoveBroadcaster::RegisterHuristicFunc2NPC(SectorPredicate::SectorHuristicFunc2NPC);


	ServerCore::MoveBroadcaster::RegisterAddPacketFunc(SectorPredicate::SectorAddPacketFunc);
	ServerCore::MoveBroadcaster::RegisterRemovePacketFunc(SectorPredicate::SectorRemovePacketFunc);
	ServerCore::MoveBroadcaster::RegisterMovePacketFunc(SectorPredicate::SectorMovePacketFunc);
	
	const auto pServerService = std::make_unique<ServerCore::ServerService>
		(
			  Mgr(CoreGlobal)->GetIocpCore()
			, ServerCore::NetAddress{ L"0.0.0.0",7777 }
			, ServerCore::MakeSharedAligned<ClientSession>
			, 1001
		);

	ASSERT_CRASH(pServerService->Start());

	Mgr(ThreadMgr)->Launch(
		  ThreadMgr::NUM_OF_THREADS
		, *pServerService
		, []() noexcept {const volatile auto init_builder = GetBuilder(); }
	);
}