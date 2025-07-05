#pragma once
#include "../NagiocpX/Singleton.hpp"
#include "NavigationMesh.h"

enum class NAVI_MESH_TYPE : uint8_t
{
	MAIN_WORLD,
	BOSS_ROOM,

	END,
};

class Common::NavigationMesh;

class Navigator
	:public NagiocpX::Singleton<Navigator>
{
	friend class Singleton;
	Navigator();
	~Navigator();
public:
	// 네비메시들 초기화 방법은 더 생각.
	void Init()noexcept;
public:
	Common::NavigationMesh* GetNavMesh(const NAVI_MESH_TYPE eType)const noexcept { return m_arrNavMesh[(int)eType]; }

private:
	Common::NavigationMesh* m_arrNavMesh[(int)NAVI_MESH_TYPE::END] = {};
};

#define NAVIGATION (Navigator::GetInst())
