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
	int index = 0; 
	std::lock_guard<std::mutex> lk{ m_mt };
	for (const auto& entry : std::filesystem::directory_iterator(RESOURCE_PATH(L"navmesh")))
	{
		if (entry.is_regular_file() && entry.path().extension() == L".bin")
		{
			if (index >= (int)NAVI_MESH_TYPE::END)
			{
				std::wcout << L"[WARN] 최대 개수 초과\n";
				break;
			}
			m_arrNavMesh[index] = new Common::NavigationMesh;
			m_arrNavMesh[index++]->Init(entry.path().wstring());
		}
	}
}
