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
	for (const auto n : m_arrNavMesh)delete n;
}

void Navigator::Init() noexcept
{
	for (int i = 0; i < NAVI_MESH_NUM::NUM_END; ++i)m_arrNavMesh[i] = new NaviMesh;
	//m_arrNavMesh[0]->Load(RESOURCE_PATH(L"NAVIMESH.bin"));
}
