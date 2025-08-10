#include "pch.h"
#include "BossBehaviorNode.h"
#include "TickTimer.h"
#include "PathFinder_Common.h"
#include "ClusterInfoHelper.h"
#include "MoveBroadcaster.h"
#include "Projectile.h"
#include "NaviAgent_Common.h"
#include "NavigationMesh.h"
#include "HP.h"

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
constexpr const size_t g_num_of_rand_pos = sizeof(g_jump_shoot_pos) / sizeof(g_jump_shoot_pos[0]);
const Vector3 g_reset_pos = Vector3(-47.336597F, 18.003374F, -244.53511F);



static inline const float CalculateDelayTime(const Vector3& start, const Vector3& dest) {
	return 2.5f + (Vector3::Distance(start, dest) / 10.f);
}



NodeStatus SelectPattern::Tick(const ComponentSystemNPC* const owner_comp_sys, TickTimerBT* const bt_root_timer, const NagiocpX::S_ptr<NagiocpX::ContentsEntity>& awaker) noexcept
{
	const auto DT = bt_root_timer->GetFloatDT();
	const auto boss_storage_node = static_cast<BossStorageNode* const>(bt_root_timer->GetRootNode());

	const float r = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
	const auto& player_list = bt_root_timer->GetTempVecForInsightObj();
	// TODO: 적당한 유저 찾기
	const auto boss_entity = owner_comp_sys->GetOwnerEntity();
	bt_root_timer->BroadcastObjInSight(bt_root_timer->GetTempVecForInsightObj(),
		Create_s2c_BOSS_MOVE(ToFlatVec(boss_entity->GetComp<PositionComponent>()->pos), 20.f, Nagox::Enum::BOSS_MOVE_TYPE_BOSS_MOVE_TYPE_1, boss_entity->GetComp<PositionComponent>()->body_angle)
	);
	if (player_list.empty())return NodeStatus::FAILURE;
	boss_storage_node->m_cur_target_acc[boss_storage_node->m_cur_target_idx] -= DT;
	if (0.f >= boss_storage_node->m_cur_target_acc[boss_storage_node->m_cur_target_idx])
	{
		boss_storage_node->m_cur_target_acc[boss_storage_node->m_cur_target_idx] = 5.f;
		boss_storage_node->m_cur_target_idx = (boss_storage_node->m_cur_target_idx + 1) % player_list.size();
	}
	boss_storage_node->m_cur_target = player_list[boss_storage_node->m_cur_target_idx]->SharedFromThis();
	if (BOSS_PHASE::PHASE_2 == boss_storage_node->m_boss_phase)
	{
		if (m_bIsBreath && m_bIsFirstBreath)
		{
			m_bIsFirstBreath = false;
			return NodeStatus::SUCCESS;
		}
	}
	if (r < m_probability)
	{
		//if (max_count == m_count++)
		{
			//m_count = 0;
			m_probability = m_origin_prob;
			//return NodeStatus::FAILURE;
		}
		m_probability = std::max(0.f, m_probability - 1.0f);
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
	const auto boss_storage_node = static_cast<BossStorageNode* const>(bt_root_timer->GetRootNode());
	const auto& player_list = bt_root_timer->GetTempVecForInsightObj();
	// TODO: 적당한 유저 찾기
	const auto DT = bt_root_timer->GetFloatDT();
	if(player_list.empty())return NodeStatus::FAILURE;
	boss_storage_node->m_cur_target_acc[boss_storage_node->m_cur_target_idx] -= DT;
	if (0.f >= boss_storage_node->m_cur_target_acc[boss_storage_node->m_cur_target_idx])
	{
		boss_storage_node->m_cur_target_acc[boss_storage_node->m_cur_target_idx] = 5.f;
		boss_storage_node->m_cur_target_idx = (boss_storage_node->m_cur_target_idx + 1) % player_list.size();
	}
	boss_storage_node->m_cur_target = player_list[boss_storage_node->m_cur_target_idx]->SharedFromThis();
	return NodeStatus::SUCCESS;
}

NodeStatus MoveToTarget::Tick(const ComponentSystemNPC* const owner_comp_sys, TickTimerBT* const bt_root_timer, const NagiocpX::S_ptr<NagiocpX::ContentsEntity>& awaker) noexcept
{
	const auto boss_storage_node = static_cast<BossStorageNode* const>(bt_root_timer->GetRootNode());
	if(!boss_storage_node->m_cur_target)return NodeStatus::FAILURE;
	const auto& player_list = bt_root_timer->GetTempVecForInsightObj();
	// 유저에게 이동
	const auto boss_entity = owner_comp_sys->GetOwnerEntity();
	const auto DT = bt_root_timer->GetFloatDT();
	constexpr const float BOSS_SPEED = 10.f;
	const float MELLE_ATK_DIST = m_dist;

	
	m_accTime = 5.f;
	const auto target_pos_comp = boss_storage_node->m_cur_target->GetComp<PositionComponent>();
	const auto boss_pos_comp = owner_comp_sys->GetComp<PositionComponent>();
	bt_root_timer->BroadcastObjInSight(bt_root_timer->GetTempVecForInsightObj(),
		Create_s2c_BOSS_MOVE(ToFlatVec(boss_entity->GetComp<PositionComponent>()->pos), 20.f, Nagox::Enum::BOSS_MOVE_TYPE_BOSS_MOVE_TYPE_1, boss_entity->GetComp<PositionComponent>()->body_angle));
	const auto target_pos = target_pos_comp->pos;
	const auto boss_pos = boss_pos_comp->pos;
	boss_storage_node->m_cur_target_acc[boss_storage_node->m_cur_target_idx] -= DT;
	if (0.f >= boss_storage_node->m_cur_target_acc[boss_storage_node->m_cur_target_idx])
	{
		boss_storage_node->m_cur_target_acc[boss_storage_node->m_cur_target_idx] = 5.f;
		boss_storage_node->m_cur_target_idx = (boss_storage_node->m_cur_target_idx + 1) % player_list.size();
	}
	boss_storage_node->m_cur_target = player_list[boss_storage_node->m_cur_target_idx]->SharedFromThis();
	if (CommonMath::IsInDistanceDX(target_pos, boss_pos, MELLE_ATK_DIST))
	{
		m_accTime = 5.f;
		return NodeStatus::SUCCESS;
	}
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


		status = nav_q->findStraightPath(&start_z_pos.x, &dest_z_pos.x, path, pathCount, &straightPath[0].x, straightPathFlags, straightPathPolys, &straightPathCount, 2);
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
		Create_s2c_BOSS_MOVE(ToFlatVec(boss_entity->GetComp<PositionComponent>()->pos), 20.f, Nagox::Enum::BOSS_MOVE_TYPE_BOSS_MOVE_TYPE_1, boss_entity->GetComp<PositionComponent>()->body_angle)
	);

	return NodeStatus::RUNNING;
}

