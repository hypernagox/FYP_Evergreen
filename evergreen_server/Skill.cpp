#include "pch.h"
#include "Skill.h"
#include "MoveBroadcaster.h"
#include "PositionComponent.h"
#include "Collider_Common.h"
#include "HP.h"
#include "TimerRoutine.h"
#include "Projectile.h"
#include "StatusSystem.h"

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