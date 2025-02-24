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
#include <regex>
#include "InputHandler.h"
#include "ServerObjectMgr.h"
#include "ServerObject.h"
#include "NaviCell.h"
#include "Navigator.h"
#include "ServerTimeMgr.h"

#include "GizmoBoxRenderer.h"
#include "GizmoCylinderRenderer.h"
#include "GizmoSphereRenderer.h"

#include "../common/json.hpp"

using namespace udsdx;

std::shared_ptr<Scene> scene;

std::shared_ptr<SceneObject> playerLightObj;
std::shared_ptr<SceneObject> g_heroObj;
std::shared_ptr<SceneObject> terrainObj;

AuthenticPlayer* g_heroComponent;

std::shared_ptr<udsdx::Material> terrainMaterial;
std::shared_ptr<udsdx::Material> playerMaterial;
std::shared_ptr<udsdx::Material> g_skyboxMaterial;
std::array<std::shared_ptr<udsdx::Material>, 2> g_treeMaterials;
std::shared_ptr<udsdx::Mesh> terrainMesh;

std::unique_ptr<HeightMap> heightMap;
std::unique_ptr<TerrainData> terrainData;

Vector3 moveDirection = Vector3::Zero;
Vector3 g_curPos = {};
float yawDirection = 0.0f;
float g_lightAngle = 0.0f;

