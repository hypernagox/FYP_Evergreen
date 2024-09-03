#include "pch.h"
#include "ContentsWorld.h"
#include "Sector.h"

void ContentsWorld::InitWorld() noexcept
{
	m_vecSectors.resize(1);
	m_vecSectors[0].emplace_back(ServerCore::MakeShared<ServerCore::Sector>(NUM_OF_GROUPS, 0, 0, this));
}
