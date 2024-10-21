#include "pch.h"
#include "ServerSession.h"
#include "s2c_DummyPacketHandler.h"
#include "Service.h"
#include "CreateBuffer4Dummy.h"

std::shared_ptr<HeightMap> g_heightMap;

extern Vector3 SetTerrainPos(const Vector3& v);

int main()
{
	g_heightMap = std::make_shared<HeightMap>(RESOURCE_PATH(L"terrain_height.raw"), 4096, 4096);
	Mgr(CoreGlobal)->Init();
	s2c_DummyPacketHandler::Init();

	const auto pClientService = std::make_unique<ServerCore::ClientService>
		(
			  Mgr(CoreGlobal)->GetIocpCore()
			, ServerCore::NetAddress{ L"127.0.0.1",7777 }
			, ServerCore::MakeSharedAligned<ServerSession>
			, 999
		);

	ASSERT_CRASH(pClientService->Start());

	Mgr(ThreadMgr)->Launch(
		  2
		, []() noexcept {const volatile auto init_builder = GetBuilder(); }
	);


	int n;
	std::cin >> n;

	Mgr(ThreadMgr)->Join();
}


Vector3 SetTerrainPos(const Vector3& v)
{
	Vector3 terrainPos = v * 0.01f;
	terrainPos.x = fmod(terrainPos.x + 1.0f, 1.0f);
	terrainPos.z = fmod(terrainPos.z + 1.0f, 1.0f);
	float terrainHeight = g_heightMap->GetHeight(terrainPos.x * 4096, terrainPos.z * 4096);
	terrainPos.y = std::max(terrainPos.y, terrainHeight);
	return terrainPos * 100.0f;
}
