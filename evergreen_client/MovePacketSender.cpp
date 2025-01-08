#include "pch.h"
#include "MovePacketSender.h"
#include "EntityMovement.h"
#include "ServerObject.h"
#include "AuthenticPlayer.h"
#include <CreateBuffer4Client.h>
#include "ServerTimeMgr.h"

extern Vector3 g_curPos;

void MovePacketSender::Update() noexcept
{
	const auto owner_hero = m_owner->GetSceneObject()->GetComponent<AuthenticPlayer>();
	const auto owner_movement = m_owner->GetSceneObject()->GetComponent<EntityMovement>();
	m_accTime += DT;
	 bool& flag = owner_hero->GetSendFlag();
	 if (flag || m_sendInterval <= m_accTime)
	{
		 NetMgr(ServerTimeMgr)->SetTimeStampByName("MOVE_PKT");
		const auto root_obj = owner_hero->GetSceneObject();
		
		const auto pos = root_obj->GetTransform()->GetLocalPosition();
		const auto vel = owner_movement->GetVelocity();
		const auto accel = owner_movement->GetAcceleration();
		const auto bodyY = owner_hero->GetYAngle();
		const auto time_stamp = NetHelper::GetServerTimeStamp();
		Send(
			Create_c2s_MOVE(
				  ToFlatVec3(pos)
				, ToFlatVec3(vel)
				, ToFlatVec3(accel)
				, bodyY
				, time_stamp)
		);
		m_accTime = 0.f;
	}
	 flag = false;
}
