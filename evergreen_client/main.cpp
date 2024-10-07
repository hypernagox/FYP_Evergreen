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
#include "AuthenticPlayer.h"
#include "PlayerRenderer.h"
#include "MovePacketSender.h"
#include "EntityMovement.h"
#include <regex>
#include "InputHandler.h"
#include "ServerObjectMgr.h"
#include "ServerObject.h"
#include "NaviMesh.h"
#include "NaviCell.h"
#include "Navigator.h"

using namespace udsdx;

std::shared_ptr<Scene> scene;

std::shared_ptr<SceneObject> playerLightObj;

std::shared_ptr<SceneObject> g_heroObj;

std::shared_ptr<SceneObject> terrainObj;

AuthenticPlayer* g_heroComponent;

std::shared_ptr<udsdx::Material> terrainMaterial;
std::shared_ptr<udsdx::Material> playerMaterial;
std::shared_ptr<udsdx::Material> g_skyboxMaterial;
std::shared_ptr<udsdx::Mesh> terrainMesh;

std::unique_ptr<HeightMap> heightMap;

Vector3 moveDirection = Vector3::Zero;
Vector3 g_curPos = {};
float yawDirection = 0.0f;
float g_lightAngle = 0.0f;

void Update(const Time& time);

std::shared_ptr<udsdx::Mesh> CreateMeshFromHeightMap(const HeightMap* heightMap, LONG segmentWidth, LONG segmentHeight, float heightScale);

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

    NAVIGATION->Init();
    NAVIGATION->RegisterDestroy();
    s2c_PacketHandler::Init();
    
   // NAVIGATION->GetNavMesh(NAVI_MESH_NUM::NUM_0)-> Load(RESOURCE_PATH(L"NAVIMESH.bin"));
    //NAVIGATION->GetNavMesh(NAVI_MESH_NUM::NUM_0)->LoadByObj(RESOURCE_PATH(L"navmesh10.obj"));
    if constexpr (true == g_bUseNetWork)
    {
        if constexpr (true == g_bUseDefaultIP)
        {
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
            RE_INPUT:
                std::wcout << L"Input IP Address: ";
                std::wcin >> inputIP;
                if (!isValidIPAddress(inputIP))
                {
                    std::wcout << L"Invalid Address !'\n";
                    goto RE_INPUT;
                }
            } while (!NetMgr(NetworkMgr)->Connect<ServerSession>(inputIP, 7777, s2c_PacketHandler::GetPacketHandlerList()));
        }
    }
    
    if constexpr (true == g_bUseNetWork)
    {
        Send(Create_c2s_LOGIN("Hello"));
    }

    auto res = INSTANCE(Resource);
    auto shader = res->Load<Shader>(RESOURCE_PATH(L"color.hlsl"));

    playerMaterial = std::make_shared<udsdx::Material>();
    playerMaterial->SetMainTexture(res->Load<udsdx::Texture>(RESOURCE_PATH(L"Sprite-0001.png")));

    terrainMaterial = std::make_shared<udsdx::Material>();
    terrainMaterial->SetMainTexture(res->Load<udsdx::Texture>(RESOURCE_PATH(L"Black_Sand_BaseColor.tif")));

    scene = std::make_shared<Scene>();

    g_heroObj = std::make_shared<SceneObject>();
    
    g_heroObj->GetTransform()->SetLocalPosition(NAVIGATION->GetNavMesh(NAVI_MESH_NUM::NUM_0)->FindCellContainingOrClosestPoint({})->GetCellCenter());
   
    auto heroServerComponent = g_heroObj->AddComponent<ServerObject>();
    heroServerComponent->AddComp<MovePacketSender>();
    
    g_heroObj->AddComponent<InputHandler>();
    g_heroObj->AddComponent<PlayerRenderer>();
    g_heroObj->AddComponent<EntityMovement>();
    g_heroComponent = g_heroObj->AddComponent<AuthenticPlayer>();
  
    scene->AddObject(g_heroObj);

    playerLightObj = std::make_shared<SceneObject>();
    auto playerLight = playerLightObj->AddComponent<LightDirectional>();
    playerLightObj->GetTransform()->SetLocalRotation(Quaternion::CreateFromYawPitchRoll(PI / 4.0f, PI / 4.0f, 0.0f));

    scene->AddObject(playerLightObj);

    heightMap = std::make_unique<HeightMap>(RESOURCE_PATH(L"terrain_test.raw"), 4096/8, 4096/8);
    terrainMesh = CreateMeshFromHeightMap(heightMap.get(), 1000, 1000, 1.0f);
    terrainMesh->UploadBuffers(INSTANCE(Core)->GetDevice(), INSTANCE(Core)->GetCommandList());

    constexpr float terrainScale = 100.0f;
    constexpr float terrainOffset = -1200.0f;
    terrainObj = std::make_shared<SceneObject>();
    //terrainObj->GetTransform()->SetLocalScale(Vector3::One *0.8f);
   //terrainObj->GetTransform()->SetLocalPosition(Vector3(terrainOffset, 0.0f, -terrainOffset));
    //terrainObj->GetTransform()->SetLocalPosition(Vector3(500, 0.0f, 500));
    auto terrainRenderer = terrainObj->AddComponent<MeshRenderer>();
    terrainRenderer->SetMesh(res->Load<udsdx::Mesh>(RESOURCE_PATH(L"Terrain10.obj")));
    terrainRenderer->SetMaterial(terrainMaterial.get());
    terrainRenderer->SetShader(shader);

    {
        auto terrainObj = std::make_shared<SceneObject>();
       // terrainObj->GetTransform()->SetLocalScale(Vector3::One *2.f);
       // terrainObj->GetTransform()->SetLocalScale(Vector3::One * 1.f);;
       // terrainObj->GetTransform()->SetLocalPosition(Vector3(-256, 0.0f, -256 ));
        auto terrainRenderer = terrainObj->AddComponent<MeshRenderer>();
        terrainRenderer->SetMesh(res->Load<udsdx::Mesh>(RESOURCE_PATH(L"navmesh10.obj")));
        terrainRenderer->SetMaterial(terrainMaterial.get());
        terrainRenderer->SetTopology(D3D10_PRIMITIVE_TOPOLOGY_LINELIST);
        terrainRenderer->SetShader(shader);
        scene->AddObject(terrainObj);
    }
    scene->AddObject(terrainObj);

    if constexpr (true == g_bUseNetWork)
    {
        ServerObjectMgr::GetInst()->SetTargetScene(scene);
        Send(Create_c2s_ENTER(ToFlatVec3(g_heroObj->GetTransform()->GetLocalPosition())));
    }

    if constexpr (true == g_bUseNetWork)
    {
		NetMgr(NetworkMgr)->DoNetworkIO();
	}

    INSTANCE(Input)->SetRelativeMouse(true);

    if constexpr (true == g_bUseNetWork)
    {
        UpdownStudio::RegisterIOUpdateCallback([]()noexcept {
            NetMgr(NetworkMgr)->DoNetworkIO();
            });
    }
    return UpdownStudio::Run(scene, nCmdShow);
}

