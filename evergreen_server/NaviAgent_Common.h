#pragma once
#include "NaviAgent.h"

class PositionComponent;

class NavigationMesh;


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
public:
	void SetCellPos(const Vector3& prev_pos, const Vector3& post_pos)noexcept;
	const auto GetAgentConcreate()noexcept { return &m_agent; }
private:
	Common::NaviAgent m_agent;
	PositionComponent* m_posComp;
};