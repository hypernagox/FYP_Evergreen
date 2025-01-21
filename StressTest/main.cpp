#include "pch.h"
#include "ServerSession.h"
#include "s2c_DummyPacketHandler.h"
#include "Service.h"
#include "CreateBuffer4Dummy.h"
#include <regex>

std::shared_ptr<HeightMap> g_heightMap;
bool isValidIPAddress(std::wstring_view ipAddress) {
	const std::wregex ipRegex(L"^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");
	return std::regex_match(ipAddress.data(), ipRegex);
}
extern Vector3 SetTerrainPos(const Vector3& v);

int main()
{
	g_heightMap = std::make_shared<HeightMap>(RESOURCE_PATH(L"terrain_height.raw"), 4096, 4096);
	Mgr(CoreGlobal)->Init();
	s2c_DummyPacketHandler::Init();

	const auto pClientService = new ServerCore::ClientService
		(
			  Mgr(CoreGlobal)->GetIocpCore()
			, ServerCore::NetAddress{ L"127.0.0.1",7777 }
			, ServerCore::xnew<ServerSession>
			, s2c_DummyPacketHandler::GetPacketHandlerList()
			, 50
		);
	
	ASSERT_CRASH(pClientService->Start());
	
	Mgr(ThreadMgr)->Launch(
		  2
		, []() noexcept {const volatile auto init_builder = GetBuilder(); }
	);

	//std::wstring inputIP;
	//std::wcout << L"Input IP Address: ";
	//std::wcin >> inputIP;
	//if (!isValidIPAddress(inputIP))
	//{
	//	std::wcout << L"Invalid Address !'\n";
	//}
	//std::cout << "Num of Threads: ";
	//int num_th = 0;
	//std::cin >> num_th;
	//std::cout << "Num of Dummy: ";
	//int num_dm = 0;
	//std::cin >> num_dm;
	//
	//const auto pClientService = new ServerCore::ClientService
	//	(
	//		Mgr(CoreGlobal)->GetIocpCore()
	//		, ServerCore::NetAddress{ inputIP,7777 }
	//		, ServerCore::xnew<ServerSession>
	//		, s2c_DummyPacketHandler::GetPacketHandlerList()
	//		, num_dm
	//	);
	//
	//ASSERT_CRASH(pClientService->Start());
	//
	//Mgr(ThreadMgr)->Launch(
	//	num_th
	//	, []() noexcept {const volatile auto init_builder = GetBuilder(); }
	//);

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
