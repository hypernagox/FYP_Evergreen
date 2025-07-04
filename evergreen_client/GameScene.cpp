#include "pch.h"
#include "GameScene.h"

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
#include "EntityInteraction.h"
#include "InteractiveEntity.h"

#include "PlayerStatusGUI.h"
#include "PlayerQuickSlotGUI.h"
#include "PlayerInventoryGUI.h"
#include "PlayerCraftGUI.h"
#include "QuestGUI.h"
#include "LogFloatGUI.h"
#include "GameGUIFacade.h"
#include "RequestPopupGUI.h"
#include "PartyStatusGUI.h"
#include "FocusAgentGUI.h"
#include "GamePauseGUI.h"
#include "PlayerTagGUI.h"
#include "InteractionFloatGUI.h"
#include "DamageCountGUI.h"
#include "ChannelSwitchGUI.h"
#include "TransitionOverlayGUI.h"

#include "GizmoBoxRenderer.h"
#include "GizmoCylinderRenderer.h"
#include "GizmoSphereRenderer.h"

#include "ServerObjectMgr.h"
#include "GuideSystem.h"
#include "TutorialUI.h"
#include "MinimapRenderer.h"
#include "EnvironmentRenderer.h"
#include "PopupGUIManager.h"
#include "MainScene.h"

using namespace udsdx;

extern EnvironmentParameters g_defaultEnvironmentParam;
extern EnvironmentParameters g_dungeonEnvironmentParam;

GameScene::GameScene() : Scene()
{
}

