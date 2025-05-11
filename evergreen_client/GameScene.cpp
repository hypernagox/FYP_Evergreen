#include "pch.h"
#include "GameScene.h"

#include <HeightMap.h>
#include "TerrainData.h"
#include "TerrainInstanceRenderer.h"
#include "AuthenticPlayer.h"
#include "SpectatorPlayer.h"
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
#include "EntityInteraction.h"
#include "InteractiveEntity.h"

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
#include "PlayerTagGUI.h"
#include "InteractionFloatGUI.h"

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
    m_heroComponent = m_heroObj->AddComponent<AuthenticPlayer>();
    m_heroComponent->SetHeightMap(heightMap);
    auto entityInteraction = m_heroObj->AddComponent<EntityInteraction>();
	entityInteraction->SetTargetScene(this);
    auto heroServerComponent = m_heroObj->GetComponent<ServerObject>();
    heroServerComponent->AddComp<MovePacketSender>();

    Vector3 start_pos = Vector3{
        -315.8432f,
        84.93234f,
        -33.050846f
    };
    auto& cell = heroServerComponent->m_pNaviAgent->GetCurCell();
    cell = NAVIGATION->GetNavMesh(NAVI_MESH_NUM::NUM_0)->GetNaviCell(start_pos);

	m_activeObjectGroup = std::make_shared<SceneObject>();

    m_heroObj->GetTransform()->SetLocalPosition(start_pos);

    m_spectatorObj = std::make_shared<SceneObject>();
    m_spectatorObj->AddComponent<SpectatorPlayer>();
    
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

    {
        std::map<std::string, udsdx::Texture*> textureMap;
        for (auto texture : INSTANCE(Resource)->LoadAll<udsdx::Texture>())
            textureMap[texture->GetName().data()] = texture;
        std::map<std::string, std::filesystem::path> prefabMap;

        std::shared_ptr<SceneObject> treeObj = std::make_shared<SceneObject>();
        auto treeRenderer = treeObj->AddComponent<MeshRenderer>();

        treeObj->GetTransform()->SetLocalScale(Vector3::One * 0.01f);
        treeObj->GetTransform()->SetLocalPosition(Vector3(-42.968254f, 74.610634f, -87.984f));
        treeObj->SetActive(true);

        treeRenderer->SetMesh(res->Load<udsdx::Mesh>(RESOURCE_PATH(L"goldentree\\tree.fbx")));
        treeRenderer->SetShader(res->Load<udsdx::Shader>(RESOURCE_PATH(L"color.hlsl")));

        {
            std::shared_ptr<udsdx::Material> treeMaterial = std::make_shared<udsdx::Material>();
            treeMaterial->SetSourceTexture(res->Load<udsdx::Texture>(RESOURCE_PATH(L"goldentree\\leaves_color.png")), 0);
            treeMaterial->SetSourceTexture(res->Load<udsdx::Texture>(RESOURCE_PATH(L"goldentree\\leaves_nm.png")), 1);

            treeRenderer->SetMaterial(treeMaterial.get(), 0);
            m_harvestMaterials.emplace_back(treeMaterial);
        }

        {
            std::shared_ptr<udsdx::Material> treeMaterial = std::make_shared<udsdx::Material>();
            treeMaterial->SetSourceTexture(res->Load<udsdx::Texture>(RESOURCE_PATH(L"goldentree\\trunk_Base_color.png")), 0);
            treeMaterial->SetSourceTexture(res->Load<udsdx::Texture>(RESOURCE_PATH(L"goldentree\\trunk_Normal_OpenGL.png")), 1);

            treeRenderer->SetMaterial(treeMaterial.get(), 1);
            m_harvestMaterials.emplace_back(treeMaterial);
        }

        {
            auto interactiveEntity = treeObj->AddComponent<InteractiveEntity>();
            interactiveEntity->SetInteractionText(L"보상 얻기");
            interactiveEntity->SetInteractionCallback([]() { Send(Create_c2s_CHANGE_HARVEST_STATE()); });
        }

        AddActiveObject(treeObj);
        GuideSystem::GetInst()->AddHarvestMeshObject(treeObj);

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
            if (terrainData->GetPrototypeInstanceCount(filename) > 0)
                AddTerrainInstances(directory.path(), textureMap, terrainData);
            prefabMap[filename] = directory.path();
        }

        std::ifstream file(RESOURCE_PATH(L"environment\\ExportedGameSpawns.json"));
        nlohmann::json j;
        file >> j;
        for (auto& prototype : j)
        {
            std::string filename = prototype["prefab"];
            if (prefabMap.find(filename) != prefabMap.end())
                AddHarvestObjects(prefabMap[filename], textureMap, prototype);
        }
    }

    {
        m_terrainObj = std::make_shared<SceneObject>();
        m_terrainObj->GetTransform()->SetLocalPosition(terrainPos);
        m_terrainObj->GetTransform()->SetLocalScale(terrainScale);
        auto terrainRenderer = m_terrainObj->AddComponent<MeshRenderer>();
        terrainRenderer->SetMesh(m_terrainMesh.get());
        terrainRenderer->SetMaterial(m_terrainMaterial.get());
        terrainRenderer->SetShader(res->Load<Shader>(RESOURCE_PATH(L"terrain.hlsl")));

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

		auto EntityInteractionObj = std::make_shared<SceneObject>();
		auto interactionFloatGUI = EntityInteractionObj->AddComponent<InteractionFloatGUI>();
		entityInteraction->SetInteractionFloatGUI(interactionFloatGUI);
		m_playerInterfaceGroup->AddChild(EntityInteractionObj);

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

        m_playerTagObj = std::make_shared<SceneObject>();
        auto playerTagRenderer = m_playerTagObj->AddComponent<PlayerTagGUI>();

        m_playerInterfaceGroup->AddChild(m_playerTagObj);
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

    if (INSTANCE(Input)->GetKeyDown(Keyboard::I))
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
    if (INSTANCE(Input)->GetKeyDown(Keyboard::Tab))
    {
        OnTogglePlayerMode(!m_bSpectatorMode);
	}

    m_playerTagObj->GetComponent<PlayerTagGUI>()->SetTargetPosition(m_heroObj->GetTransform()->GetWorldPosition() + Vector3::Up * 1.8f);

    Scene::Update(time);
}

