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
#include "PlayerEquipmentGUI.h"
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
    auto resource = INSTANCE(Resource);
    auto shader = resource->Load<Shader>(RESOURCE_PATH(L"color.hlsl"));

    m_heroObj = SceneObject::MakeShared();
    m_heroComponent = m_heroObj->AddComponent<AuthenticPlayer>();
    auto entityInteraction = m_heroObj->AddComponent<EntityInteraction>();
    entityInteraction->SetTargetScene(this);
    m_heroServerObject = m_heroObj->GetComponent<ServerObject>();
    m_heroServerObject->AddComp<MovePacketSender>();

    Vector3 start_pos = Vector3{
        -315.8432f,
        84.93234f,
        -33.050846f
    };
    auto& cell = m_heroServerObject->GetNaviAgent()->GetCurCell();
    cell = NAVIGATION->GetNavMesh(NAVI_MESH_TYPE::MAIN_WORLD)->GetNaviCell(start_pos);
    
    m_activeObjectGroup = SceneObject::MakeShared();
    for (auto& objectGroup : m_activeObjectSubGroups)
    {
        objectGroup = SceneObject::MakeShared();
        m_activeObjectGroup->AddChild(objectGroup);
    }

    m_heroObj->GetTransform()->SetLocalPosition(start_pos);

    m_spectatorObj = SceneObject::MakeShared();
    m_spectatorObj->AddComponent<SpectatorPlayer>();

