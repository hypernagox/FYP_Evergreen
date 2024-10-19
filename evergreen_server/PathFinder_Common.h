#pragma once
#include "PathFinder.h"

class PositionComponent;

class Common::NaviAgent;

class PathFinder
	:public ContentsComponent
{
public:
	CONSTRUCTOR_CONTENTS_COMPONENT(PathFinder)
public:
	std::span<Vector3> GetPath(const Vector3& start, const Vector3& dest)const noexcept;
public:
	void SetAgent(Common::NaviAgent* const agent)noexcept { m_pathFinder.SetNaviAgent(agent); }
private:
	Common::PathFinder m_pathFinder;
};

