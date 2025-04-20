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

#include "PlayerStatusGUI.h"
#include "PlayerQuickSlotGUI.h"
#include "PlayerInventoryGUI.h"
#include "PlayerCraftGUI.h"

#include "GizmoBoxRenderer.h"
#include "GizmoCylinderRenderer.h"
#include "GizmoSphereRenderer.h"

#include "../common/json.hpp"
#include "ServerObjectMgr.h"
#include "FocusAgentGUI.h"

using namespace udsdx;

std::shared_ptr<Scene> scene;

std::shared_ptr<SceneObject> playerLightObj;
std::shared_ptr<SceneObject> g_heroObj;
std::shared_ptr<SceneObject> terrainObj;

AuthenticPlayer* g_heroComponent;

std::shared_ptr<udsdx::Material> terrainMaterial;
std::shared_ptr<udsdx::Material> terrainDetailMaterial;
std::shared_ptr<udsdx::Material> playerMaterial;
std::shared_ptr<udsdx::Material> g_skyboxMaterial;
std::shared_ptr<udsdx::Material> g_gizmoMaterial;
std::shared_ptr<udsdx::Mesh> terrainMesh;

std::unique_ptr<TerrainDetail> terrainDetail;
std::unique_ptr<HeightMap> heightMap;
std::unique_ptr<TerrainData> terrainData;

std::shared_ptr<SceneObject> g_inventoryObj;
std::shared_ptr<SceneObject> g_craftObj;

std::vector<std::shared_ptr<udsdx::Material>> g_instanceMaterials;

std::unique_ptr<SoundEffectInstance> g_menuSound;

Vector3 moveDirection = Vector3::Zero;
Vector3 g_curPos = {};
float yawDirection = 0.0f;
float g_lightAngle = 0.0f;

