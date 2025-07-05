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
	void Init()noexcept;
public:
	Common::NavigationMesh* GetNavMesh(const NAVI_MESH_TYPE eType)const noexcept { return m_arrNavMesh[(int)eType]; }

private:
	static inline thread_local constinit Common::NavigationMesh* m_arrNavMesh[(int)NAVI_MESH_TYPE::END] = {};
};

#define NAVIGATION (Navigator::GetInst())
