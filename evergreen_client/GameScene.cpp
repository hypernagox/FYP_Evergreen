#include "pch.h"
#include "GameScene.h"

#include <HeightMap.h>
#include "TerrainData.h"
#include "TerrainInstanceRenderer.h"
#include "AuthenticPlayer.h"
#include "PlayerRenderer.h"
#include "MovePacketSender.h"
#include "EntityMovement.h"
#include "TerrainDetail.h"
#include "TerrainDetailRenderer.h"
#include "InputHandler.h"
#include "ServerObjectMgr.h"
#include "ServerObject.h"
#include "NaviCell.h"
#include "Navigator.h"
#include "BezierMovement.h"

#include "PlayerStatusGUI.h"
#include "PlayerQuickSlotGUI.h"
#include "PlayerInventoryGUI.h"
#include "PlayerCraftGUI.h"
#include "PartyListGUI.h"
#include "LogFloatGUI.h"
#include "GameGUIFacade.h"
#include "RequestPopupGUI.h"
#include "PartyStatusGUI.h"
#include "FocusAgentGUI.h"
#include "GamePauseGUI.h"
#include "MainMenuGUI.h"

#include "GizmoBoxRenderer.h"
#include "GizmoCylinderRenderer.h"
#include "GizmoSphereRenderer.h"

#include "ServerObjectMgr.h"
#include "GuideSystem.h"

using namespace udsdx;

static std::shared_ptr<udsdx::Mesh> CreateMeshFromHeightMap(const HeightMap* heightMap, LONG segmentWidth, LONG segmentHeight, float heightScale)
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
            // clockwise order
			indices.emplace_back(y * (segmentWidth + 1) + x);
            indices.emplace_back((y + 1) * (segmentWidth + 1) + x);
			indices.emplace_back(y * (segmentWidth + 1) + x + 1);
			indices.emplace_back((y + 1) * (segmentWidth + 1) + x + 1);
			indices.emplace_back(y * (segmentWidth + 1) + x + 1);
            indices.emplace_back((y + 1) * (segmentWidth + 1) + x);
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

