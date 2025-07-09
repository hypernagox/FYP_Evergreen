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
	DestroyTLS();
	for (int i = 0; i < (int)NAVI_MESH_TYPE::END; ++i)
	{
		dtFreeNavMesh(const_cast<dtNavMesh*>(m_dt_nav_mesh[i]));
	}
}

void Navigator::Init()noexcept
{
	// 혹시라도 컴퓨터/OS에 따라서 디렉토리 읽는 순서에 따라 인덱싱 오류생길까봐 하드코딩함
	{
		m_dt_nav_mesh[(int)NAVI_MESH_TYPE::MAIN_WORLD] = Common::NavigationMesh::LoadNavMesh(RESOURCE_PATH(L"\\navmesh\\main_navmesh.bin"));
	}
	{
		m_dt_nav_mesh[(int)NAVI_MESH_TYPE::BOSS_ROOM] = Common::NavigationMesh::LoadNavMesh(RESOURCE_PATH(L"\\navmesh\\boss_navmesh.bin"));
	}
	
}

void Navigator::InitTLS() noexcept
{
	static std::mutex g_mt = {};
	std::lock_guard<std::mutex> lk{ g_mt };
	for (int i = 0; i < (int)NAVI_MESH_TYPE::END; ++i)
	{
		m_arrNavMesh[i] = new Common::NavigationMesh;
		m_arrNavMesh[i]->Init(m_dt_nav_mesh[i]);
	}
}

void Navigator::DestroyTLS() noexcept
{
	for (int i = 0; i < (int)NAVI_MESH_TYPE::END; ++i)
	{
		if (!m_arrNavMesh[i])continue;
		delete m_arrNavMesh[i];
		m_arrNavMesh[i] = nullptr;
	}
}
