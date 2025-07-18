#include "pch.h"
#include "BossBehaviorNode.h"
#include "TickTimer.h"
#include "PathFinder_Common.h"
#include "ClusterInfoHelper.h"
#include "MoveBroadcaster.h"
#include "Projectile.h"
#include "NaviAgent_Common.h"
#include "NavigationMesh.h"

using namespace NagiocpX;

const Vector3 g_jump_shoot_pos[]
{
	Vector3(-115.95728F,24.735518F,-241.00452F),
	Vector3(14.283817F,22.855463F,-193.01305F),
	Vector3(23.96977F,23.393854F,-216.5209F),
	Vector3(-110.828636F,15.338949F,-219.69366F),
	Vector3(-84.92324F,15.455506F,-220.32578F),
	Vector3(-52.685413F,14.0523615F,-211.08817F),
	Vector3(-30.660482F,14.17017F,-214.1239F)
};
constexpr const size_t g_num_of_rand_pos = sizeof(g_jump_shoot_pos) / sizeof(g_jump_shoot_pos[0]); \
static constinit int g_pos_idx = 0;
const Vector3 g_reset_pos = Vector3(-47.336597F, 18.003374F, -244.53511F);

S_ptr<ContentsEntity> cur_target = {};
int g_cur_target_idx = 0;
float g_cur_target_acc[3]{ 5.f,5.f,5.f };

NodeStatus SelectPattern::Tick(const ComponentSystemNPC* const owner_comp_sys, TickTimerBT* const bt_root_timer, const NagiocpX::S_ptr<NagiocpX::ContentsEntity>& awaker) noexcept
{
	const auto DT = bt_root_timer->GetFloatDT();
	const float r = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
	const auto& player_list = bt_root_timer->GetTempVecForInsightObj();
	// TODO: 적당한 유저 찾기
	if (player_list.empty())return NodeStatus::FAILURE;
	g_cur_target_acc[g_cur_target_idx] -= DT;
	if (0.f >= g_cur_target_acc[g_cur_target_idx])
	{
		g_cur_target_acc[g_cur_target_idx] = 5.f;
		g_cur_target_idx = (g_cur_target_idx + 1) % player_list.size();
	}
	cur_target = player_list[g_cur_target_idx]->SharedFromThis();
	if (r < m_probability)
	{
		//if (max_count == m_count++)
		{
			//m_count = 0;
			m_probability = m_origin_prob;
			//return NodeStatus::FAILURE;
		}
		m_probability = std::max(0.f, m_probability - 0.5f);
		return NodeStatus::SUCCESS;
	}
	else
	{
		m_probability = m_origin_prob;
		return NodeStatus::FAILURE;
	}
}

NodeStatus SelectTarget::Tick(const ComponentSystemNPC* const owner_comp_sys, TickTimerBT* const bt_root_timer, const NagiocpX::S_ptr<NagiocpX::ContentsEntity>& awaker) noexcept
{
	const auto& player_list = bt_root_timer->GetTempVecForInsightObj();
	// TODO: 적당한 유저 찾기
	const auto DT = bt_root_timer->GetFloatDT();
	if(player_list.empty())return NodeStatus::FAILURE;
	g_cur_target_acc[g_cur_target_idx] -= DT;
	if (0.f >= g_cur_target_acc[g_cur_target_idx])
	{
		g_cur_target_acc[g_cur_target_idx] = 5.f;
		g_cur_target_idx = (g_cur_target_idx + 1) % player_list.size();
	}
	cur_target = player_list[g_cur_target_idx]->SharedFromThis();
	return NodeStatus::SUCCESS;
}

