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
#include "HarvestLoader.h"
#include "Regenerator.h"
#include "PositionComponent.h"
#include "ClusterInfoHelper.h"
#include "NaviAgent.h"
#include "NaviAgent_Common.h"
#include "Interaction.h"

using namespace NagiocpX;
constexpr const int32_t NUM_OF_NPC = 100001;
constexpr const int32_t NUM_OF_MAX_USER = 5005;

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
		//	const auto pos = m->GetComp<PositionComponent>()->pos;
		//	const auto m2 = m.get();
		//	Field::GetField(0)->EnterFieldWithFloatXYNPC(PositionComponent::GetXZWithOffsetGlobal(m2), m);
		//}

		//for (int i = 0; i < 200; ++i)
		//{
		//	EntityBuilder b;
		//	b.group_type = Nagox::Enum::GROUP_TYPE_NPC;
		//	b.obj_type = 0;
		//	const auto m = EntityFactory::CreateRangeMonster(b);
		//	Mgr(FieldMgr)->GetField(0)->EnterFieldNPC(m);
		//}

		const auto& h = HarvestLoader::GetHarvestPos();
		for (const auto& [pos,type,mesh_type] : h)
		{
			EntityBuilder b;
			b.group_type = Nagox::Enum::GROUP_TYPE_HARVEST;
			b.obj_type = (uint8_t)type;
			b.x = pos.x;
			b.y = pos.y;
			b.z = pos.z;
			const auto m = EntityFactory::CreateHarvest(b);
			m->SetDetailType(HARVEST_STATE::AVAILABLE);
			m->GetComp<HarvestInteraction>()->SetInteractionType(mesh_type);
			const auto m2 = m.get();
			Field::GetField(0)->EnterFieldWithFloatXYNPC(PositionComponent::GetXZWithOffsetGlobal(m2), m);
			//Field::GetField(0)->EnterFieldNPCWithFloatXY(,m);
		}
		const Vector3 points[] = {
	Vector3(-259.22272F,84.94523F,-15.314469F),
	Vector3(-242.5948F,83.53157F,-20.12327F)  ,
	Vector3(-244.0131F,83.93936F,-1.8343055F) ,
	Vector3(-225.44817F,81.969826F,-12.672854F),
	Vector3(-210.61905F,81.896484F,5.847359F) ,
	Vector3(-199.39998F,80.16998F,-11.756538F),
	Vector3(-192.54158F,80.21647F,5.781999F)  ,
	Vector3(-174.05774F,78.48746F,-9.923426F) ,
	Vector3(-158.33324F,77.9395F,5.0919523F)  ,
		};
		//const auto num = sizeof(points) / sizeof(points[0]);
		//for (int i = 0; i < num; ++i)
		//{
		//	EntityBuilder b;
		//	b.group_type = Nagox::Enum::GROUP_TYPE::GROUP_TYPE_MONSTER;
		//	b.obj_type = MONSTER_TYPE_INFO::FOX;
		//	const auto m = EntityFactory::CreateMonster(b);
		//	//static_cast<Regenerator*>(m->GetDeleter())->m_targetField = SharedFromThis<NagiocpX::Field>();
		//	//m->GetComp<PositionComponent>()->pos = points[i];
		//	auto p = points[i];
		//	//float f[3]{ 10,10000,10 };
		//	//auto p2 = p;
		//	//dtPolyRef ref;
		//	//NAVIGATION->GetNavMesh(NUM_0)->GetNavMeshQuery()->findNearestPoly(&p.x, f,
		//	//	NAVIGATION->GetNavMesh(NUM_0)->GetNavFilter(), &ref, &p2.x
		//	//);
		//
		//	//p.y = NAVIGATION->GetNavMesh(NUM_0)->GetNaviCell(p).CalculateHeight(p, NAVIGATION->GetNavMesh(NUM_0));
		//	m->GetComp<NaviAgent>()->SetPos(p);
		//	m->GetComp<PositionComponent>()->pos = p;
		//	const auto pos = p;
		//	//m->GetComp<NaviAgent>()->InitCrowd();
		//	Field::GetField(0)->EnterFieldWithFloatXYNPC(pos.x + 512.f, pos.z + 512.f, m);
		//	//EnterFieldNPC(m);
		//}

		HarvestLoader::FreeHarvestLoader();
		//{
		//	EntityBuilder b;
		//	b.group_type = Nagox::Enum::GROUP_TYPE_NPC;
		//	b.obj_type = 0;
		//	b.x = -10.0f;
		//	b.y = 0.f;
		//	b.z = -10.0f;
		//	Field::GetField(0)->EnterFieldNPC(EntityFactory::CreateNPC(b));
		//}
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

