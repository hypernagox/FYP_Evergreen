#include "pch.h"
#include "Service.h"
#include "ClientSession.h"
#include "SectorPredicate.h"
#include "ContentsWorld.h"
#include <filesystem>
#include "Navigator.h"
#include "EntityFactory.h"

using namespace ServerCore;
constexpr const int32_t NUM_OF_NPC = 10240;
extern std::vector<DirectX::BoundingBox> boxes;

void InitTLSFunc()noexcept
{
	const volatile auto init_builder = GetBuilder();
	//NAVIGATION->navMesh->InitNavMeshQuery();
}

void DestroyTLSFunc()noexcept
{
	NAVIGATION->GetNavMesh(NAVI_MESH_NUM::NUM_0)->FreeNavMeshQuery();
}

int main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	
	Mgr(CoreGlobal)->Init();
	c2s_PacketHandler::Init();
	NAVIGATION->Init();
	
	//NAVIGATION->GetNavMesh(NAVI_MESH_NUM::NUM_0)->LoadByObj(RESOURCE_PATH(L"navmesh10.obj"));
	NAVIGATION->RegisterDestroy();
	//AStar::SaveAStarPath(NAVIGATION->GetNavMesh(NAVI_MESH_NUM::NUM_0));
	Mgr(WorldMgr)->SetNumOfNPC<NUM_OF_NPC>();
	Mgr(WorldMgr)->RegisterWolrd<ContentsWorld>(0);

	ServerCore::MoveBroadcaster::RegisterHuristicFunc2Session(SectorPredicate::SectorHuristicFunc2Session);
	ServerCore::MoveBroadcaster::RegisterHuristicFunc2NPC(SectorPredicate::SectorHuristicFunc2NPC);


	ServerCore::MoveBroadcaster::RegisterAddPacketFunc(SectorPredicate::SectorAddPacketFunc);
	ServerCore::MoveBroadcaster::RegisterRemovePacketFunc(SectorPredicate::SectorRemovePacketFunc);
	ServerCore::MoveBroadcaster::RegisterMovePacketFunc(SectorPredicate::SectorMovePacketFunc);
	
	for(int i=0;i<100;++i)
	{
		EntityBuilder b;
		b.group_type = Nagox::Enum::GROUP_TYPE::GROUP_TYPE_MONSTER;
		b.obj_type = MONSTER_TYPE_INFO::FOX;
		const auto m = EntityFactory::CreateMonster(b);
		Mgr(WorldMgr)->GetWorld(0)->EnterWorldNPC(m);
	}
	const auto pServerService = std::make_unique<ServerCore::ServerService>
		(
			  Mgr(CoreGlobal)->GetIocpCore()
			, ServerCore::NetAddress{ L"0.0.0.0",7777 }
			, ServerCore::MakeSharedAligned<ClientSession>
			, 5001
		);

	ASSERT_CRASH(pServerService->Start());

	Mgr(ThreadMgr)->Launch(
		  ThreadMgr::NUM_OF_THREADS
		, DestroyTLSFunc
		, InitTLSFunc
	);
}