NodeStatus MoveToTarget::Tick(const ComponentSystemNPC* const owner_comp_sys, TickTimerBT* const bt_root_timer, const NagiocpX::S_ptr<NagiocpX::ContentsEntity>& awaker) noexcept
{
	if(!cur_target)return NodeStatus::FAILURE;
	const auto& player_list = bt_root_timer->GetTempVecForInsightObj();
	// 유저에게 이동
	const auto boss_entity = owner_comp_sys->GetOwnerEntity();
	const auto DT = bt_root_timer->GetFloatDT();
	constexpr const float BOSS_SPEED = 10.f;
	constexpr const float MELLE_ATK_DIST = 2.f;

	const auto target_pos_comp = cur_target->GetComp<PositionComponent>();
	const auto boss_pos_comp = owner_comp_sys->GetComp<PositionComponent>();

	const auto target_pos = target_pos_comp->pos;
	const auto boss_pos = boss_pos_comp->pos;
	g_cur_target_acc[g_cur_target_idx] -= DT;
	if (0.f >= g_cur_target_acc[g_cur_target_idx])
	{
		g_cur_target_acc[g_cur_target_idx] = 5.f;
		g_cur_target_idx = (g_cur_target_idx + 1) % player_list.size();
	}
	cur_target = player_list[g_cur_target_idx]->SharedFromThis();
	if (CommonMath::IsInDistanceDX(target_pos, boss_pos, MELLE_ATK_DIST))return NodeStatus::SUCCESS;
	extern constinit thread_local float straightPathRaw[10 * 3];
	std::span<Vector3> path_point;
	{
		constexpr const uint8_t PATH_COUNT = 10;
		int pathCount = 1;
		const auto start_z_pos = CommonMath::InverseZ(boss_pos);
		auto dest_z_pos = CommonMath::InverseZ(target_pos);
		const auto agent = owner_comp_sys->GetComp<NaviAgent>();
		const auto nav = agent->GetNavMesh();
		const auto nav_q = nav->GetNavMeshQuery();
		const auto nav_f = nav->GetNavFilter();
		const auto start_poly = agent->GetAgentConcreate()->GetCurCell().GetPolyRef();
		dtPolyRef dest_poly = 0;
		dtPolyRef path[PATH_COUNT]{ start_poly };

		dtStatus status = nav_q->findNearestPoly(&dest_z_pos.x, Common::NaviCell::g_extent, nav_f, &dest_poly, &dest_z_pos.x);

		if (dtStatusFailed(status))
		{
			// std::cout << "못 찾음\n";
			return NodeStatus::FAILURE;
		}
		
		status = nav_q->findPath(start_poly, dest_poly, &start_z_pos.x, &dest_z_pos.x, nav_f, path, &pathCount, PATH_COUNT);

		if (dtStatusFailed(status))
		{
			// std::cout << "못 찾음\n";
			return NodeStatus::FAILURE;
		}

		const auto straightPath = (Vector3*)straightPathRaw;
		unsigned char straightPathFlags[PATH_COUNT];
		dtPolyRef straightPathPolys[PATH_COUNT];
		int straightPathCount = 0;


		status = nav_q->findStraightPath(&start_z_pos.x, &dest_z_pos.x, path, pathCount, &straightPath[0].x, straightPathFlags, straightPathPolys, &straightPathCount, PATH_COUNT);
		if (dtStatusFailed(status))
		{
			//std::cout << "못 찾음\n";
			return NodeStatus::FAILURE;
		}

		auto b = straightPath;
		const auto e = straightPath + straightPathCount;
		while (e != b)
		{
			b->z = -b->z;
			++b;
		}
		path_point = std::span <Vector3>{ straightPath,straightPath + straightPathCount };
	}


	
	if(path_point.empty())return NodeStatus::FAILURE;
	auto dir = path_point.size() >= 2 ? path_point[1] : path_point[0];
	dir -= boss_pos;
	dir.Normalize();

	const auto dx2 = boss_pos.x + dir.x * BOSS_SPEED * DT;
	const auto dy2 = boss_pos.y + dir.y * BOSS_SPEED * DT;
	const auto dz2 = boss_pos.z + dir.z * BOSS_SPEED * DT;
	const Vector3 dest_pos2{ dx2, dy2, dz2 };

	boss_entity->GetComp<PositionComponent>()->AdjustMovement(DT, dir, BOSS_SPEED);

	boss_entity->GetComp<PositionComponent>()->body_angle = atan2f(dir.x, dir.z) * 180.f / 3.141592f;
	
	bt_root_timer->BroadcastObjInSight(bt_root_timer->GetTempVecForInsightObj(),
		Create_s2c_BOSS_MOVE(ToFlatVec(boss_entity->GetComp<PositionComponent>()->pos), 20.f, Nagox::Enum::BOSS_MOVE_TYPE_BOSS_MOVE_TYPE_1)
	);

	return NodeStatus::RUNNING;
}

