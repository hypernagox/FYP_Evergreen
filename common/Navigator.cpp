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
	for (int i = 0; i < NAVI_MESH_NUM::NUM_END; ++i)
	{
		delete m_arrNavMesh[i];
	}
}

void Navigator::Init() noexcept
{
	for (int i = 0; i < NAVI_MESH_NUM::NUM_END; ++i)
	{
		m_arrNavMesh[i] = new Common::NavigationMesh;
		m_arrNavMesh[i]->Init(RESOURCE_PATH(L"navmesh.bin"));
	}
}