bool g_inFocus = false;

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
    Common::DataRegistry::Load();
    s2c_PacketHandler::Init();

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

    auto res = INSTANCE(Resource);
    auto shader = res->Load<Shader>(RESOURCE_PATH(L"color.hlsl"));
    auto shaderTerrain = res->Load<Shader>(RESOURCE_PATH(L"terrain.hlsl"));

    playerMaterial = std::make_shared<udsdx::Material>();
    playerMaterial->SetSourceTexture(res->Load<udsdx::Texture>(RESOURCE_PATH(L"Sprite-0001.png")));

    terrainMaterial = std::make_shared<udsdx::Material>();
    terrainMaterial->SetSourceTexture(res->Load<udsdx::Texture>(RESOURCE_PATH(L"environment\\Maps\\TerrainSplatmap_0.tga")), 0);
    terrainMaterial->SetSourceTexture(res->Load<udsdx::Texture>(RESOURCE_PATH(L"environment\\Maps\\TerrainSplatmap_1.tga")), 1);
    terrainMaterial->SetSourceTexture(res->Load<udsdx::Texture>(RESOURCE_PATH(L"environment\\Maps\\TerrainSrc_0.png")), 2);
    terrainMaterial->SetSourceTexture(res->Load<udsdx::Texture>(RESOURCE_PATH(L"environment\\Maps\\TerrainSrc_1.tga")), 3);
    terrainMaterial->SetSourceTexture(res->Load<udsdx::Texture>(RESOURCE_PATH(L"environment\\Maps\\TerrainSrc_2.png")), 4);
    terrainMaterial->SetSourceTexture(res->Load<udsdx::Texture>(RESOURCE_PATH(L"environment\\Maps\\TerrainSrc_3.png")), 5);
    terrainMaterial->SetSourceTexture(res->Load<udsdx::Texture>(RESOURCE_PATH(L"environment\\Maps\\Maps\\T_ground_soil_01_BC_SM.tga")), 6);
    terrainMaterial->SetSourceTexture(res->Load<udsdx::Texture>(RESOURCE_PATH(L"environment\\Maps\\Maps\\T_ground_moss_01_BC_SM.tga")), 7);
    terrainMaterial->SetSourceTexture(res->Load<udsdx::Texture>(RESOURCE_PATH(L"environment\\Maps\\Maps\\Grass_and_Clover.tif")), 8);

    scene = std::make_shared<Scene>();

    g_heroObj = std::make_shared<SceneObject>();
    
    ServerObjectMgr::GetInst()->SetMainHero(NetMgr(NetworkMgr)->GetSessionID(), g_heroObj);
    auto heroServerComponent = g_heroObj->AddComponent<ServerObject>();
  
    heroServerComponent->AddComp<MovePacketSender>();
    g_heroComponent = g_heroObj->AddComponent<AuthenticPlayer>();
  
    Vector3 temp = Vector3(-4.345f, 76.17f, 0.0f);
    auto& cell = heroServerComponent->m_pNaviAgent->GetCurCell();
    cell = NAVIGATION->GetNavMesh(NAVI_MESH_NUM::NUM_0)->GetNaviCell(temp);

    g_heroObj->GetTransform()->SetLocalPosition(temp);

    scene->AddObject(g_heroObj);

    // Region: Focus Agent GUI
    {
        auto focusAgentObj = std::make_shared<SceneObject>();
        auto focusAgent = focusAgentObj->AddComponent<FocusAgentGUI>();

        focusAgent->SetSize(Vector2::One * 8192.0f);
        focusAgent->SetTryExitCallback([]() {
            // TODO: 일단 끄기전 서버오브젝트 컨테이너 밀어줌
            ServerObjectMgr::GetInst()->Clear();
            // TODO: 이거해야 메모리릭 없는데 왜 터짐?
           // NAVIGATION->GetNavMesh(NAVI_MESH_NUM::NUM_0)->FreeNavMeshQuery();
            UpdownStudio::Quit();
            });
        focusAgent->SetTryClickCallback([]() {
            g_heroComponent->TryClickScreen();
            });

        scene->AddObject(focusAgentObj);
    }

    playerLightObj = std::make_shared<SceneObject>();
    auto playerLight = playerLightObj->AddComponent<LightDirectional>();
    Vector3 n = Vector3::Transform(Vector3::Up, Quaternion::CreateFromAxisAngle(Vector3(1.0f, 0.0f, -1.0f), 75.0f - 105.0f * 0.5f));
    playerLightObj->GetTransform()->SetLocalRotation(Quaternion::CreateFromYawPitchRoll(-PIDIV4, PI / 4.0f, 0) * Quaternion::CreateFromAxisAngle(n, g_lightAngle));

    scene->AddObject(playerLightObj);

    heightMap = std::make_unique<HeightMap>(RESOURCE_PATH(L"terrain_beta.raw"), 513, 513);
    terrainMesh = CreateMeshFromHeightMap(heightMap.get(), 128, 128, 1.0f);
    terrainMesh->UploadBuffers(INSTANCE(Core)->GetDevice(), INSTANCE(Core)->GetCommandList());

    const float TerrainSize = GET_DATA(float, "TerrainSize", "Value");
    const Vector3 terrainPos = Vector3(-TerrainSize * 0.5f, 0, -TerrainSize * 0.5f);
    const Vector3 terrainScale = Vector3::One * TerrainSize;

    {
        terrainDetailMaterial = std::make_shared<udsdx::Material>();
        terrainDetailMaterial->SetSourceTexture(res->Load<udsdx::Texture>(RESOURCE_PATH(L"environment\\Grass.tga")), 0);

        terrainDetail = std::make_unique<TerrainDetail>(heightMap.get(), RESOURCE_PATH(L"environment\\Terrain_Detail.raw"), 512, 512, 16, INSTANCE(Core)->GetDevice(), INSTANCE(Core)->GetCommandList());
        std::shared_ptr<SceneObject> terrainDetailObj = std::make_shared<SceneObject>();
        auto terrainDetailRenderer = terrainDetailObj->AddComponent<TerrainDetailRenderer>();
        terrainDetailRenderer->SetTerrainDetail(terrainDetail.get());
        terrainDetailRenderer->SetShader(res->Load<udsdx::Shader>(RESOURCE_PATH(L"detailbillboard.hlsl")));
        terrainDetailRenderer->SetMaterial(terrainDetailMaterial.get());

        terrainDetailObj->GetTransform()->SetLocalPosition(terrainPos);
        terrainDetailObj->GetTransform()->SetLocalScale(terrainScale);

        scene->AddObject(terrainDetailObj);
    }

    terrainData = std::make_unique<TerrainData>(RESOURCE_PATH(L"environment\\ExportedInstance.json"), 1.0f, 0.01f);

    std::map<std::string, udsdx::Texture*> textureMap;
    for (auto texture : INSTANCE(Resource)->LoadAll<udsdx::Texture>())
		textureMap[texture->GetName().data()] = texture;

    for (const auto& directory : std::filesystem::recursive_directory_iterator(RESOURCE_PATH(L"environment")))
    {
        // if the file is not a regular file(e.g. if it is a directory), skip it
        if (!directory.is_regular_file())
            continue;

        std::string filename = directory.path().filename().stem().string();
        std::wstring suffix = directory.path().extension().wstring();
        std::transform(suffix.begin(), suffix.end(), suffix.begin(), ::tolower);

        if (suffix != L".fbx")
            continue;
        if (terrainData->GetPrototypeInstanceCount(filename) == 0)
			continue;

        auto terrainInstance = std::make_shared<SceneObject>();
        auto terrainInstanceRenderer = terrainInstance->AddComponent<TerrainInstanceRenderer>();
        auto mesh = res->Load<udsdx::Mesh>(directory.path().c_str());

        const auto& subMeshes = mesh->GetSubmeshes();
        for (size_t i = 0; i < subMeshes.size(); ++i)
        {
            auto material = std::make_shared<udsdx::Material>();
            std::string key = subMeshes[i].DiffuseTexturePath;
            std::transform(key.begin(), key.end(), key.begin(), ::tolower);

            // if the extension is .psd, replace it with .tga
            if (key.ends_with(".psd"))
				key.replace(key.end() - 4, key.end(), ".tga");

            if (auto it = textureMap.find(key); it != textureMap.end())
				material->SetSourceTexture(it->second);
			else
				material->SetSourceTexture(res->Load<udsdx::Texture>(RESOURCE_PATH(L"Sprite-0001.png")));
			g_instanceMaterials.emplace_back(material);
            terrainInstanceRenderer->SetMaterial(material.get(), i);
		}

        terrainInstanceRenderer->SetTerrainData(terrainData.get(), filename);
        terrainInstanceRenderer->SetMesh(mesh);
        terrainInstanceRenderer->SetShader(res->Load<udsdx::Shader>(RESOURCE_PATH(L"colorinstanced.hlsl")));

        scene->AddObject(terrainInstance);
    }

    {
        terrainObj = std::make_shared<SceneObject>();
        terrainObj->GetTransform()->SetLocalPosition(terrainPos);
        terrainObj->GetTransform()->SetLocalScale(terrainScale);
        auto terrainRenderer = terrainObj->AddComponent<MeshRenderer>();
        terrainRenderer->SetMesh(terrainMesh.get());
        terrainRenderer->SetMaterial(terrainMaterial.get());
        terrainRenderer->SetShader(shaderTerrain);
        terrainRenderer->SetTopology(D3D_PRIMITIVE_TOPOLOGY_16_CONTROL_POINT_PATCHLIST);

        scene->AddObject(terrainObj);
    }

    {
        auto skyboxObj = std::make_shared<SceneObject>();
        auto skyboxRenderer = skyboxObj->AddComponent<InlineMeshRenderer>();
        skyboxRenderer->SetShader(res->Load<Shader>(RESOURCE_PATH(L"skybox.hlsl")));
        skyboxRenderer->SetVertexCount(6);
        skyboxRenderer->SetCastShadow(false);

        auto skyboxTexture = res->Load<udsdx::Texture>(RESOURCE_PATH(L"Skybox.jpg"));
        g_skyboxMaterial = std::make_shared<udsdx::Material>();
        g_skyboxMaterial->SetSourceTexture(skyboxTexture);
        skyboxRenderer->SetMaterial(g_skyboxMaterial.get());

        scene->AddObject(skyboxObj);
    }

    {
        auto guiObj = std::make_shared<SceneObject>();
		auto guiRenderer = guiObj->AddComponent<PlayerStatusGUI>();
		scene->AddObject(guiObj);
        g_heroComponent->SetPlayerStatusGUI(guiRenderer);

        auto quickSlotObj = std::make_shared<SceneObject>();
        auto quickSlotRenderer = quickSlotObj->AddComponent<PlayerQuickSlotGUI>();
        scene->AddObject(quickSlotObj);
        g_heroComponent->SetPlayerQuickSlotGUI(quickSlotRenderer);

        g_inventoryObj = std::make_shared<SceneObject>();
        auto inventoryRenderer = g_inventoryObj->AddComponent<PlayerInventoryGUI>();
        scene->AddObject(g_inventoryObj);
        g_heroComponent->SetPlayerInventoryGUI(inventoryRenderer);

        g_craftObj = std::make_shared<SceneObject>();
        auto craftComp = g_craftObj->AddComponent<PlayerCraftGUI>();
        scene->AddObject(g_craftObj);
        g_heroComponent->SetPlayerCraftGUI(craftComp);
    }

    {
        auto textObj = std::make_shared<SceneObject>();
        auto textRenderer = textObj->AddComponent<GUIText>();
        textObj->GetTransform()->SetLocalPosition(Vector3(-640, 480, 0));
        textRenderer->SetText(GET_DATA(std::wstring, "Intro", "Start"));
        textRenderer->SetFont(res->Load<udsdx::Font>(RESOURCE_PATH(L"pretendard.spritefont")));

        scene->AddObject(textObj);
    }

    if (false)
    {
        g_gizmoMaterial = std::make_shared<udsdx::Material>();
        g_gizmoMaterial->SetSourceTexture(res->Load<udsdx::Texture>(RESOURCE_PATH(L"Sprite-0001.png")));

        auto navMeshVisualizer = std::make_shared<SceneObject>();
        auto navMeshRenderer = navMeshVisualizer->AddComponent<MeshRenderer>();
        navMeshRenderer->SetMesh(res->Load<udsdx::Mesh>(RESOURCE_PATH(L"navmesh.obj")));
        navMeshRenderer->SetShader(res->Load<udsdx::Shader>(RESOURCE_PATH(L"color.hlsl")));
        navMeshRenderer->SetMaterial(g_gizmoMaterial.get());

        scene->AddObject(navMeshVisualizer);
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

    auto audioAction = [](bool isOpen) {
		if (isOpen)
            g_menuSound = INSTANCE(Resource)->Load<udsdx::AudioClip>(RESOURCE_PATH(L"audio\\uiopen.wav"))->CreateInstance();
		else
            g_menuSound = INSTANCE(Resource)->Load<udsdx::AudioClip>(RESOURCE_PATH(L"audio\\uiclose.wav"))->CreateInstance();
        g_menuSound->SetVolume(0.5f);
        g_menuSound->Play();
	};

    if (INSTANCE(Input)->GetKeyDown(Keyboard::E))
    {
        g_inventoryObj->SetActive(!g_inventoryObj->GetActive());
        audioAction(g_inventoryObj->GetActive());
    }
    if (INSTANCE(Input)->GetKeyDown(Keyboard::C))
    {
        g_craftObj->SetActive(!g_craftObj->GetActive());
        audioAction(g_craftObj->GetActive());
    }

    Vector3 terrainPos = g_heroObj->GetTransform()->GetLocalPosition() / terrainObj->GetTransform()->GetLocalScale();
    terrainPos.x = fmod(terrainPos.x + 1.0f, 1.0f);
    terrainPos.z = fmod(terrainPos.z + 1.0f, 1.0f);
    float terrainHeight = heightMap->GetHeight(terrainPos.x * heightMap->GetPixelWidth(), terrainPos.z * heightMap->GetPixelHeight());
    terrainPos.y = std::max(terrainPos.y, terrainHeight);
    //g_heroObj->GetTransform()->SetLocalPosition(terrainPos * 100.0f);
   // g_curPos = terrainPos * 100.0f;
   // SetTerrainPos(g_heroObj);
    // g_lightAngle += DT * 0.5f;
    float theta = 105.0f * DEG2RAD;
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

            LONG ib[6]{};

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