void GameScene::EnterGame()
{
    AddObject(m_heroObj);
    AddObject(m_spectatorObj);
    AddObject(m_activeObjectGroup);
    AddObject(m_playerInterfaceGroup);
    m_focusAgentObj->SetActive(true);
    m_mainMenuCameraObject->SetActive(false);

    OnTogglePause(false);
    OnTogglePlayerMode(false);

    {
        extern std::shared_ptr<GameScene> g_scene;
        GuideSystem::GetInst()->SetMainPlayer(m_heroObj);
        GuideSystem::GetInst()->SetTargetScene(g_scene);
    }

    // 만약 여기서 와일루프 돌면, 서버로부터 아직 ID발급을 못받았다는 이야기
    NetMgr(NetworkMgr)->ProcessLogin();

    // 서버오브젝트 매니저는 뭐 테스트 할 때 필요 할수도있음..
    ServerObjectMgr::GetInst()->SetMainHero(NetMgr(NetworkMgr)->GetSessionID(), m_heroObj);

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
    m_heroObj->GetComponent<InputHandler>()->SetActive(!isPaused && !m_bSpectatorMode);
    m_spectatorObj->GetComponent<InputHandler>()->SetActive(!isPaused && m_bSpectatorMode);
    m_playerInterfaceGroup->SetActive(!isPaused);
}

void GameScene::OnTogglePlayerMode(bool spectatorMode)
{
    m_bSpectatorMode = spectatorMode;
    m_heroObj->GetComponent<InputHandler>()->SetActive(!spectatorMode);
    m_spectatorObj->GetComponent<InputHandler>()->SetActive(spectatorMode);
    auto heroCamera = m_heroObj->GetComponent<AuthenticPlayer>()->GetCameraComponent();
    auto spectatorCamera = m_spectatorObj->GetComponent<SpectatorPlayer>()->GetCameraComponent();
    heroCamera->SetActive(!spectatorMode);
    spectatorCamera->SetActive(spectatorMode);

    if (spectatorMode)
    {
        m_spectatorObj->GetTransform()->SetLocalPosition(heroCamera->GetTransform()->GetWorldPosition());
        m_spectatorObj->GetTransform()->SetLocalRotation(heroCamera->GetTransform()->GetWorldRotation());
    }
}

