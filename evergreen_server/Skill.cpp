#include "pch.h"
#include "Skill.h"
#include "MoveBroadcaster.h"
#include "PositionComponent.h"
#include "Collider_Common.h"
#include "HP.h"
#include "TimerRoutine.h"
#include "Projectile.h"
#include "StatusSystem.h"
#include "Service.h"
#include "ClusterPredicate.h"
#include "NaviAgent_Common.h"
#include "Queueabler.h"
#include "Cluster.h"
#include "ClusterInfoHelper.h"

using namespace NagiocpX;

bool WarriorDefaultAttack::ExecuteSkill(class StatusSystem* const use_entity_system) noexcept
{
	//DO_BENCH_GLOBAL_THIS_FUNC;

	// TODO: 월드가 달라졌다면 뷰리스트의 갱신이 필요
	// TODO: 생포인터로 개기지 말자
	// 정수값 아이디만 쓰거나 쉐어드를 쓰자
	const auto pOwner = use_entity_system->GetOwnerEntityRaw();

	const auto pos_comp = pOwner->GetComp<PositionComponent>();
	
	constexpr Vector3 forward(0.0f, 0.0f, 1.0f);

	const DirectX::SimpleMath::Matrix rotationMatrix = DirectX::SimpleMath::Matrix::CreateRotationY(pos_comp->body_angle);

	//std::cout << "MY angle: " << pkt_.body_angle() << '\n';
	//std::cout << "Mypos: ";
	//PrintLogEndl(&pos_comp->pos.x);
	Vector3 rotatedForward = Vector3::Transform(forward, rotationMatrix);
	auto c = pOwner->GetComp<AABBCollider>()->GetCollider<Common::AABBBox>();
	//c->m_offSet = rotatedForward;
	//auto box = c->GetAABB();
	bool isHit = false;
	//pos_comp->pos = ::ToDxVec(pkt_.atk_pos());
	rotatedForward.y = 0.f;
	auto ppp = pos_comp->pos;
	ppp.y += 2.f;
	Common::Sphere sp{ &ppp ,1.f };
	//std::cout << "Player Pos: ";
	//PrintVector3(pos_comp->pos);
	//Common::Fan fan{ pos_comp->pos ,rotatedForward,30.f,4.f };

	//fan.m_offSet = rotatedForward * 2;

	//if (const auto sector = pOwner->GetCurCluster())

	{
		const auto& mon_list = pOwner->GetComp<MoveBroadcaster>()->GetViewListNPC();
		for (const auto pmon : mon_list)
		{
			//if (const auto pmon = Mgr(FieldMgr)->GetNPC(mon_id))
			{
				if (const auto pCol = pmon->GetComp<Collider>())
				{
					const auto owner = pCol->GetOwnerEntity();

					if (sp.IsCollision(pCol->GetCollider()))
						//if (pCol->IsCollision(box))
					{
						//NAVIGATION->GetNavMesh(NAVI_MESH_NUM::NUM_0)->GetCrowd()->getEditableAgent(owner->GetComp<NaviAgent>()->m_my_idx)->active = false;
						//
						//owner->TryOnDestroy();
						//std::cout << "Hit Pos: ";
						//PrintVector3(pCol->GetPosComp()->pos);
						owner->GetComp<HP>()->PostDoDmg(1, pOwner->SharedFromThis());
						isHit = true;
					}
					else
					{
						//const auto ppp = pCol->GetCollider()->GetPosWithOffset();
						//PrintLogEndl(&ppp.x);
					}
				}

			}
		}
	}
	{
		pOwner->GetComp<MoveBroadcaster>()->BroadcastPacket(Create_s2c_PLAYER_ATTACK(pOwner->GetObjectID64(), pos_comp->body_angle, ToFlatVec(pos_comp->pos),Nagox::Enum::SKILL_TYPE_DEFAULT));
	}
	//std::cout << "\n\n\n";
	//std::ranges::sort(vvv, [player_pos = pos_comp->pos](const Vector3& a, const Vector3& b) {
	//	const auto aa = CommonMath::GetDistPowDX(a, player_pos);
	//	const auto bb = CommonMath::GetDistPowDX(b, player_pos);
	//	return aa < bb;
	//	});
	//for (const auto& v : vvv)PrintVector3(v);
	if (isHit)
	{
		//std::cout << "Hit!\n";
	}
	//std::cout << "\n\n\n";
	return true;
}

