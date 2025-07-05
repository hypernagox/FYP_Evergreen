#include "pch.h"
#include "Navigator.h"
#include "PathManager.h"
#include "NaviCell.h"
#include "NavigationMesh.h"

Navigator::Navigator()
{
}

Navigator::~Navigator()
{
	for (int i = 0; i < (int)NAVI_MESH_TYPE::END; ++i)
	{
		delete m_arrNavMesh[i];
	}
}

void Navigator::Init() noexcept
{
	static constinit std::mutex g_mt = {};
	// 혹시라도 컴퓨터/OS에 따라서 디렉토리 읽는 순서에 따라 인덱싱 오류생길까봐 하드코딩함
	std::lock_guard<std::mutex> lk{ g_mt };
	{
		m_arrNavMesh[(int)NAVI_MESH_TYPE::MAIN_WORLD] = new Common::NavigationMesh;
		m_arrNavMesh[(int)NAVI_MESH_TYPE::MAIN_WORLD]->Init(RESOURCE_PATH(L"\\navmesh\\all_tiles_navmesh2try.bin"));
	}
	{
		m_arrNavMesh[(int)NAVI_MESH_TYPE::BOSS_ROOM] = new Common::NavigationMesh;
		m_arrNavMesh[(int)NAVI_MESH_TYPE::BOSS_ROOM]->Init(RESOURCE_PATH(L"\\navmesh\\boss_navmesh.bin"));
	}
}