XVector<Cluster*> GlobalClusterFilter(const ContentsEntity* const entity, const Field* const field)noexcept;
XVector<Cluster*> GetAllAdjClusterFunc(const ContentsEntity* const entity, const Field* const field)noexcept;
XVector<Cluster*> GlobalClusterFilterForTest(const ContentsEntity* const entity, const Field* const field)noexcept;

int main()
{
	ContentsInitiator con_init;
	ClusterPredicate broad_helper;
	Common::DataRegistry::Load();
	HarvestLoader::LoadHarvest({}, L"environment\\ExportedGameSpawns.json");

	NagiocpX::PrintKoreaRealTime("Server Start !");
	
	ClusterInfoHelper::RegisterClusterFilter(GlobalClusterFilter);
	//ClusterInfoHelper::RegisterAllClusterFunc(GetAllAdjClusterFunc);
	ClusterInfoHelper::RegisterAllClusterFunc(GlobalClusterFilter);

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

XVector<Cluster*> GlobalClusterFilter(const ContentsEntity* const entity, const Field* const field)noexcept
{
	XVector<Cluster*> visibleClusters;
	visibleClusters.reserve(9);

	const auto [x, z] = PositionComponent::GetXZWithOffsetGlobal(entity);

	const auto view_range = DISTANCE_FILTER;

	const auto row = field->GetNumOfClusterRow();
	const auto col = field->GetNumOfClusterCol();

	const float tileWidth = field->GetClusterXScale();
	const float tileHeight = field->GetClusterYScale();

	const int cx = static_cast<int>(x / tileWidth);
	const int cz = static_cast<int>(z / tileHeight);

	const float viewRangeSq = view_range * view_range;

	for (int dz = -1; dz <= 1; ++dz)
	{
		for (int dx = -1; dx <= 1; ++dx)
		{
			const int nx = cx + dx;
			const int nz = cz + dz;
			if (nx < 0 || nz < 0 || nx >= col || nz >= row)continue;

			const float centerX = (nx + 0.5f) * tileWidth;
			const float centerZ = (nz + 0.5f) * tileHeight;

			const float x_diff = (centerX - x);
			const float z_diff = (centerZ - z);

			const float dxSq = (x_diff) * (x_diff);
			const float dzSq = (z_diff) * (z_diff);

			//const float dxSq = (centerX - x) * (centerX - x);
			//const float dzSq = (centerZ - z) * (centerZ - z);

			if (dxSq + dzSq <= viewRangeSq)
			{
				visibleClusters.emplace_back(field->GetCluster(nx, nz));
			}
		}
	}

	const auto cur_cluster = field->GetCluster(cx, cz);

	if (std::ranges::find(visibleClusters, cur_cluster) == visibleClusters.end())
	{
		visibleClusters.emplace_back(cur_cluster);
	}

	return visibleClusters;
}

XVector<Cluster*> GetAllAdjClusterFunc(const ContentsEntity* const entity, const Field* const field)noexcept
{
	XVector<Cluster*> visibleClusters;
	visibleClusters.reserve(9);
	const auto [x, z] = PositionComponent::GetXZWithOffsetGlobal(entity);

	const auto row = field->GetNumOfClusterRow();
	const auto col = field->GetNumOfClusterCol();

	const float tileWidth = field->GetClusterXScale();
	const float tileHeight = field->GetClusterYScale();

	const int cx = static_cast<int>(x / tileWidth);
	const int cz = static_cast<int>(z / tileHeight);

	for (int dz = -1; dz <= 1; ++dz)
	{
		for (int dx = -1; dx <= 1; ++dx)
		{
			const int nx = cx + dx;
			const int nz = cz + dz;
			if (nx < 0 || nz < 0 || nx >= col || nz >= row)continue;
			visibleClusters.emplace_back(field->GetCluster(nx, nz));
		}
	}

	return visibleClusters;
}

XVector<Cluster*> GlobalClusterFilterForTest(const ContentsEntity* const entity, const Field* const field)noexcept
{
	const auto [x, z] = PositionComponent::GetXZWithOffsetGlobal(entity);

	const auto view_range = DISTANCE_FILTER;

	const auto row = field->GetNumOfClusterRow();
	const auto col = field->GetNumOfClusterCol();

	const float tileWidth = field->GetClusterXScale();
	const float tileHeight = field->GetClusterYScale();

	const int cx = static_cast<int>(x / tileWidth);
	const int cz = static_cast<int>(z / tileHeight);

	return Vector<Cluster*>{field->GetCluster(cx, cz)};
}