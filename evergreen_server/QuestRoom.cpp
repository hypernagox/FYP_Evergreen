#include "pch.h"
#include "QuestRoom.h"
#include "EntityFactory.h"
#include "Regenerator.h"
#include "PartyQuestSystem.h"
#include "ClientSession.h"
#include "TaskTimerMgr.h"
#include "Navigator.h"
#include "NavigationMesh.h"
#include "PathNPC.h"
#include "PositionComponent.h"
#include "NaviAgent.h"
#include "NaviAgent_Common.h"
#include "ClusterPredicate.h"
#include "Collider_Common.h"
#include "HP.h"
#include "Death.h"
#include "Collider_Common.h"

std::atomic_int aaaa;

static constinit const Vector3 ViLLAGE_ENTRANCE = Vector3(-103.19971F, 75.85828F, 11.403826F);

QuestRoom::QuestRoom() noexcept
{
	m_fieldID = -1;
	m_field_x_scale = 1024;
	m_field_y_scale = 1024;

	m_cluster_x_scale = m_field_x_scale;
	m_cluster_y_scale = m_field_y_scale;

	InitFieldGlobal();
	InitFieldTLS();
	std::cout << ++aaaa << std::endl;

}

const Vector3 mon_quest_pos[]
{
	Vector3(-95.737885F,75.60237F,0.45216048F),
	Vector3(-84.906555F,74.91477F,1.8365619F),
	Vector3(-76.985535F,75.653595F,-2.3457224F),
	Vector3(-63.27754F,74.19354F,8.34514F),
	Vector3(-51.666164F,73.28497F,18.615622F),
	Vector3(-70.57306F,74.45939F,26.04673F),
	Vector3(-58.571342F,74.35416F,31.72524F),
};