bool PriestDefaultAttack::ExecuteSkill(class StatusSystem* const use_entity_system)noexcept
{
	const auto pOwner = use_entity_system->GetOwnerEntityRaw();

	const auto pos_comp = pOwner->GetComp<PositionComponent>();
	constexpr Vector3 forward(0.0f, 0.0f, 1.0f);

	const DirectX::SimpleMath::Matrix rotationMatrix = DirectX::SimpleMath::Matrix::CreateRotationY(pos_comp->body_angle);

	const Vector3 rotatedForward = Vector3::Transform(forward, rotationMatrix);


	const auto proj = TimerHandler::CreateTimerWithoutHandle<PlayerProjectile>(100);
	proj.timer->m_pos = (pos_comp->pos);
	proj.timer->m_speed = rotatedForward * 40.f;
	proj.timer->SelectObjList(pOwner->GetComp<MoveBroadcaster>()->GetViewListNPC());
	proj.timer->m_owner = pOwner->SharedFromThis();

	pOwner->GetComp<MoveBroadcaster>()->BroadcastPacket(Create_s2c_PLAYER_ATTACK(pOwner->GetObjectID64(), pos_comp->body_angle, ToFlatVec(pos_comp->pos), Nagox::Enum::SKILL_TYPE_DEFAULT));

	return true;
}

