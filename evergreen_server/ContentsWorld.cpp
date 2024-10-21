#include "pch.h"
#include "ContentsWorld.h"
#include "Sector.h"
#include "TaskTimerMgr.h"
#include "Navigator.h"
#include "NavigationMesh.h"
#include "Timer.h"

void UPDATE()
{
	static ServerCore::Timer t;
	t.Update();
	NAVIGATION->GetNavMesh(NAVI_MESH_NUM::NUM_0)->GetCrowd()->update(t.GetDT(), 0);
	Mgr(TaskTimerMgr)->ReserveAsyncTask(200, []() {UPDATE(); });
}

void ContentsWorld::InitWorld() noexcept
{
	m_vecSectors.resize(1);
	m_vecSectors[0].emplace_back(ServerCore::MakeShared<ServerCore::Sector>(NUM_OF_GROUPS, 0, 0, this));
	
	UPDATE();
}