GameScene::GameScene(HeightMap* heightMap, TerrainData* terrainData, TerrainDetail* terrainDetail) : Scene()
{
    auto res = INSTANCE(Resource);
    auto shader = res->Load<Shader>(RESOURCE_PATH(L"color.hlsl"));
    auto shaderTerrain = res->Load<Shader>(RESOURCE_PATH(L"terrain.hlsl"));

    m_playerMaterial = std::make_shared<udsdx::Material>();
    m_playerMaterial->SetSourceTexture(res->Load<udsdx::Texture>(RESOURCE_PATH(L"Sprite-0001.png")));

    m_terrainMaterial = std::make_shared<udsdx::Material>();
    m_terrainMaterial->SetSourceTexture(res->Load<udsdx::Texture>(RESOURCE_PATH(L"environment\\Maps\\TerrainSplatmap_0.tga")), 0);
    m_terrainMaterial->SetSourceTexture(res->Load<udsdx::Texture>(RESOURCE_PATH(L"environment\\Maps\\TerrainSplatmap_1.tga")), 1);
    m_terrainMaterial->SetSourceTexture(res->Load<udsdx::Texture>(RESOURCE_PATH(L"environment\\Maps\\TerrainSrc_0.png")), 2);
    m_terrainMaterial->SetSourceTexture(res->Load<udsdx::Texture>(RESOURCE_PATH(L"environment\\Maps\\TerrainSrc_1.tga")), 3);
    m_terrainMaterial->SetSourceTexture(res->Load<udsdx::Texture>(RESOURCE_PATH(L"environment\\Maps\\TerrainSrc_2.png")), 4);
    m_terrainMaterial->SetSourceTexture(res->Load<udsdx::Texture>(RESOURCE_PATH(L"environment\\Maps\\TerrainSrc_3.png")), 5);
    m_terrainMaterial->SetSourceTexture(res->Load<udsdx::Texture>(RESOURCE_PATH(L"environment\\Maps\\T_ground_soil_01_BC_SM.tga")), 6);
    m_terrainMaterial->SetSourceTexture(res->Load<udsdx::Texture>(RESOURCE_PATH(L"environment\\Maps\\T_ground_moss_01_BC_SM.tga")), 7);
    m_terrainMaterial->SetSourceTexture(res->Load<udsdx::Texture>(RESOURCE_PATH(L"environment\\Maps\\Grass_and_Clover.tif")), 8);
    m_terrainMaterial->SetSourceTexture(res->Load<udsdx::Texture>(RESOURCE_PATH(L"environment\\Maps\\TerrainNorm_0.png")), 9);
    m_terrainMaterial->SetSourceTexture(res->Load<udsdx::Texture>(RESOURCE_PATH(L"environment\\Maps\\TerrainNorm_1.png")), 10);
    m_terrainMaterial->SetSourceTexture(res->Load<udsdx::Texture>(RESOURCE_PATH(L"environment\\Maps\\TerrainNorm_2.png")), 11);
    m_terrainMaterial->SetSourceTexture(res->Load<udsdx::Texture>(RESOURCE_PATH(L"environment\\Maps\\TerrainNorm_3.png")), 12);
    m_terrainMaterial->SetSourceTexture(res->Load<udsdx::Texture>(RESOURCE_PATH(L"environment\\Maps\\T_ground_soil_01_N.png")), 13);
    m_terrainMaterial->SetSourceTexture(res->Load<udsdx::Texture>(RESOURCE_PATH(L"environment\\Maps\\TerrainNorm_5.png")), 14);
    m_terrainMaterial->SetSourceTexture(res->Load<udsdx::Texture>(RESOURCE_PATH(L"environment\\Maps\\TerrainNorm_6.tif")), 15);

    m_heroObj = std::make_shared<SceneObject>();

    ServerObjectMgr::GetInst()->SetMainHero(NetMgr(NetworkMgr)->GetSessionID(), m_heroObj);
    auto heroServerComponent = m_heroObj->AddComponent<ServerObject>();
   
    heroServerComponent->AddComp<MovePacketSender>();
    m_heroComponent = m_heroObj->AddComponent<AuthenticPlayer>();

    Vector3 temp = Vector3(-4.345f, 76.17f, 0.0f);
    auto& cell = heroServerComponent->m_pNaviAgent->GetCurCell();
    cell = NAVIGATION->GetNavMesh(NAVI_MESH_NUM::NUM_0)->GetNaviCell(temp);

    m_heroObj->GetTransform()->SetLocalPosition(temp);

    m_playerLightObj = std::make_shared<SceneObject>();
    auto playerLight = m_playerLightObj->AddComponent<LightDirectional>();
    Vector3 n = Vector3::Transform(Vector3::Up, Quaternion::CreateFromAxisAngle(Vector3(1.0f, 0.0f, -1.0f), 75.0f - 105.0f * 0.5f));
    m_playerLightObj->GetTransform()->SetLocalRotation(Quaternion::CreateFromYawPitchRoll(-PIDIV4, PIDIV4, 0) * Quaternion::CreateFromAxisAngle(n, 0.0f));

    AddObject(m_playerLightObj);

    m_terrainMesh = CreateMeshFromHeightMap(heightMap, 512, 512, 1.0f);
    m_terrainMesh->UploadBuffers(INSTANCE(Core)->GetDevice(), INSTANCE(Core)->GetCommandList());

    const float TerrainSize = GET_DATA(float, "TerrainSize", "Value");
    const Vector3 terrainPos = Vector3(-TerrainSize * 0.5f, 0, -TerrainSize * 0.5f);
    const Vector3 terrainScale = Vector3::One * TerrainSize;

    {
        m_terrainDetailMaterial = std::make_shared<udsdx::Material>();
        m_terrainDetailMaterial->SetSourceTexture(res->Load<udsdx::Texture>(RESOURCE_PATH(L"environment\\Grass.tga")), 0);

        std::shared_ptr<SceneObject> terrainDetailObj = std::make_shared<SceneObject>();
        auto terrainDetailRenderer = terrainDetailObj->AddComponent<TerrainDetailRenderer>();
        terrainDetailRenderer->SetTerrainDetail(terrainDetail);
        terrainDetailRenderer->SetShader(res->Load<udsdx::Shader>(RESOURCE_PATH(L"detailbillboard.hlsl")));
        terrainDetailRenderer->SetMaterial(m_terrainDetailMaterial.get());

        terrainDetailObj->GetTransform()->SetLocalPosition(terrainPos);
        terrainDetailObj->GetTransform()->SetLocalScale(terrainScale);

        AddObject(terrainDetailObj);
    }


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
            std::string aKey = subMeshes[i].DiffuseTexturePath;
            std::string nKey = subMeshes[i].NormalTexturePath;
            std::transform(aKey.begin(), aKey.end(), aKey.begin(), ::tolower);
            std::transform(nKey.begin(), nKey.end(), nKey.begin(), ::tolower);

            // if the extension is .psd, replace it with .tga
            if (aKey.ends_with(".psd"))
                aKey.replace(aKey.end() - 4, aKey.end(), ".tga");
            if (nKey.ends_with(".psd"))
                nKey.replace(nKey.end() - 4, nKey.end(), ".tga");

            if (auto it = textureMap.find(aKey); it != textureMap.end())
                material->SetSourceTexture(it->second);
            else
                material->SetSourceTexture(res->Load<udsdx::Texture>(RESOURCE_PATH(L"Sprite-0001.png")));
            if (auto it = textureMap.find(nKey); it != textureMap.end())
                material->SetSourceTexture(it->second, 1);
			else
				material->SetSourceTexture(res->Load<udsdx::Texture>(RESOURCE_PATH(L"Sprite-0001.png")), 1);

            m_instanceMaterials.emplace_back(material);
            terrainInstanceRenderer->SetMaterial(material.get(), static_cast<int>(i));
        }

        terrainInstanceRenderer->SetTerrainData(terrainData, filename);
        terrainInstanceRenderer->SetMesh(mesh);
        terrainInstanceRenderer->SetShader(res->Load<udsdx::Shader>(RESOURCE_PATH(L"colorinstanced.hlsl")));

        AddObject(terrainInstance);
    }

    {
        m_terrainObj = std::make_shared<SceneObject>();
        m_terrainObj->GetTransform()->SetLocalPosition(terrainPos);
        m_terrainObj->GetTransform()->SetLocalScale(terrainScale);
        auto terrainRenderer = m_terrainObj->AddComponent<MeshRenderer>();
        terrainRenderer->SetMesh(m_terrainMesh.get());
        terrainRenderer->SetMaterial(m_terrainMaterial.get());
        terrainRenderer->SetShader(shaderTerrain);

        AddObject(m_terrainObj);
    }

    {
        auto skyboxObj = std::make_shared<SceneObject>();
        auto skyboxRenderer = skyboxObj->AddComponent<InlineMeshRenderer>();
        skyboxRenderer->SetShader(res->Load<Shader>(RESOURCE_PATH(L"skybox.hlsl")));
        skyboxRenderer->SetVertexCount(6);
        skyboxRenderer->SetCastShadow(false);

        auto skyboxTexture = res->Load<udsdx::Texture>(RESOURCE_PATH(L"Skybox.jpg"));
        m_skyboxMaterial = std::make_shared<udsdx::Material>();
        m_skyboxMaterial->SetSourceTexture(skyboxTexture);
        skyboxRenderer->SetMaterial(m_skyboxMaterial.get());

        AddObject(skyboxObj);
    }

    {
        m_playerInterfaceGroup = std::make_shared<SceneObject>();

        m_focusAgentObj = std::make_shared<SceneObject>();
        auto focusAgent = m_focusAgentObj->AddComponent<FocusAgentGUI>();

        focusAgent->SetSize(Vector2::One * 8192.0f);
        focusAgent->SetTryExitCallback([this]() {
            m_pauseMenuObj->GetComponent<GamePauseGUI>()->ToggleActivePanel();
            });
        focusAgent->SetTryClickCallback([this]() {
            m_heroComponent->TryClickScreen();
            });
        AddObject(m_focusAgentObj);
        m_focusAgentObj->SetActive(false);

        auto guiObj = std::make_shared<SceneObject>();
        auto guiRenderer = guiObj->AddComponent<PlayerStatusGUI>();
        m_playerInterfaceGroup->AddChild(guiObj);
        m_heroComponent->SetPlayerStatusGUI(guiRenderer);

        auto quickSlotObj = std::make_shared<SceneObject>();
        auto quickSlotRenderer = quickSlotObj->AddComponent<PlayerQuickSlotGUI>();
        m_playerInterfaceGroup->AddChild(quickSlotObj);
        m_heroComponent->SetPlayerQuickSlotGUI(quickSlotRenderer);

        m_inventoryObj = std::make_shared<SceneObject>();
        auto inventoryRenderer = m_inventoryObj->AddComponent<PlayerInventoryGUI>();
        m_playerInterfaceGroup->AddChild(m_inventoryObj);
        m_heroComponent->SetPlayerInventoryGUI(inventoryRenderer);
		m_inventoryObj->SetActive(false);

        m_craftObj = std::make_shared<SceneObject>();
        auto craftComp = m_craftObj->AddComponent<PlayerCraftGUI>();
        m_playerInterfaceGroup->AddChild(m_craftObj);
        m_heroComponent->SetPlayerCraftGUI(craftComp);
		m_craftObj->SetActive(false);

        m_partyListObj = std::make_shared<SceneObject>();
        auto partyListComp = m_partyListObj->AddComponent<PartyListGUI>();
        m_playerInterfaceGroup->AddChild(m_partyListObj);
		m_partyListObj->SetActive(false);

        auto logFloatObj = std::make_shared<SceneObject>();
        auto logFloatComp = logFloatObj->AddComponent<LogFloatGUI>();
        logFloatComp->AddText(L"Welcome to the game!");
        m_playerInterfaceGroup->AddChild(logFloatObj);

        auto requestPopupObj = std::make_shared<SceneObject>();
        auto requestPopupComp = requestPopupObj->AddComponent<RequestPopupGUI>();
        m_playerInterfaceGroup->AddChild(requestPopupObj);

        auto partyStatusObj = std::make_shared<SceneObject>();
        auto partyStatusComp = partyStatusObj->AddComponent<PartyStatusGUI>();
        m_playerInterfaceGroup->AddChild(partyStatusObj);

        m_pauseMenuObj = std::make_shared<SceneObject>();
        auto pauseMenuComp = m_pauseMenuObj->AddComponent<GamePauseGUI>();
        pauseMenuComp->SetExitGameCallback([this]() { ExitGame(); });
        pauseMenuComp->SetTogglePauseCallback([this](bool isPaused) { OnTogglePause(isPaused); });
        AddObject(m_pauseMenuObj);

        auto mainMenuObj = std::make_shared<SceneObject>();
        auto mainMenuComp = mainMenuObj->AddComponent<MainMenuGUI>();
        mainMenuComp->SetEnterGameCallback([this]() { EnterGame(); });
        mainMenuComp->SetExitGameCallback([this]() { ExitGame(); });
        AddObject(mainMenuObj);

        INSTANCE(GameGUIFacade)->PartyList = partyListComp;
        INSTANCE(GameGUIFacade)->LogFloat = logFloatComp;
        INSTANCE(GameGUIFacade)->RequestPopup = requestPopupComp;
        INSTANCE(GameGUIFacade)->PartyStatus = partyStatusComp;
    }

    {
        auto textObj = std::make_shared<SceneObject>();
        auto textRenderer = textObj->AddComponent<GUIText>();
        textObj->GetTransform()->SetLocalPosition(Vector3(-640, 480, 0));
        textRenderer->SetText(GET_DATA(std::wstring, "Intro", "Start"));
        textRenderer->SetFont(res->Load<udsdx::Font>(RESOURCE_PATH(L"pretendard.spritefont")));
		textRenderer->SetAlignment(GUIText::Alignment::UpperLeft);

        m_playerInterfaceGroup->AddChild(textObj);
    }

    {
        m_mainMenuCameraObject = std::make_shared<SceneObject>();
        m_mainMenuCameraObject->GetTransform()->SetLocalPosition(Vector3(0.0f, 120.0f, 0.0f));
        m_mainMenuCameraObject->GetTransform()->SetLocalRotation(Quaternion::CreateFromYawPitchRoll(PIDIV4, PIDIV4, 0));

        auto camera = m_mainMenuCameraObject->AddComponent<CameraPerspective>();
		camera->SetFov(PI / 4.0f);
        camera->SetClipOffset(Vector2(1.0f / 3.0f, 0.0f));

		auto bezierMovement = m_mainMenuCameraObject->AddComponent<BezierMovement>();
		bezierMovement->LoadSpline(RESOURCE_PATH(L"environment\\CameraPathSpline.json"));
		bezierMovement->SetSpeed(16.0f);

        AddObject(m_mainMenuCameraObject);
    }

    if (false)
    {
        m_gizmoMaterial = std::make_shared<udsdx::Material>();
        m_gizmoMaterial->SetSourceTexture(res->Load<udsdx::Texture>(RESOURCE_PATH(L"Sprite-0001.png")));

        auto navMeshVisualizer = std::make_shared<SceneObject>();
        auto navMeshRenderer = navMeshVisualizer->AddComponent<MeshRenderer>();
        navMeshRenderer->SetMesh(res->Load<udsdx::Mesh>(RESOURCE_PATH(L"navmesh.obj")));
        navMeshRenderer->SetShader(res->Load<udsdx::Shader>(RESOURCE_PATH(L"color.hlsl")));
        navMeshRenderer->SetMaterial(m_gizmoMaterial.get());

        AddObject(navMeshVisualizer);
    }
}