NodeStatus MeleeAtack::Tick(const ComponentSystemNPC* const owner_comp_sys, TickTimerBT* const bt_root_timer, const NagiocpX::S_ptr<NagiocpX::ContentsEntity>& awaker) noexcept
{
	if (!cur_target)return NodeStatus::FAILURE;
	const auto dest_pos = cur_target->GetComp<PositionComponent>()->pos;
	const auto cur_pos = owner_comp_sys->GetComp<PositionComponent>()->pos;
	const auto DT = bt_root_timer->GetFloatDT();
	const auto dx = dest_pos.x - cur_pos.x;
	const auto dy = dest_pos.y - cur_pos.y;
	const auto dz = dest_pos.z - cur_pos.z;

	
	const float m_attack_range = 10.f;
	// TODO: 사거리 및 횟수
	if (m_attack_range * m_attack_range <= dx * dx + dy * dy + dz * dz) 
	{
		m_accTime = 2.f;
		return NodeStatus::SUCCESS;
	}
	
	m_accTime -= bt_root_timer->GetFloatDT();
	

	if (0.f >= m_accTime)
	{
		--m_count;
		auto proj = NagiocpX::TimerHandler::CreateTimerWithoutHandle<MonProjectile>(1);
		if (rand() & 1)
		{
			//proj.timer->m_pos = (owner_comp_sys->GetComp<PositionComponent>()->pos) - CommonMath::Normalized(Vector3{ dx,dy,dz }) * 0.1f;
			proj.timer->m_pos = owner_comp_sys->GetComp<PositionComponent>()->pos + Vector3{ 0,10,0 };
			proj.timer->m_proj_type = 1;
			//proj.timer->m_accDist = 99.9f;
			//proj.timer->m_
			proj.timer->SelectObjList(bt_root_timer->GetTempVecForInsightObj());
			proj.timer->m_speed = Vector3{ 0,-1,0 }*10.f;
			//proj.timer->m_speed = CommonMath::Normalized(Vector3{ dx,dy,dz }) * 10.f;
			proj.timer->m_radius = 2.f;
		}
		else
		{
			proj.timer->m_pos = (owner_comp_sys->GetComp<PositionComponent>()->pos) - CommonMath::Normalized(Vector3{ dx,dy,dz }) * 0.1f;
			proj.timer->m_proj_type = 1;
			proj.timer->m_accDist = 99.9f;
			proj.timer->SelectObjList(bt_root_timer->GetTempVecForInsightObj());
			proj.timer->m_speed = CommonMath::Normalized(Vector3{ dx,dy,dz }) * 10.f;
			proj.timer->m_radius = 2.f;
		}
		proj.timer->m_owner = owner_comp_sys->GetOwnerEntity()->SharedFromThis();
		bt_root_timer->BroadcastObjInSight(bt_root_timer->GetTempVecForInsightObj(), Create_s2c_MONSTER_ATTACK(owner_comp_sys->GetOwnerEntity()->GetObjectID(), cur_target->GetObjectID(), 1));
		// TODO: 진짜 HP깎기
		m_accTime = 2.f;
		if (m_count == 0)
		{
			std::uniform_int_distribution<> uid{ 1,5 };
			m_count = uid(LRandEngine);
			return NodeStatus::SUCCESS;
		}
		else
		{
			return NodeStatus::RUNNING;
		}
	}

	return NodeStatus::RUNNING;
}

