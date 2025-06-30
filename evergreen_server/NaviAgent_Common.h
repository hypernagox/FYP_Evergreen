#pragma once
#include "NaviAgent.h"

class PositionComponent;

class Common::NavigationMesh;


class NaviAgent
	:public ContentsComponent
{
public:
	CONSTRUCTOR_CONTENTS_COMPONENT(NaviAgent)
public:
	void Init(const Vector3& pos, Common::NavigationMesh* const pNavMesh)noexcept;
	void InitRandPos(Common::NavigationMesh* const pNavMesh)noexcept;
	void SetPosComp(PositionComponent* const posComp)noexcept { m_posComp = posComp; }
	const auto GetPosComp()noexcept { return m_posComp; }
	void SetPos(const Vector3& pos)noexcept;
	Common::NavigationMesh* const GetNavMesh()noexcept;
public:
	void SetCellPos(const float dt, const Vector3& prev_pos, const Vector3& post_pos)noexcept;
	const auto GetAgentConcreate()noexcept { return &m_agent; }
public:
	float ApplyPostPosition(const Vector3& dir, const float speed, const float dt)noexcept;
	void ForcedMovement(const Vector3 dir, const float movement_dist)noexcept;
public:
	virtual void ProcessCleanUp()noexcept override;
private:
	Common::NaviAgent m_agent;
	PositionComponent* m_posComp;
};