void Update(const Time& time);
void PlaceTrees(std::wstring_view filename, std::shared_ptr<Scene> targetScene);

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
   //NAVIGATION->GetNavMesh(NAVI_MESH_NUM::NUM_0)->LoadByObj(RESOURCE_PATH(L"navmesh1000.obj"));
    if constexpr (true == g_bUseNetWork)
    {
        if constexpr (true == g_bUseDefaultIP)
        {
            // L"3.39.255.229"
            NET_NAGOX_ASSERT(NetMgr(NetworkMgr)->Connect<ServerSession>(L"3.39.255.229", 7777, s2c_PacketHandler::GetPacketHandlerList()));
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

    auto res = INSTANCE(Resource);
    auto shader = res->Load<Shader>(RESOURCE_PATH(L"color.hlsl"));
    auto shaderTerrain = res->Load<Shader>(RESOURCE_PATH(L"terrain.hlsl"));

    playerMaterial = std::make_shared<udsdx::Material>();
    playerMaterial->SetMainTexture(res->Load<udsdx::Texture>(RESOURCE_PATH(L"Sprite-0001.png")));

    terrainMaterial = std::make_shared<udsdx::Material>();
    terrainMaterial->SetMainTexture(res->Load<udsdx::Texture>(RESOURCE_PATH(L"terrain\\T_ground_beech_forest_soil_01_BC_SM.tga")));

    scene = std::make_shared<Scene>();

    g_heroObj = std::make_shared<SceneObject>();
    
    
    auto heroServerComponent = g_heroObj->AddComponent<ServerObject>();
  
    heroServerComponent->AddComp<MovePacketSender>();
    g_heroComponent = g_heroObj->AddComponent<AuthenticPlayer>();

    // Gizmo Renderer for debugging
    auto gizmoRenderer = g_heroObj->AddComponent<GizmoCylinderRenderer>();
    gizmoRenderer->SetRadius(1.0f);
	gizmoRenderer->SetHeight(3.0f);
  
    Vector3 temp = Vector3(0.0f, 40.0f, 0.0f);
    auto& cell = heroServerComponent->m_pNaviAgent->GetCurCell();
    cell = NAVIGATION->GetNavMesh(NAVI_MESH_NUM::NUM_0)->GetNaviCell(temp);

    g_heroObj->GetTransform()->SetLocalPosition(temp);

    scene->AddObject(g_heroObj);

    playerLightObj = std::make_shared<SceneObject>();
    auto playerLight = playerLightObj->AddComponent<LightDirectional>();
    playerLightObj->GetTransform()->SetLocalRotation(Quaternion::CreateFromYawPitchRoll(PI / 4.0f, PI / 4.0f, 0.0f));

    scene->AddObject(playerLightObj);

    {
        auto treeTexture1 = res->Load<udsdx::Texture>(RESOURCE_PATH(L"environment\\T_beech_atlas_BC.tga"));
        auto treeTexture2 = res->Load<udsdx::Texture>(RESOURCE_PATH(L"environment\\T_beech_bark_01_BC.tga"));

        g_treeMaterials[0] = std::make_shared<udsdx::Material>();
        g_treeMaterials[0]->SetMainTexture(treeTexture1);
        g_treeMaterials[1] = std::make_shared<udsdx::Material>();
        g_treeMaterials[1]->SetMainTexture(treeTexture2);
    }

    heightMap = std::make_unique<HeightMap>(RESOURCE_PATH(L"terrain_height.raw"), 513, 513);
    terrainMesh = CreateMeshFromHeightMap(heightMap.get(), 128, 128, 1.0f);
    terrainMesh->UploadBuffers(INSTANCE(Core)->GetDevice(), INSTANCE(Core)->GetCommandList());

    terrainData = std::make_unique<TerrainData>(RESOURCE_PATH(L"terrain_treeInstances.json"));
    auto terrainInstance = std::make_shared<SceneObject>();
    auto terrainInstanceRenderer = terrainInstance->AddComponent<TerrainInstanceRenderer>();
    terrainInstanceRenderer->SetTerrainData(terrainData.get());
    terrainInstanceRenderer->AddMesh(res->Load<udsdx::Mesh>(RESOURCE_PATH(L"environment\\beech_plant_01.fbx")));
    terrainInstanceRenderer->AddMesh(res->Load<udsdx::Mesh>(RESOURCE_PATH(L"environment\\beech_plant_03.fbx")));
    terrainInstanceRenderer->AddMesh(res->Load<udsdx::Mesh>(RESOURCE_PATH(L"environment\\beech_tree_00_B.fbx")));
    terrainInstanceRenderer->AddMesh(res->Load<udsdx::Mesh>(RESOURCE_PATH(L"environment\\beech_tree_00_D.fbx")));
    terrainInstanceRenderer->AddMesh(res->Load<udsdx::Mesh>(RESOURCE_PATH(L"environment\\beech_tree_01.fbx")));
    terrainInstanceRenderer->AddMesh(res->Load<udsdx::Mesh>(RESOURCE_PATH(L"environment\\beech_tree_02.fbx")));
    terrainInstanceRenderer->AddMesh(res->Load<udsdx::Mesh>(RESOURCE_PATH(L"environment\\beech_tree_03.fbx")));
    terrainInstanceRenderer->AddMesh(res->Load<udsdx::Mesh>(RESOURCE_PATH(L"environment\\beech_tree_04.fbx")));
    terrainInstanceRenderer->AddMesh(res->Load<udsdx::Mesh>(RESOURCE_PATH(L"environment\\beech_tree_05.fbx")));
    terrainInstanceRenderer->AddMesh(res->Load<udsdx::Mesh>(RESOURCE_PATH(L"environment\\beech_tree_06.fbx")));
    terrainInstanceRenderer->AddMesh(res->Load<udsdx::Mesh>(RESOURCE_PATH(L"environment\\beech_tree_07.fbx")));
    terrainInstanceRenderer->AddMesh(res->Load<udsdx::Mesh>(RESOURCE_PATH(L"environment\\beech_tree_08.fbx")));
    terrainInstanceRenderer->AddMesh(res->Load<udsdx::Mesh>(RESOURCE_PATH(L"environment\\beech_tree_09.fbx")));
    terrainInstanceRenderer->AddMesh(res->Load<udsdx::Mesh>(RESOURCE_PATH(L"environment\\Forest_black_cherry_02.fbx")));
    terrainInstanceRenderer->AddMesh(res->Load<udsdx::Mesh>(RESOURCE_PATH(L"environment\\Forest_black_cherry_04.fbx")));
    terrainInstanceRenderer->SetShader(res->Load<udsdx::Shader>(RESOURCE_PATH(L"colorinstanced.hlsl")));
    terrainInstanceRenderer->SetMaterial(g_treeMaterials[1].get(), 0);
    terrainInstanceRenderer->SetMaterial(g_treeMaterials[0].get(), 1);
    scene->AddObject(terrainInstance);

    {
        terrainObj = std::make_shared<SceneObject>();
        terrainObj->GetTransform()->SetLocalPosition(Vector3(0.0f, 0.0f, 0.0f));
        terrainObj->GetTransform()->SetLocalScale(Vector3(-1.0f, 1.0f, -1.0f) * 512.0f);
        auto terrainRenderer = terrainObj->AddComponent<MeshRenderer>();
        terrainRenderer->SetMesh(terrainMesh.get());
        terrainRenderer->SetMaterial(terrainMaterial.get());
        terrainRenderer->SetShader(shaderTerrain);
        terrainRenderer->SetTopology(D3D_PRIMITIVE_TOPOLOGY_16_CONTROL_POINT_PATCHLIST);
        scene->AddObject(terrainObj);
    }

    {
        auto navMeshObj = std::make_shared<SceneObject>();
        auto navMeshRenderer = navMeshObj->AddComponent<MeshRenderer>();
        navMeshRenderer->SetMesh(res->Load<udsdx::Mesh>(RESOURCE_PATH(L"NAVIMESHOBJ.obj")));
        navMeshRenderer->SetMaterial(terrainMaterial.get());
        navMeshRenderer->SetShader(shader);
        navMeshRenderer->SetTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        scene->AddObject(navMeshObj);
    }

    {
        auto skyboxObj = std::make_shared<SceneObject>();
        auto skyboxRenderer = skyboxObj->AddComponent<InlineMeshRenderer>();
        skyboxRenderer->SetShader(res->Load<Shader>(RESOURCE_PATH(L"skybox.hlsl")));
        skyboxRenderer->SetVertexCount(6);
        skyboxRenderer->SetCastShadow(false);

        auto skyboxTexture = res->Load<udsdx::Texture>(RESOURCE_PATH(L"Skybox.jpg"));
        g_skyboxMaterial = std::make_shared<udsdx::Material>();
        g_skyboxMaterial->SetMainTexture(skyboxTexture);
        skyboxRenderer->SetMaterial(g_skyboxMaterial.get());

        scene->AddObject(skyboxObj);
    }

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
        // TODO: 일단 끄기전 서버오브젝트 컨테이너 밀어줌
        ServerObjectMgr::GetInst()->Clear();
        // TODO: 이거해야 메모리릭 없는데 왜 터짐?
       // NAVIGATION->GetNavMesh(NAVI_MESH_NUM::NUM_0)->FreeNavMeshQuery();
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
    playerLightObj->GetTransform()->SetLocalRotation(Quaternion::CreateFromYawPitchRoll(-PIDIV4, PI / 4.0f, 0) * Quaternion::CreateFromAxisAngle(n, g_lightAngle));
}

void PlaceTrees(std::wstring_view filename, std::shared_ptr<Scene> targetScene)
{
    auto res = INSTANCE(Resource);
	auto shader = res->Load<Shader>(RESOURCE_PATH(L"color.hlsl"));

	std::ifstream file(filename.data());
	nlohmann::json j;
	file >> j;

    for (auto& tree : j)
	{
        Vector3 position = Vector3(tree["position_x"], tree["position_y"], tree["position_z"]);
        Quaternion rotation = Quaternion::CreateFromYawPitchRoll(0.0f, -PIDIV2, tree["rotation_y"]);
        Vector3 scale = Vector3(tree["size_width"], tree["size_height"], tree["size_width"]);
        std::string treePrototype = tree["prototype_name"];
        std::wstring wtreePrototype(treePrototype.begin(), treePrototype.end());

        auto treeObj = std::make_shared<SceneObject>();
        treeObj->GetTransform()->SetLocalPosition(position * Vector3(-1.0f, 1.0f, -1.0f) * 512.0f + Vector3::Up * -100.0f);
        treeObj->GetTransform()->SetLocalRotation(rotation);
        treeObj->GetTransform()->SetLocalScale(scale * 0.02f);
        auto treeRenderer = treeObj->AddComponent<MeshRenderer>();
        treeRenderer->SetMesh(res->Load<udsdx::Mesh>(RESOURCE_PATH(L"environment\\" + wtreePrototype + L".fbx")));
        treeRenderer->SetShader(shader);

        // if the name of the tree contains "tree", set the bark material
        if (treePrototype.find("tree_00") != std::string::npos)
        {
            treeRenderer->SetMaterial(g_treeMaterials[0].get(), 0);
            treeRenderer->SetMaterial(g_treeMaterials[1].get(), 1);
        }
        if (treePrototype.find("tree") != std::string::npos)
        {
            treeRenderer->SetMaterial(g_treeMaterials[1].get(), 0);
            treeRenderer->SetMaterial(g_treeMaterials[0].get(), 1);
        }
		else
            treeRenderer->SetMaterial(g_treeMaterials[0].get(), 0);

        targetScene->AddObject(treeObj);
	}
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

            uint16_t ib[6]{};

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
        }
    }

    for (LONG y = 0; y < segmentHeight - 2; ++y)
    {
        for (LONG x = 0; x < segmentWidth - 2; ++x)
        {
            for (LONG k = 0; k < 4; ++k)
            {
                indices.push_back((y + k) * (segmentWidth + 1) + x);
                indices.push_back((y + k) * (segmentWidth + 1) + x + 1);
                indices.push_back((y + k) * (segmentWidth + 1) + x + 2);
                indices.push_back((y + k) * (segmentWidth + 1) + x + 3);
            }
        }
    }

    for (int i = 0; i < positions.size(); ++i)
    {
        normals[i].Normalize();
        vertices.emplace_back(Vertex(positions[i], uvs[i], normals[i], Vector3::One));
    }

    auto mesh = std::make_shared<udsdx::Mesh>(vertices, indices);
    return mesh;
}