NodeStatus SelectJumpPoint::Tick(const ComponentSystemNPC* const owner_comp_sys, TickTimerBT* const bt_root_timer, const NagiocpX::S_ptr<NagiocpX::ContentsEntity>& awaker) noexcept
{
	const auto DT = bt_root_timer->GetFloatDT();
	m_accTime -= DT;
	if (0.f < m_accTime)
	{
		return NodeStatus::RUNNING;
	}
	m_accTime = 1.f;
	// TODO: 순간이동보단 고속이동
	const auto idx = g_pos_idx;
	g_pos_idx = (g_pos_idx + 1) % g_num_of_rand_pos;

	const auto target_pos = g_jump_shoot_pos[idx];
	const auto boss_entity = owner_comp_sys->GetOwnerEntity();
	const auto dir = CommonMath::Normalized(target_pos - boss_entity->GetComp<PositionComponent>()->pos);

	boss_entity->GetComp<PositionComponent>()->pos = target_pos;
	boss_entity->GetComp<PositionComponent>()->body_angle = atan2f(dir.x, dir.z) * 180.f / 3.141592f;

	bt_root_timer->BroadcastObjInSight(bt_root_timer->GetTempVecForInsightObj(),
		Create_s2c_BOSS_FLY(ToFlatVec(boss_entity->GetComp<PositionComponent>()->pos),Nagox::Enum::BOSS_FLY_TYPE_BOSS_FLY_TYPE_1)
	);
	return NodeStatus::SUCCESS;
}

NodeStatus ResetPos::Tick(const ComponentSystemNPC* const owner_comp_sys, TickTimerBT* const bt_root_timer, const NagiocpX::S_ptr<NagiocpX::ContentsEntity>& awaker) noexcept
{
	// TODO: 순간이동보단 고속이동
	
	const auto target_pos = g_reset_pos;
	const auto boss_entity = owner_comp_sys->GetOwnerEntity();
	const auto dir = CommonMath::Normalized(target_pos - boss_entity->GetComp<PositionComponent>()->pos);

	boss_entity->GetComp<PositionComponent>()->pos = target_pos;
	boss_entity->GetComp<PositionComponent>()->body_angle = atan2f(dir.x, dir.z) * 180.f / 3.141592f;

	
	bt_root_timer->BroadcastObjInSight(bt_root_timer->GetTempVecForInsightObj(),
		Create_s2c_BOSS_FLY(ToFlatVec(boss_entity->GetComp<PositionComponent>()->pos), Nagox::Enum::BOSS_FLY_TYPE_BOSS_FLY_TYPE_2));
	return NodeStatus::SUCCESS;
}

NodeStatus ShootFireBall::Tick(const ComponentSystemNPC* const owner_comp_sys, TickTimerBT* const bt_root_timer, const NagiocpX::S_ptr<NagiocpX::ContentsEntity>& awaker) noexcept
{
	const auto& player_list = bt_root_timer->GetTempVecForInsightObj();
	// TODO: 적당한 유저 찾기	
	const auto boss_entity = owner_comp_sys->GetOwnerEntity();
	if (player_list.empty())return NodeStatus::FAILURE;
	const auto DT = bt_root_timer->GetFloatDT();
	m_accTime -= DT;
	if (count == 0 && 0.f >= m_accTime)
	{
		count = 5;
		m_accTime = 2.f;
		return NodeStatus::SUCCESS;
	}
	if (count == 0 || 0.f < m_accTime)
	{
		return NodeStatus::RUNNING;
	}
	const auto proj = NagiocpX::TimerHandler::CreateTimerWithoutHandle<MonProjectile>(10);

	proj.timer->m_pos = (owner_comp_sys->GetComp<PositionComponent>()->pos);

	for (const auto users : player_list)
	{
		

		auto dir = users->GetComp<PositionComponent>()->pos - proj.timer->m_pos;
		dir.Normalize();

		proj.timer->m_speed = dir * 20.f;

		proj.timer->SelectObjList(player_list);
		proj.timer->m_owner = owner_comp_sys->GetOwnerEntity()->SharedFromThis();
	}
	
	m_accTime = .5f;
	--count;
	if (count == 0)
	{
		m_accTime = 2.f;
	}
	return NodeStatus::RUNNING;
}

