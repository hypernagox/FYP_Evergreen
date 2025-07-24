#include "pch.h"
#include "ForcedMovement.h"
#include "ServerObject.h"
#include "EntityMovement.h"
#include "MoveInterpolator.h"
#include "NavigationMesh.h"
#include "NaviAgent.h"
#include "MovePacketSender.h"

void ForcedMovement::Update()noexcept
{
	if (!m_bIsForcedMovement)return;
	const auto owner = m_owner->GetSceneObject();
	acc1 = std::max(acc1 - DT, 0.f);
	
	if (speed_flag)
	{
		const auto ms = m_owner->GetComp<MovePacketSender>();
		if (ms)
		{
			ms->SetSendInterval(0.f);
		}
		const auto navi = owner->GetComponent<ServerObject>()->GetNaviAgent();
		const auto transform = owner->GetTransform();
		const auto prev_pos = transform->GetLocalPosition();
		const auto post_pos = prev_pos + m_dir * DT * 10.f;
		Vector3 temp = prev_pos;
		navi->SetCellPos(DT, prev_pos, post_pos, temp);
		transform->SetLocalPosition(temp);
		owner->GetComponent<EntityMovement>()->prev_pos = temp;
		acc2 -= DT;
		if (acc2 <= 0.f)
		{
			speed_flag = false;
			m_bIsForcedMovement = false;
			acc1 = 5.f;
			acc2 = .1f;
			if (ms)
			{
				ms->SetSendInterval(0.1f);
			}
		}
	}
}

bool ForcedMovement::CheckDash() noexcept
{
	if (!speed_flag && INSTANCE(Input)->GetKeyDown(Keyboard::Space))
	{

		acc2 = .1f;
		const auto owner = m_owner->GetSceneObject();
		const auto prev_pos = m_owner->GetTransform()->GetLocalPosition();
		const auto vel = CommonMath::Normalized(owner->GetComponent<EntityMovement>()->GetVelocity());
		m_dest = prev_pos + vel * 5.f;
		auto temp = m_dest;
		m_owner->GetNaviAgent()->ForcedMovement(
			prev_pos,
			temp,
			m_dest
		);
		m_dir = CommonMath::Normalized(m_dest - prev_pos);
		const auto c = m_dir.Dot(vel);
		if (0.f >= c)
			return false;
		m_bIsForcedMovement = true;
		speed_flag = true;
		Send(Create_c2s_DASH(ToFlatVec3(m_dest)));
		return true;
	}
	return false;
}

void ForcedMovement::SetForcedMovement(const Vector3& dest_pos) noexcept
{
	acc2 = .1f;
	const auto owner = m_owner->GetSceneObject();
	const auto prev_pos = m_owner->GetTransform()->GetLocalPosition();
	const auto vel = CommonMath::Normalized(dest_pos - prev_pos);
	m_dest = prev_pos + vel * 2.f;
	auto temp = m_dest;
	m_owner->GetNaviAgent()->ForcedMovement(
		prev_pos,
		temp,
		m_dest
	);
	m_dir = CommonMath::Normalized(m_dest - prev_pos);
	//const auto c = m_dir.Dot(vel);
	//if (0.f >= c)return;
	m_bIsForcedMovement = true;
	speed_flag = true;
}