bool WarriorSkill_1::ExecuteSkill(StatusSystem* const use_entity_system) noexcept
{
	// TODO: 수정가능성있음
	
	//DO_BENCH_GLOBAL_THIS_FUNC;

	// TODO: 월드가 달라졌다면 뷰리스트의 갱신이 필요
	// TODO: 생포인터로 개기지 말자
	// 정수값 아이디만 쓰거나 쉐어드를 쓰자
	const auto pOwner = use_entity_system->GetOwnerEntityRaw();

	const auto pos_comp = pOwner->GetComp<PositionComponent>();

	constexpr Vector3 forward(0.0f, 0.0f, 1.0f);

	const DirectX::SimpleMath::Matrix rotationMatrix = DirectX::SimpleMath::Matrix::CreateRotationY(pos_comp->body_angle);

	//std::cout << "MY angle: " << pkt_.body_angle() << '\n';
	//std::cout << "Mypos: ";
	//PrintLogEndl(&pos_comp->pos.x);
	Vector3 rotatedForward = Vector3::Transform(forward, rotationMatrix);
	auto c = pOwner->GetComp<AABBCollider>()->GetCollider<Common::AABBBox>();
	//c->m_offSet = rotatedForward;
	//auto box = c->GetAABB();
	bool isHit = false;
	//pos_comp->pos = ::ToDxVec(pkt_.atk_pos());
	rotatedForward.y = 0.f;
	auto ppp = pos_comp->pos;
	ppp.y += 2.f;
	Common::Sphere sp{ &ppp ,2.5f };
	//std::cout << "Player Pos: ";
	//PrintVector3(pos_comp->pos);
	//Common::Fan fan{ pos_comp->pos ,rotatedForward,30.f,4.f };

	//fan.m_offSet = rotatedForward * 2;

	//if (const auto sector = pOwner->GetCurCluster())

	{
		const auto& mon_list = pOwner->GetComp<MoveBroadcaster>()->GetViewListNPC();
		for (const auto pmon : mon_list)
		{
			//if (const auto pmon = Mgr(FieldMgr)->GetNPC(mon_id))
			{
				if (const auto pCol = pmon->GetComp<Collider>())
				{
					const auto owner = pCol->GetOwnerEntityRaw();

					if (sp.IsCollision(pCol->GetCollider()))
						//if (pCol->IsCollision(box))
					{
						//NAVIGATION->GetNavMesh(NAVI_MESH_NUM::NUM_0)->GetCrowd()->getEditableAgent(owner->GetComp<NaviAgent>()->m_my_idx)->active = false;
						//
						//owner->TryOnDestroy();
						//std::cout << "Hit Pos: ";
						//PrintVector3(pCol->GetPosComp()->pos);
						if (const auto navi_agent = owner->GetComp<NaviAgent>())
						{
							const auto& atk_pos = ppp;
							const auto& victim_pos = owner->GetComp<PositionComponent>()->pos;
							const auto dir = CommonMath::Normalized(victim_pos - atk_pos);
							ClusterPredicate c;
							if (Nagox::Enum::MONSTER_TYPE_BOSS != owner->GetDetailType())
							{
								owner->GetQueueabler()->EnqueueAsync(&NaviAgent::ForcedMovement, navi_agent, dir, 10.f);
								owner->GetComp<ClusterInfoHelper>()->BroadcastCluster(c.ClusterPredicate::CreateMovePacket(owner));
							}
						}
						owner->GetComp<HP>()->PostDoDmg(1, pOwner->SharedFromThis(), 5);
						isHit = true;
					}
					else
					{
						//const auto ppp = pCol->GetCollider()->GetPosWithOffset();
						//PrintLogEndl(&ppp.x);
					}
				}

			}
		}
	}
	{
		pOwner->GetComp<MoveBroadcaster>()->BroadcastPacket(Create_s2c_PLAYER_ATTACK(pOwner->GetObjectID64(), pos_comp->body_angle, ToFlatVec(pos_comp->pos), Nagox::Enum::SKILL_TYPE_SKILL_1));
	}
	//std::cout << "\n\n\n";
	//std::ranges::sort(vvv, [player_pos = pos_comp->pos](const Vector3& a, const Vector3& b) {
	//	const auto aa = CommonMath::GetDistPowDX(a, player_pos);
	//	const auto bb = CommonMath::GetDistPowDX(b, player_pos);
	//	return aa < bb;
	//	});
	//for (const auto& v : vvv)PrintVector3(v);
	if (isHit)
	{
		//std::cout << "Hit!\n";
	}
	//std::cout << "\n\n\n";
	return true;
}

bool PriestSkill_1::ExecuteSkill(StatusSystem* const use_entity_system) noexcept
{
	const auto pOwner = use_entity_system->GetOwnerEntityRaw();

	const auto pos_comp = pOwner->GetComp<PositionComponent>();
	const auto owner_pos = pos_comp->pos;
	XVector<S_ptr<ContentsEntity>> users;
	XVector<uint32_t> healed_user_ids;
	{
		const auto& user_list = pOwner->GetComp<MoveBroadcaster>()->GetViewListSession();
		users.reserve(user_list.size());
		healed_user_ids.reserve(user_list.size());
		for (const auto [id, player] : user_list)
		{
			auto user = GetSessionEntity(id);
			if (!user)continue;
			const auto other_pos = user->GetComp<PositionComponent>()->pos;
			if (CommonMath::IsInDistanceDX(other_pos, owner_pos, 5.f))
			{
				user->GetComp<HP>()->PostDoHeal(300);
				healed_user_ids.emplace_back(user->GetObjectID());
			}
			users.emplace_back(std::move(user));
		}
		for (const auto healed_user_id : healed_user_ids)
		{
			for (const auto& user : users)
			{
				auto heal_pkt = Create_s2c_HEAL(
					healed_user_id,
					300
				);
				user->GetSession()->SendAsync(heal_pkt);
				pOwner->GetSession()->SendAsync(std::move(heal_pkt));
			}
		}
	}
	{
		pOwner->GetComp<MoveBroadcaster>()->BroadcastPacket(Create_s2c_PLAYER_ATTACK(pOwner->GetObjectID64(), pos_comp->body_angle, ToFlatVec(pos_comp->pos), Nagox::Enum::SKILL_TYPE_SKILL_1));
	}
	return true;
}