void Update(const Time& time)
{
    INSTANCE(EventTimer)->Update(time.deltaTime);

    if (INSTANCE(Input)->GetKeyDown(Keyboard::F1))
	{
        float p = rand() % 100 / 100.0f;
        float t = time.totalTime + p;
        DebugConsole::Log("Registering the event at " + std::to_string(t) + "s");
        INSTANCE(EventTimer)->RegisterEvent(p, [t]() {
            DebugConsole::Log("Calling the event at " + std::to_string(t) + "s");
			});
	}

    if (INSTANCE(Input)->GetKeyDown(Keyboard::Escape))
    {
        UpdownStudio::Quit();
    }

    Vector3 terrainPos = g_heroObj->GetTransform()->GetLocalPosition() * 0.01f;
    terrainPos.x = fmod(terrainPos.x + 1.0f, 1.0f);
    terrainPos.z = fmod(terrainPos.z + 1.0f, 1.0f);
    float terrainHeight = heightMap->GetHeight(terrainPos.x * 4096/8, terrainPos.z * 4096/8);
    terrainPos.y = std::max(terrainPos.y, terrainHeight);
    //g_heroObj->GetTransform()->SetLocalPosition(terrainPos * 100.0f);
   // g_curPos = terrainPos * 100.0f;
   // SetTerrainPos(g_heroObj);
    // g_lightAngle += DT * 0.5f;
    float theta = 105.0f * DEG2RAD;
    Vector3 n = Vector3::Transform(Vector3::Up, Quaternion::CreateFromAxisAngle(Vector3(1.0f, 0.0f, -1.0f), 75.0f - 105.0f * 0.5f));
    playerLightObj->GetTransform()->SetLocalRotation(Quaternion::CreateFromYawPitchRoll(-PIDIV4, PI / 3.0f, 0) * Quaternion::CreateFromAxisAngle(n, g_lightAngle));
}

