#include "pch.h"
// Windows Header Files
#include <windows.h>
// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include "EventTimer.h"
#include "ServerSession.h"

#include <HeightMap.h>
#include "TerrainData.h"
#include "TerrainInstanceRenderer.h"
#include "AuthenticPlayer.h"
#include "PlayerRenderer.h"
#include "MovePacketSender.h"
#include "EntityMovement.h"
#include "TerrainDetail.h"
#include "TerrainDetailRenderer.h"
#include <regex>
#include "InputHandler.h"
#include "ServerObjectMgr.h"
#include "ServerObject.h"
#include "NaviCell.h"
#include "Navigator.h"
#include "ServerTimeMgr.h"

#include "../common/json.hpp"
#include "ServerObjectMgr.h"

#include "GameScene.h"
#include "GuideSystem.h"

using namespace udsdx;

std::shared_ptr<GameScene> g_scene;

std::unique_ptr<TerrainDetail> g_terrainDetail;
std::unique_ptr<HeightMap> g_heightMap;
std::unique_ptr<TerrainData> g_terrainData;

void Update(const Time& time);
void ProcessLogin();

bool isValidIPAddress(std::wstring_view ipAddress) {
    const std::wregex ipRegex(L"^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");
    return std::regex_match(ipAddress.data(), ipRegex);
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    INSTANCE(Resource)->SetResourceRootPath(RESOURCE_PATH(L""));
    UpdownStudio::Initialize(hInstance);
    UpdownStudio::RegisterUpdateCallback(Update);

    GuideSystem::GetInst()->Init();
    NAVIGATION->Init();
    NAVIGATION->RegisterDestroy();
    Common::DataRegistry::Load();
    s2c_PacketHandler::Init();

    // Height Map: 지형의 (x, z) 좌표에 대한 y 높이를 담는 맵
    g_heightMap = std::make_unique<HeightMap>(RESOURCE_PATH(L"terrain_beta_04_28.raw"), 2049, 2049);

    // Terrain Detail: 잔디의 위치를 담는 맵, 잔디 위치에 대한 버텍스 / 인덱스 버퍼 포함
    g_terrainDetail = std::make_unique<TerrainDetail>(g_heightMap.get(), RESOURCE_PATH(L"environment\\Terrain_Detail.raw"), 512, 512, 16, INSTANCE(Core)->GetDevice(), INSTANCE(Core)->GetCommandList());

	// Terrain Data: 나무, 바위, 건물 등 지형에 배치할 오브젝트의 위치를 담는 자료 구조
    g_terrainData = std::make_unique<TerrainData>(RESOURCE_PATH(L"environment\\ExportedInstance.json"), 1.0f, 0.01f);

    g_scene = std::make_shared<GameScene>(g_heightMap.get(), g_terrainData.get(), g_terrainDetail.get());

    // 타겟씬은 이런저런 테스트 할 때 늘 필요 ..
   // if constexpr (true == g_bUseNetWork)
    {
        ServerObjectMgr::GetInst()->SetTargetScene(g_scene);
    }

    if constexpr (true == g_bUseNetWork)
    {
        NetMgr(NetworkMgr)->RegisterLoginRoutine(ProcessLogin);
    }

    return UpdownStudio::Run(g_scene, nCmdShow);
}

void Update(const Time& time)
{
    INSTANCE(EventTimer)->Update(time.deltaTime);
}

void ProcessLogin()
{
    UpdownStudio::RegisterIOUpdateCallback(std::bind_front(&NetHelper::NetworkMgr::DoNetworkIO, NetMgr(NetworkMgr), 0));

    if constexpr (true == g_bUseNetWork)
    {
        if constexpr (true == g_bUseDefaultIP)
        {
            // L"3.39.255.229"
            NET_NAGOX_ASSERT(NetMgr(NetworkMgr)->Connect<ServerSession>(L"127.0.0.1", 7777, s2c_PacketHandler::GetPacketHandlerList()));
        }
        else
        {
            // TODO: 종료시 이거 해제 안하면 메모리릭 경고날 듯?
            AllocConsole();

            FILE* fp = nullptr;
            freopen_s(&fp, "CONOUT$", "w", stdout);
            freopen_s(&fp, "CONIN$", "r", stdin);

            std::wstring inputIP;
            do {
                std::wcout << L"Input IP Address: ";
                std::wcin >> inputIP;
                if (!isValidIPAddress(inputIP))
                {
                    std::wcout << L"Invalid Address !'\n";
                }
            } while (!NetMgr(NetworkMgr)->Connect<ServerSession>(inputIP, 7777, s2c_PacketHandler::GetPacketHandlerList()));
        }
    }

    if constexpr (true == g_bUseNetWork)
    {
        Send(Create_c2s_LOGIN("Hello"));
        NetMgr(ServerTimeMgr)->InitAndWaitServerTimeStamp([]()noexcept {NetMgr(NetworkMgr)->Send(Create_c2s_PING_PONG()); });
    }

    if constexpr (true == g_bUseNetWork)
    {
        NetMgr(NetworkMgr)->DoNetworkIO();
    }
}