bool ArcherDefaultAttack::ExecuteSkill(StatusSystem* const use_entity_system) noexcept
{
	return true;
}

bool ArcherSkill_1::ExecuteSkill(StatusSystem* const use_entity_system) noexcept
{
	const auto pOwner = use_entity_system->GetOwnerEntityRaw();
	const auto pos_comp = pOwner->GetComp<PositionComponent>();
	constexpr Vector3 forward(0.0f, 0.0f, 1.0f);
	const DirectX::SimpleMath::Matrix rotationMatrix = DirectX::SimpleMath::Matrix::CreateRotationY(pos_comp->body_angle);
	const Vector3 rotatedForward = Vector3::Transform(forward, rotationMatrix);
	const auto broad_caster = pOwner->GetComp<MoveBroadcaster>();
	const auto& view_list = broad_caster->GetViewListNPC();

	const ContentsEntity* closestTarget = nullptr;
	float minDistSq = std::numeric_limits<float>::max();

	for (const auto npc : view_list)
	{
		const auto npcPosComp = npc->GetComp<PositionComponent>();
		const auto npc_hp = npc->GetComp<HP>();

		if (!npcPosComp || !npc_hp || npc->GetPrimaryGroupType() != Nagox::Enum::GROUP_TYPE_MONSTER)
			continue;

		const Vector3 toTarget = npcPosComp->pos - pos_comp->pos;
		const float distSq = toTarget.LengthSquared();
		if (distSq > 12.f * 12.f)
			continue;

		const float dot = rotatedForward.Dot(CommonMath::Normalized(toTarget));
		//if (dot < 0.5f) 
		//	continue;

		if (distSq < minDistSq)
		{
			minDistSq = distSq;
			closestTarget = npc;
		}
	}

	if (closestTarget)
	{
		const auto player = pOwner->SharedFromThis();
		const auto centerPos = closestTarget->GetComp<PositionComponent>()->pos;

		for (const auto npc : view_list)
		{
			const auto npcPosComp = npc->GetComp<PositionComponent>();
			const auto npc_hp = npc->GetComp<HP>();
			if (!npcPosComp || !npc_hp)
				continue;
			
			if (CommonMath::IsInDistanceDX(npcPosComp->pos, centerPos, 10.f))
			{
				npc_hp->PostDoDmg(1, player, 5);
			}
			
		}
		auto pkt1 = Create_s2c_PLAYER_ATTACK(
			pOwner->GetObjectID64(),
			pos_comp->body_angle,
			ToFlatVec(pos_comp->pos),
			Nagox::Enum::SKILL_TYPE_SKILL_1
		);
		auto pkt2 = Create_s2c_ARROW_RAIN(
			pOwner->GetObjectID64(),
			pos_comp->body_angle,
			ToFlatVec(centerPos),
			Nagox::Enum::SKILL_TYPE_SKILL_1
		);
		const auto owner_session = pOwner->GetSession();
		owner_session->SendAsync(pkt1);
		owner_session->SendAsync(pkt2);
		const auto g_main_server = NagiocpX::ServerService::GetMainService();
		const auto& session_list = broad_caster->GetViewListSession();
		auto b = session_list.data();
		const auto e = b + session_list.size();
		while (e != b) {
			if (const auto entity = g_main_server->GetSession((*b++).first))
			{
				const auto session = entity->GetSession();
				session->SendAsync(pkt1);
				session->SendAsync(pkt2);
			}
		}
	}

	

	return true;
}
