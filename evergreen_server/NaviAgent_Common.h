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
	void SetPos(const Vector3& pos)noexcept;
	void InitParams()noexcept;
public:
	void SetCellPos(const Vector3& prev_pos, const Vector3& post_pos)noexcept;
	const auto GetAgentConcreate()noexcept { return &m_agent; }
public:
	float ApplyPostPosition(const Vector3& dir, const float speed, const float dt)noexcept;
	int m_my_idx = -1;
public:
	virtual void ProcessCleanUp()noexcept override;
private:
	Common::NaviAgent m_agent;
	PositionComponent* m_posComp;
};