#include "pch.h"
#include "ServerSession.h"
#include "s2c_DummyPacketHandler.h"
#include "Service.h"
#include "CreateBuffer4Dummy.h"
#include <regex>
#include "Navigator.h"
#include "HarvestLoader.h"

std::shared_ptr<HeightMap> g_heightMap;
bool isValidIPAddress(std::wstring_view ipAddress) {
	const std::wregex ipRegex(L"^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");
	return std::regex_match(ipAddress.data(), ipRegex);
}
extern Vector3 SetTerrainPos(const Vector3& v);

class ContentsInitiator
	: public NagiocpX::Initiator
{
public:
	virtual void GlobalInitialize()noexcept override
	{
		
		
	}

	virtual void TLSInitialize()noexcept override
	{
		const volatile auto init_builder = GetBuilder();
	}

	virtual void TLSDestroy()noexcept override
	{
		
	}

	virtual void GlobalDestroy()noexcept override
	{
	}

	virtual void ControlThreadFunc()noexcept override
	{
		for (;;)
		{
			system("pause");
			char buf[32]{};
			std::cin >> buf;
			if ("EXIT" == std::string_view{ buf })break;
		}
	}
};

int main()
{
	ContentsInitiator con_init;
	//g_heightMap = std::make_shared<HeightMap>(RESOURCE_PATH(L"terrain_height.raw"), 4096, 4096);
	Mgr(CoreGlobal)->Init();
	NAVIGATION->Init();
	NAVIGATION->RegisterDestroy();
	s2c_DummyPacketHandler::Init();
	HarvestLoader::LoadHarvest({}, L"environment\\ExportedGameSpawns.json");

	const auto pClientService = new NagiocpX::ClientService
		(
			  Mgr(CoreGlobal)->GetIocpCore()
			, NagiocpX::NetAddress{ L"127.0.0.1",7777 }
			, NagiocpX::xnew<ServerSession>
			, s2c_DummyPacketHandler::GetPacketHandlerList()
			, 5000
		);
	
	
	std::thread t1{ [pClientService]() {ASSERT_CRASH(pClientService->Start()); } };
	
	Mgr(ThreadMgr)->Launch(
		  4
		, &con_init
	);

	t1.join();

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
	//const auto pClientService = new NagiocpX::ClientService
	//	(
	//		Mgr(CoreGlobal)->GetIocpCore()
	//		, NagiocpX::NetAddress{ inputIP,7777 }
	//		, NagiocpX::xnew<ServerSession>
	//		, s2c_DummyPacketHandler::GetPacketHandlerList()
	//		, num_dm
	//	);
	//
	//ASSERT_CRASH(pClientService->Start());
	//
	//Mgr(ThreadMgr)->Launch(
	//	num_th
	//  , &con_init
	//);

	uint64_t accCount = 0;
	uint64_t accTime = 0;
	const auto con = NagiocpX::Service::GetMainService()->GetAllSessionContainer();
	
	for (const auto& c : con)
	{
		if (const auto con = c.ptr.load())
		{
			const auto seesion = (ServerSession*)con->GetSession();
			accCount += seesion->m_moveCount;
			accTime += seesion->m_accDelayMs;
		}
	}

	if (accTime)
		std::cout << "\n\n\nDelay AVG: " << ((float)accTime) / accCount << std::endl;

	HarvestLoader::FreeHarvestLoader();

	system("pause");
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