#pragma region Environment Initialization
    m_environmentLightObj = SceneObject::MakeShared();
    auto playerLight = m_environmentLightObj->AddComponent<LightDirectional>();
    Vector3 n = Vector3::Transform(Vector3::Up, Quaternion::CreateFromAxisAngle(Vector3(1.0f, 0.0f, -1.0f), 75.0f - 105.0f * 0.5f));
    m_environmentLightObj->GetTransform()->SetLocalRotation(Quaternion::CreateFromYawPitchRoll(-PIDIV4, PIDIV4, 0) * Quaternion::CreateFromAxisAngle(n, 0.0f));

    AddObject(m_environmentLightObj);

    {
        m_craftTableObj = SceneObject::MakeShared();
        m_craftTableObj->GetTransform()->SetLocalPosition(Vector3(-123.22470092773438f, 75.68199920654297f, 16.593002319335939f));
        m_craftTableObj->GetTransform()->SetLocalRotation(Quaternion(0.0f, 1.0f, 0.0f, 0.0f));
        m_craftTableObj->GetTransform()->SetLocalScale(Vector3(-1.0f, 1.0f, -1.0f) * 0.01f);

        udsdx::Material craftTableMaterial(shader);
        craftTableMaterial.SetSourceTexture(resource->Load<udsdx::Texture>(RESOURCE_PATH(L"environment\\Maps\\M_Kit_1\\M_Kit_1_D.tga")), 0);
        craftTableMaterial.SetSourceTexture(resource->Load<udsdx::Texture>(RESOURCE_PATH(L"environment\\Maps\\M_Kit_1\\M_Kit_1_N.tga")), 1);

        auto craftTableRenderer = m_craftTableObj->AddComponent<MeshRenderer>();
        craftTableRenderer->SetMesh(resource->Load<udsdx::Mesh>(RESOURCE_PATH(L"environment\\Village\\O_Table_B.yms")));
        craftTableRenderer->SetMaterial(craftTableMaterial, 0);

        auto interactiveEntity = m_craftTableObj->AddComponent<InteractiveEntity>();
        interactiveEntity->SetInteractionText(L"제작하기");
        interactiveEntity->SetInteractionCallback([this]() { m_popupGUIManager->Append(m_craftObj); });

        AddActiveObject(m_craftTableObj, GameSceneType::Default);
    }

    {
        std::shared_ptr<SceneObject> treeObj = SceneObject::MakeShared();
        auto treeRenderer = treeObj->AddComponent<MeshRenderer>();

        treeObj->GetTransform()->SetLocalScale(Vector3::One * 0.01f);
        treeObj->GetTransform()->SetLocalPosition(Vector3(-42.968254f, 74.610634f, -87.984f));
        treeObj->SetActive(true);

        treeRenderer->SetMesh(resource->Load<udsdx::Mesh>(RESOURCE_PATH(L"goldentree\\tree.yms")));
        treeRenderer->SetMaterial(shader);

        {
            udsdx::Material treeMaterial(shader);
            treeMaterial.SetSourceTexture(resource->Load<udsdx::Texture>(RESOURCE_PATH(L"goldentree\\leaves_color.png")), 0);
            treeMaterial.SetSourceTexture(resource->Load<udsdx::Texture>(RESOURCE_PATH(L"goldentree\\leaves_nm.png")), 1);

            treeRenderer->SetMaterial(treeMaterial, 0);
        }

        {
            udsdx::Material treeMaterial(shader);
            treeMaterial.SetSourceTexture(resource->Load<udsdx::Texture>(RESOURCE_PATH(L"goldentree\\trunk_Base_color.png")), 0);
            treeMaterial.SetSourceTexture(resource->Load<udsdx::Texture>(RESOURCE_PATH(L"goldentree\\trunk_Normal_OpenGL.png")), 1);

            treeRenderer->SetMaterial(treeMaterial, 1);
        }

        {
            auto interactiveEntity = treeObj->AddComponent<InteractiveEntity>();
            interactiveEntity->SetInteractionText(L"보상 얻기");
            interactiveEntity->SetInteractionCallback([]() { 
                //Send(Create_c2s_CHANGE_HARVEST_STATE());
                });
        }

        AddActiveObject(treeObj, GameSceneType::Default);
        GuideSystem::GetInst()->AddHarvestMeshObject(treeObj);

        m_defaultEnvironmentObject = SceneObject::MakeShared();
        auto environmentRenderer = m_defaultEnvironmentObject->AddComponent<EnvironmentRenderer>();
        environmentRenderer->Initialize(g_defaultEnvironmentParam);
        AddObject(m_defaultEnvironmentObject);

        m_dungeonEnvironmentObject = SceneObject::MakeShared();
        auto dungeonEnvironmentRenderer = m_dungeonEnvironmentObject->AddComponent<EnvironmentRenderer>();
        dungeonEnvironmentRenderer->Initialize(g_dungeonEnvironmentParam);
        AddObject(m_dungeonEnvironmentObject);

        auto dungeonWaterObj = SceneObject::MakeShared();
        dungeonWaterObj->GetTransform()->SetLocalPosition(Vector3(0.0f, 11.3f, 0.0f));
        dungeonWaterObj->GetTransform()->SetLocalScale(0.01f);
        auto dungeonWaterRenderer = dungeonWaterObj->AddComponent<MeshRenderer>();
        dungeonWaterRenderer->SetMesh(resource->Load<udsdx::Mesh>(RESOURCE_PATH(L"environment\\plane.yms")));

        udsdx::Material waterMaterial(INSTANCE(Resource)->Load<udsdx::Shader>(RESOURCE_PATH(L"water.hlsl")));
        waterMaterial.SetSourceTexture(resource->Load<udsdx::Texture>(RESOURCE_PATH(L"environment\\Water.png")), 0);
        waterMaterial.SetSourceTexture(resource->Load<udsdx::Texture>(RESOURCE_PATH(L"environment\\Flow Speed Noise.png")), 1);
        waterMaterial.SetSourceTexture(resource->Load<udsdx::Texture>(RESOURCE_PATH(L"environment\\Water Derivative Height.png")), 2);
        dungeonWaterRenderer->SetMaterial(waterMaterial);

        m_dungeonEnvironmentObject->AddChild(dungeonWaterObj);

        std::ifstream file(RESOURCE_PATH(L"environment\\ExportedGameSpawns.json"));
        nlohmann::json j;
        file >> j;
        for (auto& prototype : j)
        {
            for (auto& instance : prototype["instances"])
            {
                auto harvestObject = environmentRenderer->AddHarvestObject(instance);
                AddActiveObject(harvestObject, GameSceneType::Default);
                GuideSystem::GetInst()->AddHarvestMeshObject(harvestObject);
            }
        }

        GuideSystem::GetInst()->AddHarvestMeshObject(treeObj);

        m_minimapRenderer = std::make_unique<MinimapRenderer>(INSTANCE(Core)->GetDevice(), 256, 256);
    }

    {
        auto skyboxObj = SceneObject::MakeShared();
        auto skyboxRenderer = skyboxObj->AddComponent<InlineMeshRenderer>();
        skyboxRenderer->SetMaterial(udsdx::Material(resource->Load<Shader>(RESOURCE_PATH(L"skybox.hlsl")), resource->Load<udsdx::Texture>(RESOURCE_PATH(L"Skybox.jpg"))));
        skyboxRenderer->SetVertexCount(6);
        skyboxRenderer->SetCastShadow(false);

        AddObject(skyboxObj);
    }