QuestRoom::~QuestRoom() noexcept
{
	const auto row = (size_t)GetNumOfClusterRow();

	for (int i = 0; i < NagiocpX::NUM_OF_THREADS; ++i)
	{
		const std::span<XVector<NagiocpX::Cluster*>> clusters{ tl_vecClusters[i], row };

		for (const auto cluster : clusters | std::views::join)
		{
			for (auto& entities : cluster->GetEntitesExceptSession())
			{
				for (auto* entity : entities.GetItemListRef())
				{
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
	entity->GetSession()->SendAsync(Create_s2c_PARTY_QUEST_CLEAR(m_ownerPartrySystem->GetCurPartyQuestID()));
}

void QuestRoom::NotifyQuestFail(NagiocpX::ContentsEntity* const entity) const noexcept
{
	// TODO: 알리기

}

void QuestRoom::InitFieldGlobal() noexcept
{
	m_fieldID = -1;
	const auto row = GetNumOfClusterRow();
	const auto col = GetNumOfClusterCol();

	InitMutexForBenchmark(row, col);
}

void QuestRoom::InitFieldTLS() noexcept
{
	const auto row = GetNumOfClusterRow();
	const auto col = GetNumOfClusterCol();

	for (int i = 0; i < NagiocpX::NUM_OF_THREADS; ++i)
	{
		const auto clusters = NagiocpX::CreateJEMallocArray<XVector<NagiocpX::Cluster*>>(row);
		tl_vecClusters[i] = clusters.data();
		for (uint8 r = 0; r < row; ++r)
		{
			tl_vecClusters[i][r].reserve(col);
			for (uint8 c = 0; c < col; ++c)
			{
				tl_vecClusters[i][r].emplace_back(NagiocpX::xnew<NagiocpX::Cluster>(
					NUM_OF_GROUPS,
					NagiocpX::ClusterInfo{ m_fieldID, c, r },
					this
				));
			}
		}
	}
}

void QuestRoom::DestroyFieldTLS() noexcept
{
	const auto row = (size_t)GetNumOfClusterRow();

	for (int i = 0; i < NagiocpX::NUM_OF_THREADS; ++i)
	{
		const std::span<XVector<NagiocpX::Cluster*>> clusters{ tl_vecClusters[i], row };

		for (const auto  cluster : clusters | std::views::join)
		{
			for (auto& entities : cluster->GetEntitesExceptSession())
			{
				for (auto* entity : entities.GetItemListRef())
				{
					entity->TryOnDestroy();
				}
			}
		}
	}
}

void QuestRoom::MigrationAfterBehavior(Field* const prev_field) noexcept
{
	//std::cout << "멤버카운트 " << (int)m_numOfMember << std::endl;
	std::cout << "성공\n";
}

void QuestRoom::DecMemberCount() noexcept
{
	const auto count = m_numOfMember.fetch_sub(1);
	//std::cout << (int)count << std::endl;
	if (1 == count)
	{
		std::cout << "삭제시작\n";
		DestroyFieldTLS();
		m_ownerPartrySystem->SetCurRoomInstance(nullptr);
	}
}

std::shared_ptr<PartyQuestSystem> QuestRoom::GetOwnerSystem() const noexcept
{
	return m_ownerPartrySystem;
}

void QuestRoom::CheckPartyQuestState()noexcept
{
	// TODO: 락 고려
	if (ProcessPartyQuest())
	{
		EntityBuilder b;
		b.group_type = Nagox::Enum::GROUP_TYPE::GROUP_TYPE_HARVEST;
		b.obj_type = 0;
		auto p = Vector3(-42.968254F, 75.610634F, -87.984F);
		//p = m_ownerPartrySystem->m_member[0]->GetOwnerEntity()->GetComp<PositionComponent>()->pos;
		b.x = p.x;
		b.y = p.y;
		b.z = p.z;
		const auto m = EntityFactory::CreateClearTree(b);
		m->SetDetailType(HARVEST_STATE::AVAILABLE);
		const auto pos = m->GetComp<PositionComponent>()->pos;
		for (const auto& players : m_ownerPartrySystem->GetPartyMembers())
		{
			if (!players)continue;
			NotifyQuestClear(players.get());
		}
		EnterFieldWithFloatXYNPC(pos.x + 512.f, pos.z + 512.f, m);
		// TODO 근본적인 해결책
		//auto owner = m_ownerPartrySystem->m_member[0];
		m_isClear.store(true);
		//Mgr(TaskTimerMgr)->ReserveAsyncTask(1000,[this, owner]() {
		//	//for (const auto& players : owner->m_party_quest_system->m_member)
		//	//{
		//	//	if (!players)continue;
		//	//	//NotifyQuestClear(players->GetOwnerEntity());
		//	//}
		//	m_isClear.store(true);
		//	//m_ownerPartrySystem->GetPartyLeader()->m_cur_my_party_system.load()->MissionEnd();
		//	});
		return;
	}
	else if (IsFailPartyQuest())
	{
		for (const auto& players : m_ownerPartrySystem->GetPartyMembers())
		{
			if (!players)continue;
			NotifyQuestFail(players.get());
		}
		return;
	}
}

void QuestRoom::SetQuestBeginPos(const Vector3& pos) noexcept
{
	const auto party_sys = m_ownerPartrySystem;
	if (!party_sys)return;
	const auto members = party_sys->GetPartyMembers();
	for (const auto& member : members)
	{
		if (!member)continue;
		const auto pos_comp = member->GetComp<PositionComponent>();
		pos_comp->pos = pos;

		member->GetClientSession()->SendAsync(Create_s2c_FORCED_MOVE(member->GetObjectID(), ToFlatVec(pos)));
	}
}


bool InvadeQuestBase::ProcessPartyQuest() noexcept
{
	return true;
}

void FoxQuest::InitQuestField() noexcept
{
	//for (int i = 0; i < 500; ++i)
	//{
	//	EntityBuilder b;
	//	b.group_type = Nagox::Enum::GROUP_TYPE::GROUP_TYPE_MONSTER;
	//	b.obj_type = MONSTER_TYPE_INFO::FOX;
	//	const auto m = EntityFactory::CreateMonster(b);
	//	static_cast<Regenerator*>(m->GetDeleter())->m_targetField = SharedFromThis<NagiocpX::Field>();
	//	const auto pos = m->GetComp<PositionComponent>()->pos;
	//	EnterFieldWithFloatXYNPC(pos.x + 512.f, pos.z + 512.f, m);
	//	//EnterFieldNPC(m);
	//}

	const auto num = (int)(sizeof(mon_quest_pos) / sizeof(mon_quest_pos[0]));

	m_mon_count.store_relaxed(num);

	for (int i = 0; i < num; ++i)
	{
		EntityBuilder b;
		b.group_type = Nagox::Enum::GROUP_TYPE::GROUP_TYPE_MONSTER;
		b.obj_type = Nagox::Enum::MONSTER_TYPE_FOX;
		const auto m = EntityFactory::CreateMonster(b);
		//static_cast<Regenerator*>(m->GetDeleter())->m_targetField = SharedFromThis<NagiocpX::Field>();

		m->GetComp<PositionComponent>()->pos = mon_quest_pos[i];
		m->GetComp<NaviAgent>()->SetPos(mon_quest_pos[i]);
		const auto pos = m->GetComp<PositionComponent>()->pos;
		EnterFieldWithFloatXYNPC(pos.x + 512.f, pos.z + 512.f, m);
		//EnterFieldNPC(m);
	}
	SetQuestBeginPos(ViLLAGE_ENTRANCE);
}

void GoblinQuest::InitQuestField() noexcept
{
	//for (int i = 0; i < 500; ++i)
	//{
	//	EntityBuilder b;
	//	b.group_type = Nagox::Enum::GROUP_TYPE_NPC;
	//	b.obj_type = 0;
	//	const auto m = EntityFactory::CreateRangeMonster(b);
	//	static_cast<Regenerator*>(m->GetDeleter())->m_targetField = SharedFromThis<NagiocpX::Field>();
	//
	//	const auto pos = m->GetComp<PositionComponent>()->pos;
	//	EnterFieldWithFloatXYNPC(pos.x + 512.f, pos.z + 512.f, m);
	//	//EnterFieldNPC(m);
	//}

	const auto num = (int)(sizeof(mon_quest_pos) / sizeof(mon_quest_pos[0]));

	m_mon_count.store_relaxed(num);

	for (int i = 0; i < num; ++i)
	{
		EntityBuilder b;
		b.group_type = Nagox::Enum::GROUP_TYPE_MONSTER;
		b.obj_type = Nagox::Enum::MONSTER_TYPE_GOBLIN;
		const auto m = EntityFactory::CreateRangeMonster(b);
		//static_cast<Regenerator*>(m->GetDeleter())->m_targetField = SharedFromThis<NagiocpX::Field>();
		m->GetComp<PositionComponent>()->pos = mon_quest_pos[i];
		m->GetComp<NaviAgent>()->SetPos(mon_quest_pos[i]);
		const auto pos = m->GetComp<PositionComponent>()->pos;
		EnterFieldWithFloatXYNPC(pos.x + 512.f, pos.z + 512.f, m);
	}
	SetQuestBeginPos(ViLLAGE_ENTRANCE);
}

void TutorialGuardQuest::InitQuestField() noexcept
{
	SetPathNPC({
	Vector3(-270.50497F,86.48416F,-23.966377F),
	Vector3(-262.50986F,86.0128F,-23.310076F),
	Vector3(-248.07124F,84.52976F,-15.239957F),
	Vector3(-236.64597F,83.31353F,-5.343036F),
	Vector3(-229.64525F,82.18539F,-2.87825F),
	Vector3(-209.29497F,81.04176F,-3.3852773F),
	Vector3(-188.42654F,79.49546F,-3.2104106F),
	Vector3(-169.08516F,78.3188F,-3.3492434F),
	Vector3(-149.05505F,78.25525F,-0.90608793F),
	Vector3(-139.2348F,78.220726F,-0.37422684F),
	Vector3(-122.58272F,75.91813F,11.575291F),
		});
	SetMonsters({
		Vector3(-259.22272F, 84.94523F, -15.314469F),
		Vector3(-242.5948F, 83.53157F, -20.12327F),
		Vector3(-244.0131F, 83.93936F, -1.8343055F),
		Vector3(-225.44817F, 81.969826F, -12.672854F),
		Vector3(-210.61905F, 81.896484F, 5.847359F),
		Vector3(-199.39998F, 80.16998F, -11.756538F),
		Vector3(-192.54158F, 80.21647F, 5.781999F),
		Vector3(-174.05774F, 78.48746F, -9.923426F),
		Vector3(-158.33324F, 77.9395F, 5.0919523F) }
		);
}

void TutorialGuardQuest::SetMonsters(const XVector<Vector3> points) noexcept
{
	const auto num = (int)points.size();
	for (int i = 0; i < num; ++i)
	{
		EntityBuilder b;
		b.group_type = Nagox::Enum::GROUP_TYPE::GROUP_TYPE_MONSTER;
		b.obj_type = Nagox::Enum::MONSTER_TYPE_FOX;
		const auto m = EntityFactory::CreateMonster(b);
		auto p = points[i];
		m->GetComp<NaviAgent>()->SetPos(p);
		m->GetComp<PositionComponent>()->pos = p;
		const auto pos = p;
		EnterFieldWithFloatXYNPC(pos.x + 512.f, pos.z + 512.f, m);
	}
}

void NPCGuardQuestBase::SetPathNPC(XVector<Vector3> path) noexcept
{
	EntityBuilder b;
	b.group_type = Nagox::Enum::GROUP_TYPE_MONSTER;
	b.obj_type = Nagox::Enum::MONSTER_TYPE_BEAR;
	const auto m = EntityFactory::CreatePathNPC(b);
	const auto m2 = m;
	//EnterFieldNPC(m);
	// TODO: 위험
	EnterFieldWithFloatXYNPC(path[0].x + 512.f, path[0].z + 512.f, m);
	m2->GetComp<PathNPC>()->m_owner_system = GetOwnerSystem();
	m2->GetComp<PathNPC>()->InitPathNPC(std::move(path));
}

void BearQuest::InitQuestField() noexcept
{
	const auto num = (int)(sizeof(mon_quest_pos) / sizeof(mon_quest_pos[0]));

	m_mon_count.store_relaxed(num);

	for (int i = 0; i < num; ++i)
	{
		EntityBuilder b;
		b.group_type = Nagox::Enum::GROUP_TYPE_MONSTER;
		b.obj_type = Nagox::Enum::MONSTER_TYPE_BEAR;
		const auto m = EntityFactory::CreateMonster(b);
		//static_cast<Regenerator*>(m->GetDeleter())->m_targetField = SharedFromThis<NagiocpX::Field>();
		m->GetComp<PositionComponent>()->pos = mon_quest_pos[i];
		m->GetComp<NaviAgent>()->SetPos(mon_quest_pos[i]);
		const auto pos = m->GetComp<PositionComponent>()->pos;
		EnterFieldWithFloatXYNPC(pos.x + 512.f, pos.z + 512.f, m);
	}
	SetQuestBeginPos(ViLLAGE_ENTRANCE);
}

PatrolUnit* const InvadeQuestBase::AddPatrolUnit(Nagox::Enum::MONSTER_TYPE mon_type, const Vector3& start, const Vector3& dest, const float speed) noexcept
{
	auto& patrol = m_patrols.emplace_back(mon_type, start, dest, speed);
	const auto temp_ptr = patrol.m_patrol;
	const auto pos = temp_ptr->GetComp<PositionComponent>()->pos;
	EnterFieldWithFloatXYNPC(pos.x + 512.f, pos.z + 512.f, temp_ptr);
	return &patrol;
}

void InvadeQuestBase::Update()noexcept
{
	m_timer.Update();
	const auto DT = m_timer.GetDT();
	const auto party_sys = m_ownerPartrySystem;
	for (auto& patrol : m_patrols)
	{
		if (patrol.UpdatePatrol(DT))
		{
			if (party_sys)
			{
				patrol.CheckInsight(party_sys, m_resetPos);
			}
		}
	}
	if (!m_bIsRunning)return;
	Mgr(TaskTimerMgr)->ReserveAsyncTask(100, [r = SharedFromThis<InvadeQuestBase>()]() {
		r->Update();
		});
}

void InvadeQuestBase::StartUpdate(const Vector3& reset_pos) noexcept
{
	
	for (const auto& patrol : m_patrols)
	{
		const auto& monster_entity = patrol.m_patrol;
		const auto agent = monster_entity->AddComp<NaviAgent>();
		agent->SetPosComp(monster_entity->GetComp<PositionComponent>());
		agent->GetAgentConcreate()->SetNavMesh(NAVIGATION->GetNavMesh(NAVI_MESH_NUM::NUM_0));
		monster_entity->GetComp<NaviAgent>()->SetPos(monster_entity->GetComp<PositionComponent>()->pos);
	}

	m_resetPos = reset_pos;
	m_timer.Update();
	Mgr(TaskTimerMgr)->ReserveAsyncTask(100, [r = SharedFromThis<InvadeQuestBase>()]() {
		r->Update();
		});
	SetQuestBeginPos(reset_pos);
}

ContentsEntity* const InvadeQuestBase::SetTargetObject(const Vector3& target_pos) noexcept
{
	const auto monster_entity = NagiocpX::CreateContentsEntity(Nagox::Enum::GROUP_TYPE_MONSTER, Nagox::Enum::MONSTER_TYPE_FOX);

	
	const auto pos_comp = monster_entity->AddComp<PositionComponent>();
	pos_comp->pos = target_pos;
	
	monster_entity->AddComp<SphereCollider>()->SetSphere(monster_entity->GetComp<PositionComponent>(), 1.5f);

	monster_entity->GetComp<SphereCollider>()->GetCollider()->m_offSet.y += 1.f;

	monster_entity->AddComp<HP>()->InitHP(GET_DATA(int, "Monster", "Fox", "hp")); // TODO 매직넘버
	monster_entity->AddComp<MonsterDeath>();
	const auto temp_ptr = monster_entity.get();
	const auto pos = pos_comp->pos;
	EnterFieldWithFloatXYNPC(pos.x + 512.f, pos.z + 512.f, monster_entity);
	return temp_ptr;
}

PatrolUnit::PatrolUnit(
	  Nagox::Enum::MONSTER_TYPE mon_type
	, const Vector3& start
	, const Vector3& dest
	, const float speed) noexcept
	: m_patrol{ NagiocpX::CreateContentsEntity(Nagox::Enum::GROUP_TYPE_MONSTER, mon_type) }
	, m_start{start}
	, m_dest{dest}
	, m_dist{ Vector3::Distance(start,dest) }
	, m_accStop{ 0.f }
	, m_bTempStopPatrol{ false }
	, m_speed{ speed }
{
	const auto pos_comp = m_patrol->AddComp<PositionComponent>();
	pos_comp->pos = start;
	const auto dir = CommonMath::Normalized(dest - start);
	pos_comp->body_angle = DirectX::XMConvertToDegrees(std::atan2(dir.x, dir.z));
}

bool PatrolUnit::UpdatePatrol(const float DT) noexcept
{
	const auto pos_comp = m_patrol->GetComp<PositionComponent>();
	const auto dir = CommonMath::Normalized(m_dest - pos_comp->pos);
	if (!m_bTempStopPatrol)
	{
		const auto delta = dir * DT * m_speed;
		const auto prev_pos = pos_comp->pos;
		m_prev_pos = prev_pos;
		pos_comp->pos += delta;
		m_dist -= delta.Length(); 
		const auto& monster_entity = m_patrol;
		const auto agent = monster_entity->GetComp<NaviAgent>();
		agent->SetCellPos(DT, prev_pos, pos_comp->pos);
		if (0.f >= m_dist)
		{
			m_dist = Vector3::Distance(m_start, m_dest);
			std::swap(m_start, m_dest);
			m_bTempStopPatrol = true;
		}
		else
		{
			ClusterPredicate c;
			m_patrol->GetCurCluster()->Broadcast(c.ClusterPredicate::CreateMovePacket(m_patrol.get()));
		}
		return true;
	}
	else
	{
		m_accStop += DT;
		if (1.5f <= m_accStop)
		{
			m_accStop = 0.f;
			m_bTempStopPatrol = false;
			const auto dir = CommonMath::Normalized(m_dest - m_start);
			pos_comp->body_angle = DirectX::XMConvertToDegrees(std::atan2(dir.x, dir.z));
			return true;
		}
		else
		{
			return false;
		}
	}
}

void PatrolUnit::CheckInsight(const std::shared_ptr<PartyQuestSystem>& party_sys, const Vector3& reset_pos) noexcept
{
	const auto members = party_sys->GetPartyMembers();
	const auto mon_pos = m_patrol->GetComp<PositionComponent>()->pos;
	const auto mon_dir = CommonMath::Normalized(m_dest - m_start);
	const auto delta = mon_pos - m_prev_pos;
	for (const auto& member : members)
	{
		if (!member) continue;

		const auto pos_comp = member->GetComp<PositionComponent>();
		auto zero_y_pos = pos_comp->pos;
		zero_y_pos.y = 0.f;
		bool found = false;
		for (int i = 0; i <= 10; ++i)
		{
			const float t = static_cast<float>(i) / 10.0f;
			Vector3 interp_pos = m_prev_pos + delta * t;
			interp_pos.y = 0.f;
			if (Common::Fan::IsIntersect(interp_pos, mon_dir, zero_y_pos, 70.f, 4.f))
			{
				found = true;
				break;
			}
		}
		if (found)
		{
			pos_comp->pos = reset_pos;
			member->GetCurCluster()->Broadcast(Create_s2c_FORCED_MOVE(member->GetObjectID(), ToFlatVec(reset_pos)));
		}
	}
}

void InvadeQuest_1::InitQuestField() noexcept
{
	{
		AddPatrolUnit(
			Nagox::Enum::MONSTER_TYPE_BEAR,
			Vector3(-29.655296F, 78.92489F, -100.080925F),
			Vector3(-47.767162F, 78.92489F, -107.52249F),
			3.f);
	}
	{
		AddPatrolUnit(
			Nagox::Enum::MONSTER_TYPE_BEAR,
			Vector3(-48.67735F, 78.92489F, -102.31094F),
			Vector3(-34.25242F, 78.96087F, -105.86635F),
			3.f);
	}
	{
		AddPatrolUnit(
			Nagox::Enum::MONSTER_TYPE_BEAR,
			Vector3(-17.33981F, 81.89751F, -130.25797F),
			Vector3(-28.772465F, 81.730095F, -113.34569F),
			3.f);
	}
	SetTargetObject(Vector3(-3.6082437F, 81.624916F, -121.17864F));
	StartUpdate(Vector3(-38.879353F, 75.17851F, -85.985756F));
}

void InvadeQuest_2::InitQuestField() noexcept
{
	{
		AddPatrolUnit(
			Nagox::Enum::MONSTER_TYPE_BEAR,
			Vector3(-99.57018F, 79.59282F, -261.79742F),
			Vector3(-84.79776F, 80.71591F, -285.81674F),
			10.f);
	}
	{
		AddPatrolUnit(
			Nagox::Enum::MONSTER_TYPE_BEAR,
			Vector3(-64.295166F, 77.92885F, -287.31644F),
			Vector3(-73.56366F, 76.989655F, -257.2729F),
			10.f);
	}
	{
		AddPatrolUnit(
			Nagox::Enum::MONSTER_TYPE_BEAR,
			Vector3(-59.886127F, 75.013176F, -249.21278F),
			Vector3(-32.887623F, 74.41561F, -266.91882F),
			10.f);
	}
	{
		AddPatrolUnit(
			Nagox::Enum::MONSTER_TYPE_BEAR,
			Vector3(-11.032458F, 72.16888F, -236.6082F),
			Vector3(0.25170872F, 71.84044F, -258.67227F),
			10.f);
	}
	{
		AddPatrolUnit(
			Nagox::Enum::MONSTER_TYPE_BEAR,
			Vector3(-85.54531F, 77.76605F, -266.77887F),
			Vector3(-80.107574F, 80.24683F, -286.1981F),
			10.f);
	}
	{
		AddPatrolUnit(
			Nagox::Enum::MONSTER_TYPE_BEAR,
			Vector3(-42.924076F, 72.60209F, -227.80591F),
			Vector3(-26.292973F, 74.84331F, -270.19565F),
			10.f);
	}
	SetTargetObject(Vector3(16.376675F, 70.53413F, -243.55034F));
	StartUpdate(Vector3(-126.12132F, 81.613266F, -267.6741F));
}

void NPCGuardQuest2::InitQuestField() noexcept
{
	SetPathNPC({
	   Vector3(215.83626F,81.56483F,127.289085F)
		,Vector3(210.23375F,82.09833F,140.25215F)
		,Vector3(202.44603F,82.04046F,142.32162F)
		,Vector3(195.5804F,82.28131F,142.95117F)
		,Vector3(182.68251F,82.870056F,138.94922F)
		,Vector3(173.06229F,83.266F,133.08772F)
		,Vector3(163.60434F,83.29547F,122.55677F)
		,Vector3(158.2657F,83.45808F,117.04457F)
		,Vector3(139.16446F,85.24577F,115.83121F)
		,Vector3(123.09017F,87.060265F,107.61383F)
		,Vector3(104.11417F,87.44576F,98.76893F)
		,Vector3(83.39752F,90.79084F,101.76995F)
		});
	SetMonsters({
		Vector3(276.3846F,87.071205F,132.33232F)
		,Vector3(270.6501F,87.435905F,144.14229F)
		,Vector3(262.24268F,86.312035F,142.35698F)
		,Vector3(262.05365F,84.54996F,131.11317F)
		,Vector3(253.66066F,82.50959F,126.64804F)
		,Vector3(242.14177F,82.71103F,136.88171F)
		,Vector3(231.86168F,81.23654F,127.57626F)
		,Vector3(200.2323F,81.90057F,132.80669F)
		,Vector3(186.08543F,82.504654F,133.19452F)
		,Vector3(167.60686F,83.77578F,138.95009F)
		,Vector3(172.88174F,84.0878F,144.22244F)
		,Vector3(160.4144F,83.77346F,131.8457F)
		,Vector3(155.5111F,83.69453F,121.039764F)
		,Vector3(150.09778F,84.05262F,108.284706F)
		,Vector3(138.35092F,85.30143F,102.45897F)
		,Vector3(129.41908F,86.021F,104.367195F)
		,Vector3(121.49226F,87.21519F,112.734474F)
		,Vector3(109.68462F,87.03806F,108.25743F)
		,Vector3(122.89287F,87.17902F,116.92817F)
		});
	SetQuestBeginPos(Vector3(312.29892F, 85.07235F, 138.55077F));
}

void NPCGuardQuest2::SetMonsters(const XVector<Vector3> points) noexcept
{
	const auto num = (int)points.size();
	for (int i = 0; i < num; ++i)
	{
		EntityBuilder b;
		b.group_type = Nagox::Enum::GROUP_TYPE::GROUP_TYPE_MONSTER;
		b.obj_type = Nagox::Enum::MONSTER_TYPE_FOX;
		const auto m = EntityFactory::CreateMonster(b);
		auto p = points[i];
		m->GetComp<NaviAgent>()->SetPos(p);
		m->GetComp<PositionComponent>()->pos = p;
		const auto pos = p;
		EnterFieldWithFloatXYNPC(pos.x + 512.f, pos.z + 512.f, m);
	}
}

void CombinationBattleQuest::InitQuestField() noexcept
{
	const Vector3 mon_quest_pos[]
	{
		Vector3(-14.789355F,86.360855F,-329.16034F)
		,Vector3(-10.508543F,85.54358F,-326.56857F)
		,Vector3(-4.273209F,84.762F,-323.69095F)
		,Vector3(2.495394F,84.92774F,-321.60648F)
		,Vector3(9.827456F,85.29504F,-321.7305F)
		,Vector3(18.507647F,84.951355F,-319.18286F)
		,Vector3(21.764019F,84.48343F,-325.29858F)
		,Vector3(19.653528F,83.73852F,-333.0341F)
		,Vector3(14.543057F,83.52652F,-336.2194F)
		,Vector3(6.175371F,84.30505F,-337.25052F)
		,Vector3(-4.27499F,84.92839F,-335.6931F)
	};
	const auto num = (int)(sizeof(mon_quest_pos) / sizeof(mon_quest_pos[0]));
	const auto num_half = num / 2;
	//m_mon_count.store_relaxed(num);

	for (int i = 0; i < num_half; ++i)
	{
		EntityBuilder b;
		b.group_type = Nagox::Enum::GROUP_TYPE::GROUP_TYPE_MONSTER;
		b.obj_type = Nagox::Enum::MONSTER_TYPE_FOX;
		const auto m = EntityFactory::CreateMonster(b);
		//static_cast<Regenerator*>(m->GetDeleter())->m_targetField = SharedFromThis<NagiocpX::Field>();

		m->GetComp<PositionComponent>()->pos = mon_quest_pos[i];
		m->GetComp<NaviAgent>()->SetPos(mon_quest_pos[i]);
		const auto pos = m->GetComp<PositionComponent>()->pos;
		EnterFieldWithFloatXYNPC(pos.x + 512.f, pos.z + 512.f, m);
		++m_mon_count;
		//EnterFieldNPC(m);
	}
	for (int i = num_half; i < num; ++i)
	{
		EntityBuilder b;
		b.group_type = Nagox::Enum::GROUP_TYPE::GROUP_TYPE_MONSTER;
		b.obj_type = Nagox::Enum::MONSTER_TYPE_BEAR;
		const auto m = EntityFactory::CreateMonster(b);
		//static_cast<Regenerator*>(m->GetDeleter())->m_targetField = SharedFromThis<NagiocpX::Field>();

		m->GetComp<PositionComponent>()->pos = mon_quest_pos[i];
		m->GetComp<NaviAgent>()->SetPos(mon_quest_pos[i]);
		const auto pos = m->GetComp<PositionComponent>()->pos;
		EnterFieldWithFloatXYNPC(pos.x + 512.f, pos.z + 512.f, m);
		//EnterFieldNPC(m);
		++m_mon_count;
	}
	SetQuestBeginPos(Vector3(-36.739315F, 88.53785F, -354.69366F));
}

void NexusQuest::InitQuestField() noexcept
{
	SetQuestBeginPos(Vector3(177.23233F, 76.67912F, -195.14803F));
}