NodeStatus MeleeAtack::Tick(const ComponentSystemNPC* const owner_comp_sys, TickTimerBT* const bt_root_timer, const NagiocpX::S_ptr<NagiocpX::ContentsEntity>& awaker) noexcept
{
	const auto boss_storage_node = static_cast<BossStorageNode* const>(bt_root_timer->GetRootNode());
	if (!boss_storage_node->m_cur_target)return NodeStatus::FAILURE;
	const auto dest_pos = boss_storage_node->m_cur_target->GetComp<PositionComponent>()->pos;
	const auto cur_pos = owner_comp_sys->GetComp<PositionComponent>()->pos;
	const auto DT = bt_root_timer->GetFloatDT();
	const auto dx = dest_pos.x - cur_pos.x;
	const auto dy = dest_pos.y - cur_pos.y;
	const auto dz = dest_pos.z - cur_pos.z;

	const auto dir = CommonMath::Normalized(dest_pos - cur_pos);
	
	const float m_attack_range = 10.f;	
	const auto boss_entity = owner_comp_sys->GetOwnerEntity();
	boss_entity->GetComp<PositionComponent>()->body_angle = atan2f(dir.x, dir.z) * 180.f / 3.141592f;
	bt_root_timer->BroadcastObjInSight(bt_root_timer->GetTempVecForInsightObj(),
		Create_s2c_BOSS_MOVE(ToFlatVec(boss_entity->GetComp<PositionComponent>()->pos), 20.f, Nagox::Enum::BOSS_MOVE_TYPE_BOSS_MOVE_TYPE_1, boss_entity->GetComp<PositionComponent>()->body_angle)
	);
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
		//if (rand() & 1)
		{
			owner_comp_sys->GetComp<ClusterInfoHelper>()->BroadcastAllCluster(Create_s2c_BOSS_READY_TO_BREATH(
				boss_entity->GetComp<PositionComponent>()->body_angle));
			auto proj = NagiocpX::TimerHandler::CreateTimerWithoutHandle<MonProjectile>(1);
			//proj.timer->m_pos = (owner_comp_sys->GetComp<PositionComponent>()->pos) - CommonMath::Normalized(Vector3{ dx,dy,dz }) * 0.1f;
			proj.timer->m_dmg = 173;
			proj.timer->m_pos = owner_comp_sys->GetComp<PositionComponent>()->pos - dir * 10.f;
			proj.timer->m_proj_type = 1;
			//proj.timer->m_accDist = 99.9f;
			//proj.timer->m_
			proj.timer->SelectObjList(bt_root_timer->GetTempVecForInsightObj());
			proj.timer->m_speed = dir * 70.5f;
			//proj.timer->m_speed = CommonMath::Normalized(Vector3{ dx,dy,dz }) * 10.f;
			proj.timer->m_radius = 3.f;
			proj.timer->m_owner = owner_comp_sys->GetOwnerEntity()->SharedFromThis();
		}
		//else
		//{
		//	auto proj = NagiocpX::TimerHandler::CreateTimerWithoutHandle<MonProjectile>(1);
		//	proj.timer->m_pos = (owner_comp_sys->GetComp<PositionComponent>()->pos) - CommonMath::Normalized(Vector3{ dx,dy,dz }) * 0.1f;
		//	proj.timer->m_proj_type = 1;
		//	proj.timer->m_accDist = 99.9f;
		//	proj.timer->SelectObjList(bt_root_timer->GetTempVecForInsightObj());
		//	proj.timer->m_speed = CommonMath::Normalized(Vector3{ dx,dy,dz }) * 10.f;
		//	proj.timer->m_radius = 2.f;
		//}
		
		bt_root_timer->BroadcastObjInSight(bt_root_timer->GetTempVecForInsightObj(), Create_s2c_MONSTER_ATTACK(owner_comp_sys->GetOwnerEntity()->GetObjectID(), boss_storage_node->m_cur_target->GetObjectID(), 1));
		// TODO: 진짜 HP깎기
		m_accTime = 2.f;
		if (m_count == 0)
		{
			std::uniform_int_distribution<> uid{ 1,3 };
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
	const auto boss_storage_node = static_cast<BossStorageNode* const>(bt_root_timer->GetRootNode());
	const auto DT = bt_root_timer->GetFloatDT();
	m_accTime -= DT;
	if (m_delay_flag)
	{
		m_accTime2 -= DT;
		if (0.f >= m_accTime2)
		{
			m_accTime = 1.f;
			m_delay_flag = false;
			const auto boss_entity = owner_comp_sys->GetOwnerEntity();
			//bt_root_timer->BroadcastObjInSight(bt_root_timer->GetTempVecForInsightObj(),
			//	Create_s2c_BOSS_MOVE(ToFlatVec(boss_entity->GetComp<PositionComponent>()->pos), 20.f, Nagox::Enum::BOSS_MOVE_TYPE_BOSS_MOVE_TYPE_1)
			//);
			return NodeStatus::SUCCESS;
		}
		else
		{
			const auto boss_entity = owner_comp_sys->GetOwnerEntity();
			//bt_root_timer->BroadcastObjInSight(bt_root_timer->GetTempVecForInsightObj(),
			//	Create_s2c_BOSS_MOVE(ToFlatVec(boss_entity->GetComp<PositionComponent>()->pos), 20.f, Nagox::Enum::BOSS_MOVE_TYPE_BOSS_MOVE_TYPE_1)
			//);
			return NodeStatus::RUNNING;
		}
	}
	//if (0.f < m_accTime)
	//{
	//	const auto boss_entity = owner_comp_sys->GetOwnerEntity();
	//	bt_root_timer->BroadcastObjInSight(bt_root_timer->GetTempVecForInsightObj(),
	//		Create_s2c_BOSS_MOVE(ToFlatVec(boss_entity->GetComp<PositionComponent>()->pos), 20.f, Nagox::Enum::BOSS_MOVE_TYPE_BOSS_MOVE_TYPE_1)
	//	);
	//	return NodeStatus::RUNNING;
	//}
	
	m_accTime = 1.f;
	// TODO: 순간이동보단 고속이동
	const auto idx = boss_storage_node->m_pos_idx;
	boss_storage_node->m_pos_idx = (boss_storage_node->m_pos_idx + 1) % g_num_of_rand_pos;

	const auto target_pos = g_jump_shoot_pos[idx];
	const auto boss_entity = owner_comp_sys->GetOwnerEntity();
	const auto dir = CommonMath::Normalized(target_pos - boss_entity->GetComp<PositionComponent>()->pos);
	m_accTime2 = std::max(CalculateDelayTime(boss_entity->GetComp<PositionComponent>()->pos, target_pos),5.f);
	
	boss_entity->GetComp<PositionComponent>()->pos = target_pos;
	boss_entity->GetComp<PositionComponent>()->body_angle = atan2f(dir.x, dir.z) * 180.f / 3.141592f;

	bt_root_timer->BroadcastObjInSight(bt_root_timer->GetTempVecForInsightObj(),
		Create_s2c_BOSS_FLY(ToFlatVec(boss_entity->GetComp<PositionComponent>()->pos),Nagox::Enum::BOSS_FLY_TYPE_BOSS_FLY_TYPE_1)
	);
	m_delay_flag = true;
	boss_storage_node->m_prev_fire = true;
	return NodeStatus::RUNNING;
}

NodeStatus ResetPos::Tick(const ComponentSystemNPC* const owner_comp_sys, TickTimerBT* const bt_root_timer, const NagiocpX::S_ptr<NagiocpX::ContentsEntity>& awaker) noexcept
{
	const auto boss_storage_node = static_cast<BossStorageNode* const>(bt_root_timer->GetRootNode());
	// TODO: 순간이동보단 고속이동
	const auto DT = bt_root_timer->GetFloatDT();

	if (m_delay_flag)
	{
		m_accTime2 -= DT;
		if (0.f >= m_accTime2)
		{
			const auto boss_entity = owner_comp_sys->GetOwnerEntity();
			//m_accTime2 = 3.f;
			flag = true;
			const auto boss_pos_comp = owner_comp_sys->GetComp<PositionComponent>();
			//const auto boss_entity = owner_comp_sys->GetOwnerEntity();
			boss_entity->GetComp<PositionComponent>()->pos = g_reset_pos;
			bt_root_timer->BroadcastObjInSight(bt_root_timer->GetTempVecForInsightObj(),
				Create_s2c_BOSS_MOVE(ToFlatVec(boss_entity->GetComp<PositionComponent>()->pos), 20.f, Nagox::Enum::BOSS_MOVE_TYPE_BOSS_MOVE_TYPE_1, boss_entity->GetComp<PositionComponent>()->body_angle));
			m_delay_flag = false;
			return NodeStatus::SUCCESS;
		}
		else
		{
			const auto boss_pos_comp = owner_comp_sys->GetComp<PositionComponent>();
			const auto boss_entity = owner_comp_sys->GetOwnerEntity();
			bt_root_timer->BroadcastObjInSight(bt_root_timer->GetTempVecForInsightObj(),
				Create_s2c_BOSS_MOVE(ToFlatVec(boss_entity->GetComp<PositionComponent>()->pos), 20.f, Nagox::Enum::BOSS_MOVE_TYPE_BOSS_MOVE_TYPE_1, boss_entity->GetComp<PositionComponent>()->body_angle));
			return NodeStatus::RUNNING;
		}
	}
	
	const auto target_pos = g_reset_pos;
	const auto boss_entity = owner_comp_sys->GetOwnerEntity();
	const auto dir = CommonMath::Normalized(target_pos - boss_entity->GetComp<PositionComponent>()->pos);
	boss_entity->GetComp<PositionComponent>()->pos = target_pos;
	
	boss_entity->GetComp<PositionComponent>()->body_angle = atan2f(dir.x, dir.z) * 180.f / 3.141592f;

	m_delay_flag = true;
	//std::cout << CalculateDelayTime(boss_entity->GetComp<PositionComponent>()->pos, target_pos) << std::endl;
	m_accTime2 = std::max(CalculateDelayTime(boss_entity->GetComp<PositionComponent>()->pos, target_pos), 5.f);
	if (boss_storage_node->m_prev_fire)
	{
		m_accTime2 = 12.5f;
		boss_storage_node->m_prev_fire = false;
	}
	bt_root_timer->BroadcastObjInSight(bt_root_timer->GetTempVecForInsightObj(),
		Create_s2c_BOSS_FLY(ToFlatVec(target_pos), Nagox::Enum::BOSS_FLY_TYPE_BOSS_FLY_TYPE_2));

	return NodeStatus::RUNNING;
}

NodeStatus ShootFireBall::Tick(const ComponentSystemNPC* const owner_comp_sys, TickTimerBT* const bt_root_timer, const NagiocpX::S_ptr<NagiocpX::ContentsEntity>& awaker) noexcept
{
	const auto boss_storage_node = static_cast<BossStorageNode* const>(bt_root_timer->GetRootNode());
	const auto& player_list = bt_root_timer->GetTempVecForInsightObj();
	// TODO: 적당한 유저 찾기	
	const auto boss_entity = owner_comp_sys->GetOwnerEntity();
	if (player_list.empty())return NodeStatus::FAILURE;
	if(!boss_storage_node->m_cur_target)return NodeStatus::FAILURE;
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
	const auto boss_pos = boss_entity->GetComp<PositionComponent>()->pos;
	for (const auto users : player_list)
	{
		
		const auto proj = NagiocpX::TimerHandler::CreateTimerWithoutHandle<MonProjectile>(10);

		proj.timer->m_radius = 0.125f;
		proj.timer->m_pos = boss_pos;
		auto dir = users->GetComp<PositionComponent>()->pos - proj.timer->m_pos;
		dir.Normalize();

		proj.timer->m_speed = dir * 50.f;
		proj.timer->m_max_dist = 100.f;
		proj.timer->SelectObjList(player_list);
		proj.timer->m_owner = owner_comp_sys->GetOwnerEntity()->SharedFromThis();
		const auto cur_dir = CommonMath::Normalized(boss_storage_node->m_cur_target->GetComp<PositionComponent>()->pos - boss_pos);
		boss_entity->GetComp<PositionComponent>()->body_angle = atan2f(cur_dir.x, cur_dir.z) * 180.f / 3.141592f;

		bt_root_timer->BroadcastObjInSight(bt_root_timer->GetTempVecForInsightObj(),
			Create_s2c_BOSS_MOVE(ToFlatVec(boss_entity->GetComp<PositionComponent>()->pos), 20.f, Nagox::Enum::BOSS_MOVE_TYPE_BOSS_MOVE_TYPE_1, boss_entity->GetComp<PositionComponent>()->body_angle));
	}
	
	m_accTime = .5f;
	--count;
	if (count == 0)
	{
		bt_root_timer->BroadcastObjInSight(bt_root_timer->GetTempVecForInsightObj(),
			Create_s2c_BOSS_MOVE(ToFlatVec(boss_entity->GetComp<PositionComponent>()->pos), 20.f, Nagox::Enum::BOSS_MOVE_TYPE_BOSS_MOVE_TYPE_1, boss_entity->GetComp<PositionComponent>()->body_angle));
		m_accTime = 5.f;
	}
	return NodeStatus::RUNNING;
}

NodeStatus SetMeteorPos::Tick(const ComponentSystemNPC* const owner_comp_sys, TickTimerBT* const bt_root_timer, const NagiocpX::S_ptr<NagiocpX::ContentsEntity>& awaker) noexcept
{
	const auto DT = bt_root_timer->GetFloatDT();
	if (m_delay_flag)
	{
		m_accTime2 -= DT;
		if (0.f >= m_accTime2)
		{
			m_delay_flag = false;
			const auto boss_entity = owner_comp_sys->GetOwnerEntity();
			bt_root_timer->BroadcastObjInSight(bt_root_timer->GetTempVecForInsightObj(),
				Create_s2c_BOSS_MOVE(ToFlatVec(boss_entity->GetComp<PositionComponent>()->pos), 20.f, Nagox::Enum::BOSS_MOVE_TYPE_BOSS_MOVE_TYPE_1, boss_entity->GetComp<PositionComponent>()->body_angle));
			return NodeStatus::SUCCESS;
		}
		else
		{
			return NodeStatus::RUNNING;
		}
	}
	
	const auto target_pos = g_reset_pos + Vector3{ 0,10,0 };
	const auto boss_entity = owner_comp_sys->GetOwnerEntity();
	const auto dir = CommonMath::Normalized(target_pos - boss_entity->GetComp<PositionComponent>()->pos);

	boss_entity->GetComp<PositionComponent>()->pos = target_pos;
	boss_entity->GetComp<PositionComponent>()->body_angle = atan2f(dir.x, dir.z) * 180.f / 3.141592f;

	bt_root_timer->BroadcastObjInSight(bt_root_timer->GetTempVecForInsightObj(),
		Create_s2c_BOSS_FLY(ToFlatVec(boss_entity->GetComp<PositionComponent>()->pos), Nagox::Enum::BOSS_FLY_TYPE_BOSS_FLY_TYPE_1)
	);
	m_accTime2 = CalculateDelayTime(boss_entity->GetComp<PositionComponent>()->pos, target_pos);
	m_accTime = 1.f;
	m_delay_flag = true;
	return NodeStatus::RUNNING;
}

NodeStatus FireMeteor::Tick(const ComponentSystemNPC* const owner_comp_sys, TickTimerBT* const bt_root_timer, const NagiocpX::S_ptr<NagiocpX::ContentsEntity>& awaker) noexcept
{
	const auto& player_list = bt_root_timer->GetTempVecForInsightObj();
	const auto boss_entity = owner_comp_sys->GetOwnerEntity();
	if (player_list.empty())return NodeStatus::FAILURE;
	const auto DT = bt_root_timer->GetFloatDT();
	if (m_hit_catapult)
	{
		m_accCatapultTime -= DT;
		if (0.f < m_accCatapultTime)
		{
			bt_root_timer->BroadcastObjInSight(bt_root_timer->GetTempVecForInsightObj(),
				Create_s2c_BOSS_MOVE(ToFlatVec(boss_entity->GetComp<PositionComponent>()->pos), 20.f, Nagox::Enum::BOSS_MOVE_TYPE_BOSS_MOVE_TYPE_1, boss_entity->GetComp<PositionComponent>()->body_angle));
			return NodeStatus::RUNNING;
		}
		else
		{
			m_now_meteor.store(false);
			count = 30;
			m_accTime = .5f;
			m_accTime2 = 3.5f;
			m_accCatapultTime = 5.f;
			m_hit_catapult.store(false);
			return NodeStatus::SUCCESS;
		}
	}
	m_accTime2 -= DT;
	if (0.f < m_accTime2)
	{
		return NodeStatus::RUNNING;
	}
	if (0 == count)
	{
		m_now_meteor.store(false);
		count = 30;
		m_accTime = .5f;
		m_accTime2 = 3.5f;
		m_accCatapultTime = 30.f;
		return NodeStatus::SUCCESS;
	}

	if (!m_now_meteor)
	{
		m_now_meteor.store(true);
		bt_root_timer->BroadcastObjInSight(bt_root_timer->GetTempVecForInsightObj(),
			Create_s2c_NOTIFY_CATAPULT(ToFlatVec(Vector3(-61.653553F, 30.55081F, -287.9226F))));
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
		proj.timer->m_radius = 0.125f;
		proj.timer->m_max_dist = 100.f;
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

NodeStatus FireBreath::Tick(const ComponentSystemNPC* const owner_comp_sys, TickTimerBT* const bt_root_timer, const NagiocpX::S_ptr<NagiocpX::ContentsEntity>& awaker) noexcept
{
	const auto boss_storage_node = static_cast<BossStorageNode* const>(bt_root_timer->GetRootNode());
	const auto& player_list = bt_root_timer->GetTempVecForInsightObj();
	// TODO: 적당한 유저 찾기	
	const auto boss_entity = owner_comp_sys->GetOwnerEntity();
	//if (player_list.empty())return NodeStatus::FAILURE;
	if (!boss_storage_node->m_cur_target)return NodeStatus::FAILURE;
	if(BOSS_PHASE::PHASE_1 == boss_storage_node->m_boss_phase)return NodeStatus::FAILURE;

	const auto DT = bt_root_timer->GetFloatDT();
	m_accTime -= DT;
	m_accReadyTime -= DT;
	if (0.f < m_accReadyTime)
	{
		return NodeStatus::RUNNING;
	}
	if (count == 0 && 0.f >= m_accTime)
	{
		count = 50;
		m_accTime = .01f;
		m_accReadyTime = 2.f;
		return NodeStatus::SUCCESS;
	}
	if (count == 0 || 0.f < m_accTime)
	{
		return NodeStatus::RUNNING;
	}
	const auto boss_pos = boss_entity->GetComp<PositionComponent>()->pos;
	//for (const auto users : player_list)
	{
		const Vector3 user_pos = boss_storage_node->m_cur_target->GetComp<PositionComponent>()->pos;
		const Vector3 to_target_dir = user_pos - boss_pos;
		Vector3 dir = to_target_dir;
		dir.Normalize();

		constexpr float spread_deg = 15.0f;
		const float random_offset_deg = ((rand() % 1000) / 1000.0f) * spread_deg * 2.0f - spread_deg;
		const float random_offset_rad = DirectX::XMConvertToRadians(random_offset_deg);

		const float cos_theta = cosf(random_offset_rad);
		const float sin_theta = sinf(random_offset_rad);

		Vector3 spread_dir;
		spread_dir.x = dir.x * cos_theta - dir.z * sin_theta;
		spread_dir.y = dir.y; // Y는 그대로 유지
		spread_dir.z = dir.x * sin_theta + dir.z * cos_theta;
		spread_dir.Normalize();

		const auto proj = NagiocpX::TimerHandler::CreateTimerWithoutHandle<MonProjectile>(10);
		proj.timer->m_proj_type = 2;
		proj.timer->m_radius = 0.125f;
		proj.timer->m_pos = boss_pos;
		proj.timer->m_speed = spread_dir * 50.f + Vector3{ 0,3,0 };
		proj.timer->m_max_dist = 100.f;
		proj.timer->SelectObjList(player_list);
		proj.timer->m_owner = owner_comp_sys->GetOwnerEntity()->SharedFromThis();
		

		const auto cur_dir = CommonMath::Normalized(boss_storage_node->m_cur_target->GetComp<PositionComponent>()->pos - boss_pos);
		boss_entity->GetComp<PositionComponent>()->body_angle = atan2f(cur_dir.x, cur_dir.z) * 180.f / 3.141592f;

		bt_root_timer->BroadcastObjInSight(bt_root_timer->GetTempVecForInsightObj(),
			Create_s2c_BOSS_MOVE(ToFlatVec(boss_entity->GetComp<PositionComponent>()->pos),
				20.f, Nagox::Enum::BOSS_MOVE_TYPE_BOSS_MOVE_TYPE_1,
				boss_entity->GetComp<PositionComponent>()->body_angle));
	}

	m_accTime = .01f;
	--count;
	if (count == 0)
	{
		bt_root_timer->BroadcastObjInSight(bt_root_timer->GetTempVecForInsightObj(),
			Create_s2c_BOSS_MOVE(ToFlatVec(boss_entity->GetComp<PositionComponent>()->pos), 20.f, Nagox::Enum::BOSS_MOVE_TYPE_BOSS_MOVE_TYPE_1, boss_entity->GetComp<PositionComponent>()->body_angle));
		m_accTime = .01f;
	}
	return NodeStatus::RUNNING;
}

NodeStatus PhaseCheckNode::Tick(const ComponentSystemNPC* const owner_comp_sys, TickTimerBT* const bt_root_timer, const NagiocpX::S_ptr<NagiocpX::ContentsEntity>& awaker) noexcept
{
	if (0.f >= m_accPhaseChangeTime && m_bIsChangedPhase)return NodeStatus::FAILURE;
	const auto boss_storage_node = static_cast<BossStorageNode* const>(bt_root_timer->GetRootNode());
	const auto hp_comp = owner_comp_sys->GetComp<HP>();
	const auto boss_max_hp = hp_comp->GetMaxHP();
	const auto boss_cur_hp = hp_comp->GetCurHP();
	if (boss_cur_hp <= boss_max_hp / 2)
	{
		const auto DT = bt_root_timer->GetFloatDT();
		m_accPhaseChangeTime -= DT;

		if (false == m_bIsChangedPhase)
		{
			m_bIsChangedPhase = true;
			owner_comp_sys->GetComp<ClusterInfoHelper>()->BroadcastAllCluster(Create_s2c_BOSS_CHANGE_PHASE());
		}
	
		if (0.f >= m_accPhaseChangeTime)
		{
			boss_storage_node->m_boss_phase.store(BOSS_PHASE::PHASE_2);
		}
		else
		{
			return NodeStatus::RUNNING;
		}
	}
	return NodeStatus::FAILURE;

}