void GameScene::AddActiveObject(const std::shared_ptr<udsdx::SceneObject>& obj)
{
	m_activeObjectGroup->AddChild(obj);
}

std::vector<InteractiveEntity*> GameScene::GetInteractiveEntities() const
{
	return m_activeObjectGroup->GetComponentsInChildren<InteractiveEntity>();
}

Camera* GameScene::GetMainCamera() const
{
    if (m_bSpectatorMode)
		return m_spectatorObj->GetComponent<SpectatorPlayer>()->GetCameraComponent();
	else
		return m_heroObj->GetComponent<AuthenticPlayer>()->GetCameraComponent();
}

void GameScene::AddTerrainInstances(std::filesystem::path path, const std::map<std::string, udsdx::Texture*>& textureMap, TerrainData* terrainData)
{
    auto terrainInstance = std::make_shared<SceneObject>();
    auto terrainInstanceRenderer = terrainInstance->AddComponent<TerrainInstanceRenderer>();
    auto mesh = INSTANCE(Resource)->Load<udsdx::Mesh>(path.c_str());

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
            material->SetSourceTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"Sprite-0001.png")));
        if (auto it = textureMap.find(nKey); it != textureMap.end())
            material->SetSourceTexture(it->second, 1);
        else
            material->SetSourceTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"Sprite-0001.png")), 1);

        m_instanceMaterials.emplace_back(material);
        terrainInstanceRenderer->SetMaterial(material.get(), static_cast<int>(i));
    }

    terrainInstanceRenderer->SetTerrainData(terrainData, path.filename().stem().string());
    terrainInstanceRenderer->SetMesh(mesh);
    terrainInstanceRenderer->SetShader(INSTANCE(Resource)->Load<udsdx::Shader>(RESOURCE_PATH(L"colorinstanced.hlsl")));

    AddObject(terrainInstance);
}

void GameScene::AddHarvestObjects(std::filesystem::path path, const std::map<std::string, udsdx::Texture*>& textureMap, const nlohmann::json& prototype)
{
    const float terrainScale = 1.0f;
    const float instanceScale = 0.01f;

    auto mesh = INSTANCE(Resource)->Load<udsdx::Mesh>(path.c_str());
    std::vector<std::shared_ptr<udsdx::Material>> materials;

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
            material->SetSourceTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"Sprite-0001.png")));
        if (auto it = textureMap.find(nKey); it != textureMap.end())
            material->SetSourceTexture(it->second, 1);
        else
            material->SetSourceTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"Sprite-0001.png")), 1);

        m_instanceMaterials.emplace_back(material);
        materials.emplace_back(material);
    }

    for (auto& instance : prototype["instances"])
    {
        Vector3 position = Vector3(instance["position"]["x"], instance["position"]["y"], instance["position"]["z"]);
        Quaternion rotation = Quaternion(instance["rotation"]["x"], instance["rotation"]["y"], instance["rotation"]["z"], instance["rotation"]["w"]);
        Vector3 scale = Vector3(instance["scale"]["x"], instance["scale"]["y"], instance["scale"]["z"]);
        scale *= Vector3(-1.0f, 1.0f, -1.0f) * instanceScale;
        position *= terrainScale;

        std::shared_ptr<SceneObject> harvestObj = std::make_shared<SceneObject>();
        auto harvestRenderer = harvestObj->AddComponent<MeshRenderer>();
        harvestRenderer->SetMesh(mesh);
        harvestRenderer->SetShader(INSTANCE(Resource)->Load<udsdx::Shader>(RESOURCE_PATH(L"color.hlsl")));
        for (size_t i = 0; i < materials.size(); ++i)
			harvestRenderer->SetMaterial(materials[i].get(), static_cast<int>(i));

        auto interactiveEntity = harvestObj->AddComponent<InteractiveEntity>();
        interactiveEntity->SetInteractionText(L"채집하기");
        interactiveEntity->SetInteractionCallback([]() { Send(Create_c2s_CHANGE_HARVEST_STATE()); });

        harvestObj->GetTransform()->SetLocalPosition(position);
        harvestObj->GetTransform()->SetLocalRotation(rotation);
        harvestObj->GetTransform()->SetLocalScale(scale);
        harvestObj->SetActive(false);

        AddActiveObject(harvestObj);
        GuideSystem::GetInst()->AddHarvestMeshObject(harvestObj);
    }
}