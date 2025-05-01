#include "pch.h"
#include "Service.h"
#include "ClientSession.h"
#include "ClusterPredicate.h"
#include <filesystem>
#include "Navigator.h"
#include "EntityFactory.h"
#include "Field.h"
#include "Cluster.h"
#include "ContentsField.h"
#include "MoveBroadcaster.h"
#include "DataRegistry.h"
#include "QuestRoom.h"
#include "HarvestSystem.h"
#include "Regenerator.h"

using namespace NagiocpX;
constexpr const int32_t NUM_OF_NPC = 100001;
constexpr const int32_t NUM_OF_MAX_USER = 12;

extern std::vector<DirectX::BoundingBox> boxes;

class ContentsInitiator
	: public NagiocpX::Initiator
{
public:
	virtual void GlobalInitialize()noexcept override
	{
		//for (int i = 0; i < 2000; ++i)
		//{
		//	EntityBuilder b;
		//	b.group_type = Nagox::Enum::GROUP_TYPE::GROUP_TYPE_MONSTER;
		//	b.obj_type = MONSTER_TYPE_INFO::FOX;
		//	const auto m = EntityFactory::CreateMonster(b);
		//	Field::GetField(0)->EnterFieldNPC(m);
		//}
		//for (int i = 0; i < 200; ++i)
		//{
		//	EntityBuilder b;
		//	b.group_type = Nagox::Enum::GROUP_TYPE_NPC;
		//	b.obj_type = 0;
		//	const auto m = EntityFactory::CreateRangeMonster(b);
		//	Mgr(FieldMgr)->GetField(0)->EnterFieldNPC(m);
		//}
		const auto& h = HarvestSystem::GetHarvestPos();
		for (const auto& pos : h)
		{
			EntityBuilder b;
			b.group_type = Nagox::Enum::GROUP_TYPE_HARVEST;
			b.obj_type = 0;
			b.x = pos.x;
			b.y = pos.y;
			b.z = pos.z;
			const auto m = EntityFactory::CreateHarvest(b);
			static_cast<Regenerator*>(m->GetDeleter())->m_targetField = Field::GetField(0)->SharedFromThis<Field>();
			Field::GetField(0)->EnterFieldNPC(m);
		}
		{
			EntityBuilder b;
			b.group_type = Nagox::Enum::GROUP_TYPE_NPC;
			b.obj_type = 0;
			b.x = -10.0f;
			b.y = 0.f;
			b.z = -10.0f;
			Field::GetField(0)->EnterFieldNPC(EntityFactory::CreateNPC(b));
		}
	}

	virtual void TLSInitialize()noexcept override
	{
		const volatile auto init_builder = GetBuilder();
	}

	virtual void TLSDestroy()noexcept override
	{
		NAVIGATION->GetNavMesh(NAVI_MESH_NUM::NUM_0)->FreeNavMeshQuery();
	}
	
	virtual void GlobalDestroy()noexcept override
	{
	}

	virtual void ControlThreadFunc()noexcept override
	{
		for (;;)
		{
			system("pause");
			char buf[32]{};
			std::cin >> buf;
			if ("EXIT" == std::string_view{ buf })break;
		}
	}
};

int main()
{
	ContentsInitiator con_init;
	ClusterPredicate broad_helper;
	Common::DataRegistry::Load();
	HarvestSystem::LoadHarvest({}, L"environment\\ExportedGameSpawns.json");

	NagiocpX::PrintKoreaRealTime("Server Start !");
	
	GET_DATA(std::string, "Warrior", "name");

	Mgr(CoreGlobal)->Init();
	c2s_PacketHandler::Init();
	NAVIGATION->Init();
	NAVIGATION->RegisterDestroy();

	MoveBroadcaster::RegisterGlobalHelper(&broad_helper);
	Mgr(FieldMgr)->SetNumOfNPC<NUM_OF_NPC>();
	Mgr(FieldMgr)->RegisterField<ContentsField>(0);
	
	const auto pServerService = new NagiocpX::ServerService
		(
			  Mgr(CoreGlobal)->GetIocpCore()
			, NagiocpX::NetAddress{ L"0.0.0.0",7777 }
			, NagiocpX::xnew<ClientSession>
			, c2s_PacketHandler::GetPacketHandlerList()
			, NUM_OF_MAX_USER
		);
	
	NAGOX_ASSERT(pServerService->Start());
	
	std::atomic_thread_fence(std::memory_order_seq_cst);
	
	Mgr(ThreadMgr)->Launch(
		  ThreadMgr::NUM_OF_THREADS
		, &con_init
	);
}