NodeStatus SetMeteorPos::Tick(const ComponentSystemNPC* const owner_comp_sys, TickTimerBT* const bt_root_timer, const NagiocpX::S_ptr<NagiocpX::ContentsEntity>& awaker) noexcept
{
	const auto DT = bt_root_timer->GetFloatDT();
	m_accTime -= DT;
	if (0.f < m_accTime)
	{
		return NodeStatus::RUNNING;
	}
	
	const auto target_pos = g_reset_pos + Vector3{ 0,10,0 };
	const auto boss_entity = owner_comp_sys->GetOwnerEntity();
	const auto dir = CommonMath::Normalized(target_pos - boss_entity->GetComp<PositionComponent>()->pos);

	boss_entity->GetComp<PositionComponent>()->pos = target_pos;
	boss_entity->GetComp<PositionComponent>()->body_angle = atan2f(dir.x, dir.z) * 180.f / 3.141592f;

	bt_root_timer->BroadcastObjInSight(bt_root_timer->GetTempVecForInsightObj(),
		Create_s2c_BOSS_FLY(ToFlatVec(boss_entity->GetComp<PositionComponent>()->pos), Nagox::Enum::BOSS_FLY_TYPE_BOSS_FLY_TYPE_1)
	);
	m_accTime = 1.f;
	return NodeStatus::SUCCESS;
}

NodeStatus FireMeteor::Tick(const ComponentSystemNPC* const owner_comp_sys, TickTimerBT* const bt_root_timer, const NagiocpX::S_ptr<NagiocpX::ContentsEntity>& awaker) noexcept
{
	const auto& player_list = bt_root_timer->GetTempVecForInsightObj();
	const auto boss_entity = owner_comp_sys->GetOwnerEntity();
	if (player_list.empty())return NodeStatus::FAILURE;
	const auto DT = bt_root_timer->GetFloatDT();
	
	m_accTime2 -= DT;
	if (0.f < m_accTime2)
	{
		return NodeStatus::RUNNING;
	}
	if (0 == count)
	{
		count = 30;
		m_accTime = .5f;
		m_accTime2 = 2.5f;
		return NodeStatus::SUCCESS;
	}
	m_accTime -= DT;
	if (0.f < m_accTime)
	{
		return NodeStatus::RUNNING;
	}
	
	const auto target_idx = rand() % player_list.size();
	const auto target_user = player_list[target_idx];
	const auto target_pos_comp = target_user->GetComp<PositionComponent>();
	const auto target_mid_pos = target_pos_comp->pos;
	Vector3 fire_pos = target_mid_pos;
	const auto nav_mesh = NAVIGATION->GetNavMesh(NAVI_MESH_TYPE::BOSS_ROOM);
	const auto nav_q = nav_mesh->GetNavMeshQuery();
	for (int i = 0; i < 5; ++i)
	{
		nav_mesh->findRandomPointAroundCircle(
			&owner_comp_sys->GetComp<PositionComponent>()->pos.x,
			30.f,
			&fire_pos.x);

		const auto proj = NagiocpX::TimerHandler::CreateTimerWithoutHandle<MonProjectile>(10);

		proj.timer->m_pos = (owner_comp_sys->GetComp<PositionComponent>()->pos);

		auto dir = fire_pos - proj.timer->m_pos;
		dir.Normalize();

		for (const auto user : player_list)
		{
			user->GetSession()->SendAsync(Create_s2c_BOSS_PROJ_MARK(
				ToFlatVec(fire_pos)
			));
		}

		proj.timer->m_speed = dir * 15.f;

		proj.timer->SelectObjList(player_list);
		proj.timer->m_owner = owner_comp_sys->GetOwnerEntity()->SharedFromThis();
		
	}
	--count;
	m_accTime = .3f;
	return NodeStatus::RUNNING;
}
