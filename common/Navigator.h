#pragma once
#include "../ServerCore/Singleton.hpp"
#include "NavigationMesh.h"

enum NAVI_MESH_NUM : uint8_t
{
	NUM_0,


	NUM_END,
};

class Common::NavigationMesh;

class Navigator
	:public ServerCore::Singleton<Navigator>
{
	friend class Singleton;
	Navigator();
	~Navigator();
public:
	// �׺�޽õ� �ʱ�ȭ ����� �� ����.
	void Init()noexcept;
public:
	Common::NavigationMesh* GetNavMesh(const NAVI_MESH_NUM eType)const noexcept { return m_arrNavMesh[eType]; }

private:
	Common::NavigationMesh* m_arrNavMesh[NAVI_MESH_NUM::NUM_END] = {};
};

#define NAVIGATION (Navigator::GetInst())