void GameScene::OnAttach()
{
    auto res = INSTANCE(Resource);
    auto shader = res->Load<Shader>(RESOURCE_PATH(L"color.hlsl"));

    m_playerMaterial = std::make_shared<udsdx::Material>();
    m_playerMaterial->SetSourceTexture(res->Load<udsdx::Texture>(RESOURCE_PATH(L"Sprite-0001.png")));

    m_heroObj = std::make_shared<SceneObject>();
    m_heroComponent = m_heroObj->AddComponent<AuthenticPlayer>();
    m_heroComponent->SetHeightMap(g_defaultEnvironmentParam.HeightMap);
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

#pragma region Environment Initialization
    m_environmentLightObj = std::make_shared<SceneObject>();
    auto playerLight = m_environmentLightObj->AddComponent<LightDirectional>();
    Vector3 n = Vector3::Transform(Vector3::Up, Quaternion::CreateFromAxisAngle(Vector3(1.0f, 0.0f, -1.0f), 75.0f - 105.0f * 0.5f));
    m_environmentLightObj->GetTransform()->SetLocalRotation(Quaternion::CreateFromYawPitchRoll(-PIDIV4, PIDIV4, 0) * Quaternion::CreateFromAxisAngle(n, 0.0f));

    AddObject(m_environmentLightObj);

    {
        m_craftTableObj = std::make_shared<SceneObject>();
        m_craftTableObj->GetTransform()->SetLocalPosition(Vector3(-123.22470092773438f, 75.68199920654297f, 16.593002319335939));
        m_craftTableObj->GetTransform()->SetLocalRotation(Quaternion(0.0f, 1.0f, 0.0f, 0.0f));
        m_craftTableObj->GetTransform()->SetLocalScale(Vector3(-1.0f, 1.0f, -1.0f) * 0.01f);

        m_craftTableMaterial = std::make_shared<udsdx::Material>();
        m_craftTableMaterial->SetSourceTexture(res->Load<udsdx::Texture>(RESOURCE_PATH(L"environment\\Maps\\M_Kit_1\\M_Kit_1_D.tga")), 0);
        m_craftTableMaterial->SetSourceTexture(res->Load<udsdx::Texture>(RESOURCE_PATH(L"environment\\Maps\\M_Kit_1\\M_Kit_1_N.tga")), 1);

        auto craftTableRenderer = m_craftTableObj->AddComponent<MeshRenderer>();
        craftTableRenderer->SetMesh(res->Load<udsdx::Mesh>(RESOURCE_PATH(L"environment\\Village\\O_Table_B.yms")));
        craftTableRenderer->SetShader(res->Load<udsdx::Shader>(RESOURCE_PATH(L"color.hlsl")));
        craftTableRenderer->SetMaterial(m_craftTableMaterial.get(), 0);

        auto interactiveEntity = m_craftTableObj->AddComponent<InteractiveEntity>();
        interactiveEntity->SetInteractionText(L"제작하기");
        interactiveEntity->SetInteractionCallback([this]() { m_popupGUIManager->Append(m_craftObj); });

        AddActiveObject(m_craftTableObj);
    }

    {
        std::shared_ptr<SceneObject> treeObj = std::make_shared<SceneObject>();
        auto treeRenderer = treeObj->AddComponent<MeshRenderer>();

        treeObj->GetTransform()->SetLocalScale(Vector3::One * 0.01f);
        treeObj->GetTransform()->SetLocalPosition(Vector3(-42.968254f, 74.610634f, -87.984f));
        treeObj->SetActive(true);

        treeRenderer->SetMesh(res->Load<udsdx::Mesh>(RESOURCE_PATH(L"goldentree\\tree.yms")));
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

        m_defaultEnvironmentObject = std::make_shared<SceneObject>();
        auto environmentRenderer = m_defaultEnvironmentObject->AddComponent<EnvironmentRenderer>();
        environmentRenderer->Initialize(g_defaultEnvironmentParam);
        AddObject(m_defaultEnvironmentObject);

        m_dungeonEnvironmentObject = std::make_shared<SceneObject>();
        auto dungeonEnvironmentRenderer = m_dungeonEnvironmentObject->AddComponent<EnvironmentRenderer>();
        dungeonEnvironmentRenderer->Initialize(g_dungeonEnvironmentParam);
        AddObject(m_dungeonEnvironmentObject);

        std::ifstream file(RESOURCE_PATH(L"environment\\ExportedGameSpawns.json"));
        nlohmann::json j;
        file >> j;
        for (auto& prototype : j)
        {
            for (auto& instance : prototype["instances"])
            {
                auto harvestObject = environmentRenderer->AddHarvestObject(instance);
                AddActiveObject(harvestObject);
                GuideSystem::GetInst()->AddHarvestMeshObject(harvestObject);
            }
        }

        GuideSystem::GetInst()->AddHarvestMeshObject(treeObj);

        m_minimapRenderer = std::make_unique<MinimapRenderer>(INSTANCE(Core)->GetDevice(), 256, 256);
        m_minimapRenderer->SetMinimapMesh(environmentRenderer->GetTerrainMesh());
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
#pragma endregion

#pragma region GUI Initialization
    {
        m_interfaceGroup = std::make_shared<SceneObject>();
        m_playerInterfaceGroup = std::make_shared<SceneObject>();

        AddObject(m_interfaceGroup);
        m_interfaceGroup->AddChild(m_playerInterfaceGroup);
        m_playerInterfaceGroup->SetActive(false);

        auto textObj = std::make_shared<SceneObject>();
        auto textRenderer = textObj->AddComponent<GUIText>();
        textObj->GetTransform()->SetLocalPosition(Vector3(-640, 480, 0));
        textRenderer->SetText(GET_DATA(std::wstring, "Script", "Intro", "Start"));
        textRenderer->SetFont(res->Load<udsdx::Font>(RESOURCE_PATH(L"pretendard.spritefont")));
        textRenderer->SetAlignment(GUIText::Alignment::UpperLeft);
        m_playerInterfaceGroup->AddChild(textObj);

        auto damageCountObj = std::make_shared<SceneObject>();
        auto damageCountRenderer = damageCountObj->AddComponent<DamageCountGUI>();
        m_playerInterfaceGroup->AddChild(damageCountObj);

        m_playerTagObj = std::make_shared<SceneObject>();
        auto playerTagRenderer = m_playerTagObj->AddComponent<PlayerTagGUI>();
        m_playerInterfaceGroup->AddChild(m_playerTagObj);

        m_focusAgentObj = std::make_shared<SceneObject>();
        auto focusAgent = m_focusAgentObj->AddComponent<FocusAgentGUI>();

        focusAgent->SetSize(Vector2::One * 8192.0f);
        focusAgent->SetTryClickCallback([this]() {
            m_heroComponent->TryClickScreen();
            });
        m_playerInterfaceGroup->AddChild(m_focusAgentObj);
        m_focusAgentObj->SetActive(false);

        m_popupGUIManager = m_interfaceGroup->AddComponent<PopupGUIManager>();
        auto popupInputHandler = m_popupGUIManager->AddComponent<InputHandler>();
        popupInputHandler->AddKeyFunc(Keyboard::Escape, KET_TAP, [this]() { m_popupGUIManager->Pop(); });

        auto minimapObj = std::make_shared<SceneObject>();
        auto minimapImage = minimapObj->AddComponent<GUIImage>();
        minimapImage->SetTexture(m_minimapRenderer->GetRenderTargetTexture(), true);
        minimapObj->GetTransform()->SetLocalPosition(Vector3(-300.0f, -300.0f, 0.0f));

        {
            auto minimapBackground = std::make_shared<SceneObject>();
            auto minimapBackgroundRenderer = minimapBackground->AddComponent<GUIImage>();
            minimapBackgroundRenderer->SetTexture(res->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\minimap_outline_gradation.png")));
            minimapBackgroundRenderer->SetSize(Vector2(360.0f, 360.0f));

            auto minimapMarkerObj = std::make_shared<SceneObject>();
            minimapMarkerObj->GetTransform()->SetLocalPosition(Vector3(0.0f, 16.0f, 0.0f));
            auto minimapMarkerRenderer = minimapMarkerObj->AddComponent<GUIImage>();
            minimapMarkerRenderer->SetTexture(res->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\minimap_marker.png")));
            minimapMarkerRenderer->SetSize(Vector2(32.0f, 32.0f));

            minimapObj->AddChild(minimapBackground);
            minimapObj->AddChild(minimapMarkerObj);
        }

        m_playerInterfaceGroup->AddChild(minimapObj);

        auto guiObj = std::make_shared<SceneObject>();
        auto guiRenderer = guiObj->AddComponent<PlayerStatusGUI>();
        m_playerInterfaceGroup->AddChild(guiObj);
        m_heroComponent->SetPlayerStatusGUI(guiRenderer);

        auto quickSlotObj = std::make_shared<SceneObject>();
        auto quickSlotRenderer = quickSlotObj->AddComponent<PlayerQuickSlotGUI>();
        m_playerInterfaceGroup->AddChild(quickSlotObj);
        m_heroComponent->SetPlayerQuickSlotGUI(quickSlotRenderer);

        auto logFloatObj = std::make_shared<SceneObject>();
        auto logFloatComp = logFloatObj->AddComponent<LogFloatGUI>();
        logFloatComp->AddText(L"Welcome to the game!");
        m_playerInterfaceGroup->AddChild(logFloatObj);

        m_tutorialObj = std::make_shared<SceneObject>();
        m_tutorialObj->AddComponent<TutorialUI>();
        m_tutorialObj->SetActive(false);
        m_playerInterfaceGroup->AddChild(m_tutorialObj);

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
        auto questGUIComp = m_partyListObj->AddComponent<QuestGUI>();
        m_playerInterfaceGroup->AddChild(m_partyListObj);
        m_partyListObj->SetActive(false);

        auto partyStatusObj = std::make_shared<SceneObject>();
        auto partyStatusComp = partyStatusObj->AddComponent<PartyStatusGUI>();
        m_playerInterfaceGroup->AddChild(partyStatusObj);

        auto EntityInteractionObj = std::make_shared<SceneObject>();
        auto interactionFloatGUI = EntityInteractionObj->AddComponent<InteractionFloatGUI>();
        entityInteraction->SetInteractionFloatGUI(interactionFloatGUI);
        m_playerInterfaceGroup->AddChild(EntityInteractionObj);

        m_pauseMenuObj = std::make_shared<SceneObject>();
        m_pauseMenuObj->SetActive(false);
        auto pauseMenuComp = m_pauseMenuObj->AddComponent<GamePauseGUI>();
        pauseMenuComp->SetExitGameCallback([this]() { ExitGame(); });
        m_interfaceGroup->AddChild(m_pauseMenuObj);

        m_channelSwitchObj = std::make_shared<SceneObject>();
        auto channelSwitchComp = m_channelSwitchObj->AddComponent<ChannelSwitchGUI>();
        channelSwitchComp->SetPanelGraphic(false);
        channelSwitchComp->SetChannelSelectedCallback([this](int channelID) {
            INSTANCE(GameGUIFacade)->TransitionOverlay->AppendTransition([this, channelID]() {
                m_popupGUIManager->PopAll();
                Send(Create_c2s_CHANGE_CHANNEL(channelID));
                },
                std::format(L"채널 변경 중 ...")
            );
            });
        m_channelSwitchObj->SetActive(false);
        pauseMenuComp->SetChannelSwitchGUI(m_channelSwitchObj);
        m_interfaceGroup->AddChild(m_channelSwitchObj);

        auto requestPopupObj = std::make_shared<SceneObject>();
        auto requestPopupComp = requestPopupObj->AddComponent<RequestPopupGUI>();
        m_interfaceGroup->AddChild(requestPopupObj);

        auto transitionOverlayObj = std::make_shared<SceneObject>();
        auto transitionOverlayComp = transitionOverlayObj->AddComponent<TransitionOverlayGUI>();
        transitionOverlayComp->BeginFadeOut();
        m_interfaceGroup->AddChild(transitionOverlayObj);

        INSTANCE(GameGUIFacade)->QuestGUI = questGUIComp;
        INSTANCE(GameGUIFacade)->LogFloat = logFloatComp;
        INSTANCE(GameGUIFacade)->RequestPopup = requestPopupComp;
        INSTANCE(GameGUIFacade)->PartyStatus = partyStatusComp;
        INSTANCE(GameGUIFacade)->DamageCount = damageCountRenderer;
        INSTANCE(GameGUIFacade)->TransitionOverlay = transitionOverlayComp;
        INSTANCE(GameGUIFacade)->PopupManager = m_popupGUIManager;
    }
#pragma endregion

    if (false)
    {
        m_gizmoMaterial = std::make_shared<udsdx::Material>();
        m_gizmoMaterial->SetSourceTexture(res->Load<udsdx::Texture>(RESOURCE_PATH(L"Sprite-0001.png")));

        auto navMeshVisualizer = std::make_shared<SceneObject>();
        auto navMeshRenderer = navMeshVisualizer->AddComponent<MeshRenderer>();
        navMeshRenderer->SetMesh(res->Load<udsdx::Mesh>(RESOURCE_PATH(L"navmesh.yms")));
        navMeshRenderer->SetShader(res->Load<udsdx::Shader>(RESOURCE_PATH(L"color.hlsl")));
        navMeshRenderer->SetMaterial(m_gizmoMaterial.get());

        AddObject(navMeshVisualizer);
    }

    ChangeGameScene(GameSceneType::Default);
}

void GameScene::OnDetach()
{
    m_minimapRenderer->OnDetach();
}

void GameScene::Update(const Time& time)
{
    if (!m_pauseMenuObj->GetActive())
    {
        if (INSTANCE(Input)->GetKeyDown(Keyboard::I))
        {
            if (m_inventoryObj->GetActive())
                m_popupGUIManager->Pop(m_inventoryObj, true);
            else
                m_popupGUIManager->Append(m_inventoryObj);
        }
        if (INSTANCE(Input)->GetKeyDown(Keyboard::C))
        {
            if (m_craftObj->GetActive())
                m_popupGUIManager->Pop(m_craftObj, true);
            else
                m_popupGUIManager->Append(m_craftObj);
        }
        if (INSTANCE(Input)->GetKeyDown(Keyboard::Q))
        {
            if (m_partyListObj->GetActive())
                m_popupGUIManager->Pop(m_partyListObj, true);
            else
                m_popupGUIManager->Append(m_partyListObj, m_partyListObj->GetComponent<QuestGUI>()->GetQuestListPanel());
        }
        if (INSTANCE(Input)->GetKeyDown(Keyboard::Tab))
        {
            OnTogglePlayerMode(!m_bSpectatorMode);
        }
        if (INSTANCE(Input)->GetKeyDown(Keyboard::V))
        {
            ChangeGameScene(m_sceneType == GameSceneType::Default ? GameSceneType::Dungeon : GameSceneType::Default);
        }
    }

    m_playerTagObj->GetComponent<PlayerTagGUI>()->SetTargetPosition(m_heroObj->GetTransform()->GetWorldPosition() + Vector3::Up * 1.8f);

    Vector3 playerPosition = m_heroObj->GetTransform()->GetWorldPosition();
    Vector3 playerForward = Vector3::Transform(Vector3::Backward, GetMainCamera()->GetTransform()->GetWorldRotation());
    Vector3 playerForwardXZ = Vector3(playerForward.x, 0.0f, playerForward.z);
    playerForwardXZ.Normalize();
    playerForwardXZ.y -= 0.5f;
    playerForwardXZ.Normalize();
    m_minimapRenderer->SetViewMatrix(playerPosition, playerForwardXZ);

    Scene::Update(time);
}

void GameScene::Render(udsdx::RenderParam& param)
{
    m_minimapRenderer->PassRender(param);

    Scene::Render(param);
}

void GameScene::EnterGame(std::shared_ptr<GameScene> sharedScene, unsigned int character, int channelID)
{
    m_heroComponent->SetPlayerType(character);
    PlayerRenderer* playerRenderer = m_heroObj->GetComponent<PlayerRenderer>();
    switch (character)
    {
        case 0:
			playerRenderer->InitializeWarrior();
			break;
        case 1:
            playerRenderer->InitializePriest();
            break;
        default:
            playerRenderer->InitializePriest();
			break;
    }

    m_popupGUIManager->SetOnPopEmptyCallback([this]() {
        m_popupGUIManager->Append(m_pauseMenuObj);
        OnTogglePause(true);
        });
    m_popupGUIManager->SetOnFocusChangedCallback([this](bool focus) {
        INSTANCE(Input)->SetRelativeMouse(focus);
        if (!m_pauseMenuObj->GetActive() && focus)
            OnTogglePause(false);
        });
    INSTANCE(Input)->SetRelativeMouse(true);

    AddObject(m_heroObj);
    AddObject(m_spectatorObj);
    AddObject(m_activeObjectGroup);
    m_focusAgentObj->SetActive(true);

    OnTogglePause(false);
    OnTogglePlayerMode(false);

    {
        ServerObjectMgr::GetInst()->SetTargetScene(sharedScene);
        GuideSystem::GetInst()->SetTargetScene(sharedScene);
        GuideSystem::GetInst()->SetMainPlayer(m_heroObj);
    }

    // 만약 여기서 와일루프 돌면, 서버로부터 아직 ID발급을 못받았다는 이야기
    NetMgr(NetworkMgr)->ProcessLogin();

    // 서버오브젝트 매니저는 뭐 테스트 할 때 필요 할수도있음..
    ServerObjectMgr::GetInst()->SetMainHero(NetMgr(NetworkMgr)->GetSessionID(), m_heroObj);

    if constexpr (true == g_bUseNetWork)
    {
        // TODO: 여기서 캐릭터 종류를 넣어주세요
        m_currentChannelID = channelID;
        Nagox::Enum::PLAYER_TYPE player_type = character ? Nagox::Enum::PLAYER_TYPE::PLAYER_TYPE_PRIEST : Nagox::Enum::PLAYER_TYPE::PLAYER_TYPE_WARRIOR;
        std::cout << (int)m_currentChannelID << std::endl;
        Send(Create_c2s_ENTER
        (ToFlatVec3(m_heroObj->GetTransform()->GetLocalPosition())
            , player_type, m_currentChannelID));
    }

    m_tutorialObj->SetActive(true);
}

void GameScene::ExitGame()
{
    if constexpr (true == g_bUseNetWork)
    {
        NetMgr(NetworkMgr)->FinishNetwork();
    }

    // TODO: 일단 끄기전 서버오브젝트 컨테이너 밀어줌
    ServerObjectMgr::GetInst()->Clear();
    GuideSystem::GetInst()->ClearHarvest();
    // TODO: 이거해야 메모리릭 없는데 왜 터짐?
    // NAVIGATION->GetNavMesh(NAVI_MESH_NUM::NUM_0)->FreeNavMeshQuery();

    std::shared_ptr<MainScene> mainScene = std::make_shared<MainScene>();
    INSTANCE(Core)->SetScene(mainScene);
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

void GameScene::AddInterfaceObject(const std::shared_ptr<udsdx::SceneObject>& obj)
{
	m_playerInterfaceGroup->AddChild(obj);
}

void GameScene::ChangeGameScene(GameSceneType type)
{
    m_sceneType = type;

    m_defaultEnvironmentObject->SetActive(false);
    m_dungeonEnvironmentObject->SetActive(false);
    switch (type)
	{
		case GameSceneType::Default:
            m_defaultEnvironmentObject->SetActive(true);
			break;
		case GameSceneType::Dungeon:
            m_dungeonEnvironmentObject->SetActive(true);
			break;
	}
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