std::shared_ptr<udsdx::Mesh> CreateMeshFromHeightMap(const HeightMap* heightMap, LONG segmentWidth, LONG segmentHeight, float heightScale)
{
    std::vector<Vertex> vertices;
    std::vector<Vector3> positions;
    std::vector<Vector3> normals;
    std::vector<Vector2> uvs;
    std::vector<UINT> indices;

    for (LONG y = 0; y < segmentHeight + 1; ++y)
    {
        for (LONG x = 0; x < segmentWidth + 1; ++x)
        {
            const float u = x / static_cast<float>(segmentWidth);
            const float v = y / static_cast<float>(segmentHeight);

            const float px = u * (heightMap->GetPixelWidth() - 1);
            const float py = v * (heightMap->GetPixelHeight() - 1);

            const float h = heightMap->GetHeight(px, py) * heightScale;

            positions.emplace_back(Vector3(u, h, v));
            normals.emplace_back(Vector3::Zero);
            uvs.emplace_back(Vector2(u, v));
        }
    }

    for (LONG y = 0; y < segmentHeight; ++y)
    {
        for (LONG x = 0; x < segmentWidth; ++x)
        {
            LONG base = y * (segmentWidth + 1) + x;

            float h01 = positions[base].y;
            float h11 = positions[base + segmentWidth + 1].y;
            float h00 = positions[base + 1].y;
            float h10 = positions[base + segmentWidth + 2].y;

            float d1 = h11 - h00;
            float d2 = h10 - h01;

            UINT ib[6]{};

            if (abs(d1) > abs(d2))
            {
                ib[0] = base;
                ib[1] = base + segmentWidth + 1;
                ib[2] = base + 1;
                ib[3] = base + segmentWidth + 2;
                ib[4] = base + 1;
                ib[5] = base + segmentWidth + 1;
            }
            else
            {
                ib[0] = base + segmentWidth + 1;
                ib[1] = base + segmentWidth + 2;
                ib[2] = base;
                ib[3] = base + 1;
                ib[4] = base;
                ib[5] = base + segmentWidth + 2;
            }

            Vector3 n1 = (positions[ib[1]] - positions[ib[0]]).Cross(positions[ib[2]] - positions[ib[0]]);
            Vector3 n2 = (positions[ib[4]] - positions[ib[3]]).Cross(positions[ib[5]] - positions[ib[3]]);
            n1.Normalize();
            n2.Normalize();

            normals[ib[0]] += n1;
            normals[ib[1]] += n1;
            normals[ib[2]] += n1;
            normals[ib[3]] += n2;
            normals[ib[4]] += n2;
            normals[ib[5]] += n2;

            for (int i = 0; i < 6; ++i)
            {
                indices.emplace_back(ib[i]);
            }
        }
    }

    for (int i = 0; i < positions.size(); ++i)
    {
        normals[i].Normalize();
        vertices.emplace_back(Vertex(positions[i], uvs[i], normals[i], Vector3::Up));
    }

    auto mesh = std::make_shared<udsdx::Mesh>(vertices, indices);
    return mesh;
}