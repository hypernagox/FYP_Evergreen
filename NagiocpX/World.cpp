#include "ServerCorePch.h"
#include "World.h"

namespace ServerCore
{
	World::~World()
	{
		std::cout << "���� �Ҹ�" << std::endl;
	}

	void World::EndWorld() noexcept
	{
		for (const auto& sectors : m_vecSectors | std::views::join)
			sectors->TryDestroy();

		TryDestroy();
	}
	
}
