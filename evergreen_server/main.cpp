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
#include "ImGUIMgr.h"
#include "PathFinder_Common.h"

using namespace NagiocpX;
constexpr const int32_t NUM_OF_NPC = 2000001;
constexpr const int32_t NUM_OF_MAX_USER = 5500;
constexpr const uint32_t NUM_OF_CHANNELS = 60;

extern std::vector<DirectX::BoundingBox> boxes;

void DrawMiniMap()noexcept
{
	constexpr const float MAP_WIDTH = 1024.f;
	constexpr const float MAP_HEIGHT = 1024.f;

	ImGui::Begin("MiniMap");

	constexpr const ImVec2 size = ImVec2(1280, 720);
	const ImVec2 topLeft = ImGui::GetCursorScreenPos();
	ImDrawList* draw = ImGui::GetWindowDrawList();

	draw->AddRectFilled(topLeft, ImVec2(topLeft.x + size.x, topLeft.y + size.y),
		IM_COL32(30, 30, 30, 255));
	draw->AddRect(topLeft, ImVec2(topLeft.x + size.x, topLeft.y + size.y),
		IM_COL32(255, 255, 255, 80));


	const auto WorldToMiniMap = [&](const Vector3& pos) -> ImVec2 {
		const float nx = (pos.x + MAP_WIDTH * 0.5f) / MAP_WIDTH;
		const float nz = (pos.z + MAP_HEIGHT * 0.5f) / MAP_HEIGHT;
		const float px = topLeft.x + nx * size.x;
		const float py = topLeft.y + nz * size.y;
		return ImVec2(px, py);
		};

	const auto users = Service::GetMainService()->GetAllSessionContainer();

	for (const auto& u : users)
	{
		const auto ptr = u.ptr.load();
		if (!ptr)continue;
		if (const auto pos_comp = ptr->GetComp<PositionComponent>())
		{
			draw->AddCircleFilled(WorldToMiniMap(pos_comp->pos), 3.0f, IM_COL32(0, 255, 0, 255));
		}
	}

	const auto monsters = Mgr(FieldMgr)->GetNPCContainer();
	
	for (const auto& m : monsters)
	{
		const auto ptr = m.ptr.load();
		if (!ptr)continue;
		if (ptr->GetComp<HarvestInteraction>())continue;
		if (const auto pos_comp = ptr->GetComp<PositionComponent>())
		{
			draw->AddCircleFilled(WorldToMiniMap(pos_comp->pos), 3.0f, IM_COL32(255, 0, 0, 255));
		}
	}

	ImGui::Dummy(size);
	ImGui::End();
}

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

		for (int c = 0; c < NUM_OF_CHANNELS; ++c)
		{
			for (int i = 0; i < 100000/NUM_OF_CHANNELS; ++i)
			{
				EntityBuilder b;
				b.group_type = Nagox::Enum::GROUP_TYPE_MONSTER;
				b.obj_type = Nagox::Enum::MONSTER_TYPE_FOX;
				const auto m = EntityFactory::CreateSheep(b);
				const auto m2 = m.get();
				static_cast<Regenerator*>(m->GetDeleter())->m_targetField = Field::GetField(c)->SharedFromThis<Field>();
				Field::GetField(c)->EnterFieldWithFloatXYNPC(PositionComponent::GetXZWithOffsetGlobal(m2), m);
			}
		}

		const auto& h = HarvestLoader::GetHarvestPos();
		for (int c = 0; c < NUM_OF_CHANNELS; ++c)
		{
			for (const auto& [pos, type, mesh_type] : h)
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
				Field::GetField(c)->EnterFieldWithFloatXYNPC(PositionComponent::GetXZWithOffsetGlobal(m2), m);
			}
		}
		HarvestLoader::FreeHarvestLoader();
	}

	virtual void TLSInitialize()noexcept override
	{
		const volatile auto init_builder = GetBuilder();
		extern thread_local std::unordered_map<uint64_t, std::pair<ZeroInt, std::array<dtPolyRef, 10>>> tl_poly_vec;
		tl_poly_vec.reserve(128);
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
		//Mgr(ImGUIMgr)->Init();
		//Mgr(ImGUIMgr)->RegisterRenderFp(DrawMiniMap);
		//Mgr(ImGUIMgr)->Update();
		//Mgr(ImGUIMgr)->ShutDown();
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

void* custom_dt_alloc(size_t size, dtAllocHint) { return ::je_malloc(size); }

int main()
{
	ContentsInitiator con_init;
	ClusterPredicate broad_helper;
	Common::DataRegistry::Load();
	HarvestLoader::LoadHarvest({}, L"environment\\ExportedGameSpawns.json");
	dtAllocSetCustom(custom_dt_alloc, ::je_free);
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
	
	for (int c = 0; c < NUM_OF_CHANNELS; ++c)
	{
		Mgr(FieldMgr)->RegisterField<ContentsField>(c)->SetFieldID(c);
	}
	NagiocpX::ServerService::SetSessionDeleter([](Session* session)noexcept {return NagiocpX::Memory::AlignedFree(session, alignof(ClientSession)); });

	const auto pServerService = new NagiocpX::ServerService
		(
			  Mgr(CoreGlobal)->GetIocpCore()
			, NagiocpX::NetAddress{ L"0.0.0.0",7777 }
			, NagiocpX::aligned_xnew<ClientSession>
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