void GameScene::Update(const Time& time)
{
    auto audioAction = [this](bool isOpen) {
        if (isOpen)
            m_menuSound = INSTANCE(Resource)->Load<udsdx::AudioClip>(RESOURCE_PATH(L"audio\\uiopen.wav"))->CreateInstance();
        else
            m_menuSound = INSTANCE(Resource)->Load<udsdx::AudioClip>(RESOURCE_PATH(L"audio\\uiclose.wav"))->CreateInstance();
        m_menuSound->SetVolume(0.5f);
        m_menuSound->Play();
        };

    if (INSTANCE(Input)->GetKeyDown(Keyboard::E))
    {
        m_inventoryObj->SetActive(!m_inventoryObj->GetActive());
        audioAction(m_inventoryObj->GetActive());
    }
    if (INSTANCE(Input)->GetKeyDown(Keyboard::C))
    {
        m_craftObj->SetActive(!m_craftObj->GetActive());
        audioAction(m_craftObj->GetActive());
    }
	if (INSTANCE(Input)->GetKeyDown(Keyboard::Q))
	{
		m_partyListObj->SetActive(!m_partyListObj->GetActive());
		audioAction(m_partyListObj->GetActive());
	}

    Scene::Update(time);
}

void GameScene::EnterGame()
{   
    AddObject(m_heroObj);
    AddObject(m_playerInterfaceGroup);
    m_focusAgentObj->SetActive(true);
    m_mainMenuCameraObject->SetActive(false);

    {
        extern std::shared_ptr<GameScene> g_scene;
        GuideSystem::GetInst()->SetMainPlayer(m_heroObj);
        GuideSystem::GetInst()->SetTargetScene(g_scene);
    }
    if constexpr (true == g_bUseNetWork)
    {
        Send(Create_c2s_ENTER(ToFlatVec3(m_heroObj->GetTransform()->GetLocalPosition())));
    }
}

void GameScene::ExitGame()
{   
    // TODO: 일단 끄기전 서버오브젝트 컨테이너 밀어줌
    ServerObjectMgr::GetInst()->Clear();
    // TODO: 이거해야 메모리릭 없는데 왜 터짐?
    // NAVIGATION->GetNavMesh(NAVI_MESH_NUM::NUM_0)->FreeNavMeshQuery();
    UpdownStudio::Quit();
}

void GameScene::OnTogglePause(bool isPaused)
{
    m_heroObj->GetComponent<InputHandler>()->SetActive(!isPaused);
    m_playerInterfaceGroup->SetActive(!isPaused);
}
