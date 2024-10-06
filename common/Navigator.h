#pragma once
#include "../ServerCore/Singleton.hpp"

enum NAVI_MESH_NUM : uint8_t
{
	NUM_0,


	NUM_END,
};

class NaviMesh;

class Navigator
	:public ServerCore::Singleton<Navigator>
{
	friend class Singleton;
	Navigator();
	~Navigator();
public:
	// 네비메시들 초기화 방법은 더 생각.
	void Init()noexcept;
public:
	NaviMesh* GetNavMesh(const NAVI_MESH_NUM eType)const noexcept { return m_arrNavMesh[eType]; }
private:
	NaviMesh* m_arrNavMesh[NAVI_MESH_NUM::NUM_END] = {};
};

#define NAVIGATION (Navigator::GetInst())
