#include "pch.h"
#include "Navigator.h"
#include "PathManager.h"
#include "NaviMesh.h"
#include "NaviCell.h"

Navigator::Navigator()
{

}

Navigator::~Navigator()
{
	for (int i = 0; i < NAVI_MESH_NUM::NUM_END; ++i)
	{
		m_arrNavMesh[i]->Save(RESOURCE_PATH(std::format(L"NAVI_MESH_{}.bin", i)));
		delete m_arrNavMesh[i];
	}
}

void Navigator::Init() noexcept
{
	for (int i = 0; i < NAVI_MESH_NUM::NUM_END; ++i)
	{
		m_arrNavMesh[i] = new NaviMesh;
		m_arrNavMesh[i]->Load(RESOURCE_PATH(std::format(L"NAVI_MESH_{}.bin", i)));
	}
}
