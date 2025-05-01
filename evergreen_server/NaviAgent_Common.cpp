#include "pch.h"
#include "NaviAgent_Common.h"
#include "NavigationMesh.h"
#include "PositionComponent.h"

//TODO: 나중에 바꿔야함
static auto InitParam()
{
	dtCrowdAgentParams params;
	memset(&params, 0, sizeof(params));
	params.radius = 1.6f;
	params.height = 2.0f;
	params.maxAcceleration = 8.0f;
	params.maxSpeed = 8.5f;
	params.collisionQueryRange = params.radius * 36.0f;
	params.pathOptimizationRange = params.radius * 30.0f;
	params.separationWeight = 3.0f;
	params.updateFlags = DT_CROWD_SEPARATION;
	return params;
}

void NaviAgent::Init(const Vector3& pos, Common::NavigationMesh* const pNavMesh) noexcept
{
	auto& cur_pos = m_posComp->pos;
	cur_pos = pos;
	m_agent.SetNavMesh(pNavMesh);
	SetPos(pos);
}

void NaviAgent::InitRandPos(Common::NavigationMesh* const pNavMesh) noexcept
{
	m_agent.SetNavMesh(pNavMesh);
	pNavMesh->GetRandomPos(m_posComp->pos, m_agent.GetCurCell());

	//dtCrowdAgentParams params = InitParam();

	auto pos = m_posComp->pos;
	CommonMath::InverseZ(pos);
	//m_my_idx = pNavMesh->GetCrowd()->addAgent(&pos.x, &params);
}

void NaviAgent::SetPos(const Vector3& pos) noexcept
{
	auto& cur_pos = m_posComp->pos;
	cur_pos = pos;
	m_agent.GetCurCell() = m_agent.GetNavMesh()->GetNaviCell(cur_pos);
}

void NaviAgent::InitParams() noexcept
{
	const auto param = InitParam();
	m_agent.GetNavMesh()->GetCrowd()->updateAgentParameters(m_my_idx, &param);
}

void NaviAgent::SetCellPos(const float dt, const Vector3& prev_pos, const Vector3& post_pos) noexcept
{
	m_agent.SetCellPos(dt, prev_pos, post_pos, m_posComp->pos);
}

float NaviAgent::ApplyPostPosition(const Vector3& dir, const float speed, const float dt) noexcept
{
	const auto dir_ = CommonMath::Normalized(dir) * speed * dt;
	const auto& prev_pos = m_posComp->pos;
	const auto post_pos = prev_pos + dir_;
	SetCellPos(dt, prev_pos, post_pos);
	return dir_.Length();
}

void NaviAgent::ProcessCleanUp() noexcept
{
}
