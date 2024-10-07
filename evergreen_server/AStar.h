#pragma once
#include "pch.h"
#include <fstream>
#include "NaviMesh.h"

class NaviCell;

class AStar
{
public:
	struct NaviNode
	{
		int idx;
		float F;
		
		auto operator <(const NaviNode& other)const noexcept { return F > other.F; }

	};
	static ServerCore::Vector<DirectX::SimpleMath::Vector3> OptimizePath(const ServerCore::Vector<DirectX::SimpleMath::Vector3>& path, const NaviMesh* const pNaviMesh, const DirectX::SimpleMath::Vector3& dpos) noexcept;
	//static bool HasLineOfSight(const NaviCell* const a, const NaviCell* const b, const NaviMesh* const pNaviMesh) noexcept;
	static float Heuristic(const NaviCell* const a, const NaviCell* const b)noexcept;
	static ServerCore::Vector<DirectX::SimpleMath::Vector3> GetAStarPath(const NaviMesh* const pNaviMesh, const int start, const int dest,const DirectX::SimpleMath::Vector3& dpos)noexcept;
private:

};