#pragma endregion

#pragma region GUI Initialization
    {
        m_interfaceGroup = SceneObject::MakeShared();
        m_playerInterfaceGroup = SceneObject::MakeShared();

        AddObject(m_interfaceGroup);
        m_interfaceGroup->AddChild(m_playerInterfaceGroup);
        m_playerInterfaceGroup->SetActive(false);

        auto textObj = SceneObject::MakeShared();
        auto textRenderer = textObj->AddComponent<GUIText>();
        textObj->GetTransform()->SetLocalPosition(Vector3(-640, 480, 0));
        textRenderer->SetText(GET_DATA(std::wstring, "Script", "Intro", "Start"));
        textRenderer->SetFont(resource->Load<udsdx::Font>(RESOURCE_PATH(L"pretendard.spritefont")));
        textRenderer->SetAlignment(GUIText::Alignment::UpperLeft);
        // m_playerInterfaceGroup->AddChild(textObj);

        auto damageCountObj = SceneObject::MakeShared();
        auto damageCountRenderer = damageCountObj->AddComponent<DamageCountGUI>();
        m_playerInterfaceGroup->AddChild(damageCountObj);

        m_playerTagObj = SceneObject::MakeShared();
        auto playerTagRenderer = m_playerTagObj->AddComponent<PlayerTagGUI>();
        m_playerInterfaceGroup->AddChild(m_playerTagObj);

        m_focusAgentObj = SceneObject::MakeShared();
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

        auto minimapObj = SceneObject::MakeShared();
        auto minimapImage = minimapObj->AddComponent<GUIImage>();
        minimapImage->SetTexture(m_minimapRenderer->GetRenderTargetTexture());
        minimapImage->SetSize(Vector2(360.0f, 360.0f));
        minimapObj->GetTransform()->SetLocalPosition(Vector3(-1060.0f, -480.0f, 0.0f));

        {
            auto minimapBackground = SceneObject::MakeShared();
            auto minimapBackgroundRenderer = minimapBackground->AddComponent<GUIImage>();
            minimapBackgroundRenderer->SetTexture(resource->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\minimap_outline_gradation.png")));
            minimapBackgroundRenderer->SetSize(Vector2(365.0f, 365.0f));

            auto minimapMarkerObj = SceneObject::MakeShared();
            minimapMarkerObj->GetTransform()->SetLocalPosition(Vector3(0.0f, 16.0f, 0.0f));
            auto minimapMarkerRenderer = minimapMarkerObj->AddComponent<GUIImage>();
            minimapMarkerRenderer->SetTexture(resource->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\minimap_marker.png")));
            minimapMarkerRenderer->SetSize(Vector2(32.0f, 32.0f));

            minimapObj->AddChild(minimapBackground);
            minimapObj->AddChild(minimapMarkerObj);
        }

        m_playerInterfaceGroup->AddChild(minimapObj);

        auto guiObj = SceneObject::MakeShared();
        auto guiRenderer = guiObj->AddComponent<PlayerStatusGUI>();
        m_playerInterfaceGroup->AddChild(guiObj);
        m_heroComponent->SetPlayerStatusGUI(guiRenderer);

        auto quickSlotObj = SceneObject::MakeShared();
        auto quickSlotRenderer = quickSlotObj->AddComponent<PlayerQuickSlotGUI>();
        m_playerInterfaceGroup->AddChild(quickSlotObj);
        m_heroComponent->SetPlayerQuickSlotGUI(quickSlotRenderer);

        auto logFloatObj = SceneObject::MakeShared();
        auto logFloatComp = logFloatObj->AddComponent<LogFloatGUI>();
        logFloatComp->AddText(L"Welcome to the game!");
        m_playerInterfaceGroup->AddChild(logFloatObj);

        m_tutorialObj = SceneObject::MakeShared();
        m_tutorialObj->AddComponent<TutorialUI>();
        m_tutorialObj->SetActive(false);
        m_playerInterfaceGroup->AddChild(m_tutorialObj);

        m_inventoryObj = SceneObject::MakeShared();
        auto inventoryRenderer = m_inventoryObj->AddComponent<PlayerInventoryGUI>();
        m_playerInterfaceGroup->AddChild(m_inventoryObj);
        m_heroComponent->SetPlayerInventoryGUI(inventoryRenderer);
        m_inventoryObj->SetActive(false);

        m_equipmentObj = SceneObject::MakeShared();
        auto equipmentRenderer = m_equipmentObj->AddComponent<PlayerEquipmentGUI>();
        m_playerInterfaceGroup->AddChild(m_equipmentObj);
        m_heroComponent->SetPlayerEquipmentGUI(equipmentRenderer);
        m_equipmentObj->SetActive(false);

        m_craftObj = SceneObject::MakeShared();
        auto craftComp = m_craftObj->AddComponent<PlayerCraftGUI>();
        m_playerInterfaceGroup->AddChild(m_craftObj);
        m_heroComponent->SetPlayerCraftGUI(craftComp);
        m_craftObj->SetActive(false);

        m_partyListObj = SceneObject::MakeShared();
        auto questGUIComp = m_partyListObj->AddComponent<QuestGUI>();
        m_playerInterfaceGroup->AddChild(m_partyListObj);
        m_partyListObj->SetActive(false);

        auto partyStatusObj = SceneObject::MakeShared();
        auto partyStatusComp = partyStatusObj->AddComponent<PartyStatusGUI>();
        m_playerInterfaceGroup->AddChild(partyStatusObj);

        auto EntityInteractionObj = SceneObject::MakeShared();
        auto interactionFloatGUI = EntityInteractionObj->AddComponent<InteractionFloatGUI>();
        entityInteraction->SetInteractionFloatGUI(interactionFloatGUI);
        m_playerInterfaceGroup->AddChild(EntityInteractionObj);

        m_pauseMenuObj = SceneObject::MakeShared();
        m_pauseMenuObj->SetActive(false);
        auto pauseMenuComp = m_pauseMenuObj->AddComponent<GamePauseGUI>();
        pauseMenuComp->SetExitGameCallback([this]() { ExitGame(); });
        m_interfaceGroup->AddChild(m_pauseMenuObj);

        m_channelSwitchObj = SceneObject::MakeShared();
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

        auto requestPopupObj = SceneObject::MakeShared();
        auto requestPopupComp = requestPopupObj->AddComponent<RequestPopupGUI>();
        m_interfaceGroup->AddChild(requestPopupObj);

        auto transitionOverlayObj = SceneObject::MakeShared();
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
        auto navMeshVisualizer = SceneObject::MakeShared();
        auto navMeshRenderer = navMeshVisualizer->AddComponent<MeshRenderer>();
        navMeshRenderer->SetMesh(resource->Load<udsdx::Mesh>(RESOURCE_PATH(L"navmesh.yms")));
        navMeshRenderer->SetMaterial(udsdx::Material(resource->Load<udsdx::Shader>(RESOURCE_PATH(L"color.hlsl")), resource->Load<udsdx::Texture>(RESOURCE_PATH(L"Sprite-0001.png"))));

        AddObject(navMeshVisualizer);
    }

    m_ambienceSound = INSTANCE(Resource)->Load<AudioClip>(RESOURCE_PATH(L"audio\\ambience.wav"))->CreateInstance();
    m_ambienceSound->Play(true);

    ChangeGameScene(GameSceneType::Default);
}

void GameScene::OnDetach()
{
    m_ambienceSound->Stop();
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
            {
                m_popupGUIManager->Append(m_inventoryObj);
                m_popupGUIManager->Pop(m_equipmentObj, true);
            }
        }
        if (INSTANCE(Input)->GetKeyDown(Keyboard::O))
        {
			if (m_equipmentObj->GetActive())
				m_popupGUIManager->Pop(m_equipmentObj, true);
            else
            {
                m_popupGUIManager->Append(m_equipmentObj);
                m_popupGUIManager->Pop(m_inventoryObj, true);
            }
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
        if (INSTANCE(Input)->GetKeyDown(Keyboard::F1))
        {
            m_interfaceGroup->SetActive(!m_interfaceGroup->GetActive());
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
    m_minimapMarks.clear();

    Scene::Update(time);
}

void GameScene::Render(udsdx::RenderParam& param)
{
    m_minimapRenderer->PassRender(param, m_minimapMarks);

    Scene::Render(param);
}

void GameScene::EnterGame(std::shared_ptr<GameScene> sharedScene, unsigned int character, int channelID)
{
    m_currentChannelID = channelID;
    m_channelSwitchObj->GetComponent<ChannelSwitchGUI>()->InitializeChannel(channelID);

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
   // NetMgr(NetworkMgr)->ProcessLogin();

    // 서버오브젝트 매니저는 뭐 테스트 할 때 필요 할수도있음..
    ServerObjectMgr::GetInst()->SetMainHero(NetMgr(NetworkMgr)->GetSessionID(), m_heroObj);

    if constexpr (true == g_bUseNetWork)
    {
        Nagox::Enum::PLAYER_TYPE player_type = Nagox::Enum::PLAYER_TYPE::PLAYER_TYPE_WARRIOR;
        switch (character)
        {
        case 0:
            player_type = Nagox::Enum::PLAYER_TYPE::PLAYER_TYPE_WARRIOR;
            break;
        case 1:
            player_type = Nagox::Enum::PLAYER_TYPE::PLAYER_TYPE_PRIEST;
            break;
        case 2:
            // TODO: 거너로 바꾸기
            player_type = Nagox::Enum::PLAYER_TYPE::PLAYER_TYPE_PRIEST;
            break;
        default:
            break;
        }
        Send(Create_c2s_ENTER(ToFlatVec3(m_heroObj->GetTransform()->GetLocalPosition()), player_type, m_currentChannelID));
        NetMgr(ServerTimeMgr)->InitAndWaitServerTimeStamp([]()noexcept {NetMgr(NetworkMgr)->Send(Create_c2s_PING_PONG()); });
    }

    m_heroComponent->FixCameraAnchor();
    m_tutorialObj->SetActive(true);


    ServerObjectMgr::GetInst()->LoadInitDataFromDB();
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
    ServerObjectMgr::GetInst()->SetTargetMainScene(mainScene);
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

void GameScene::AddActiveObject(const std::shared_ptr<udsdx::SceneObject>& obj, GameSceneType type)
{
    m_activeObjectSubGroups[static_cast<std::uint8_t>(type)]->AddChild(obj);
}

void GameScene::AddInterfaceObject(const std::shared_ptr<udsdx::SceneObject>& obj)
{
	m_playerInterfaceGroup->AddChild(obj);
}

void GameScene::RequestChangeGameScene(GameSceneType type)
{
    if (m_sceneType == type)
        return;

    INSTANCE(GameGUIFacade)->TransitionOverlay->AppendTransition(
            [this, type]() { ChangeGameScene(type); },
            std::format(L"{} 맵으로 이동 중 ...", type == GameSceneType::Default ? L"마을" : L"보스")
    	);
}

void GameScene::ChangeGameScene(GameSceneType type)
{
    m_sceneType = type;

    m_defaultEnvironmentObject->SetActive(false);
    m_dungeonEnvironmentObject->SetActive(false);

    m_activeObjectSubGroups[0]->SetActive(type == GameSceneType::Default);
    m_activeObjectSubGroups[1]->SetActive(type == GameSceneType::Dungeon);

    switch (type)
	{
		case GameSceneType::Default:
            m_heroServerObject->SetNavigationMesh(NAVI_MESH_TYPE::MAIN_WORLD);
            m_defaultEnvironmentObject->SetActive(true);
            m_minimapRenderer->SetMinimapMesh(m_defaultEnvironmentObject->GetComponent<EnvironmentRenderer>()->GetTerrainMesh());
            m_minimapRenderer->SetMinimapEnvironment(g_defaultEnvironmentParam);
            m_heroComponent->SetEnvironment(&g_defaultEnvironmentParam);

            INSTANCE(Core)->GetRenderOptionsRef().FogDensity = 15.85f;
            INSTANCE(Core)->GetRenderOptionsRef().FogHeightFalloff = 0.08f;
			break;
		case GameSceneType::Dungeon:
            m_heroServerObject->SetNavigationMesh(NAVI_MESH_TYPE::BOSS_ROOM);
            m_dungeonEnvironmentObject->SetActive(true);
            m_minimapRenderer->SetMinimapMesh(m_dungeonEnvironmentObject->GetComponent<EnvironmentRenderer>()->GetTerrainMesh());
            m_minimapRenderer->SetMinimapEnvironment(g_dungeonEnvironmentParam);
            m_heroComponent->SetEnvironment(&g_dungeonEnvironmentParam);

            INSTANCE(Core)->GetRenderOptionsRef().FogDensity = 10.0f;
            INSTANCE(Core)->GetRenderOptionsRef().FogHeightFalloff = 0.015f;
			break;
	}
}

void GameScene::AddMinimapMark(const Vector3& position)
{
    m_minimapMarks.emplace_back(position);
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