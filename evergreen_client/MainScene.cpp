#include "pch.h"
#include "MainScene.h"
#include "BezierMovement.h"
#include "PlayerSelect.h"
#include "PopupGUIManager.h"
#include "InputHandler.h"
#include "ChannelSwitchGUI.h"
#include "MainMenuGUI.h"
#include "MainMenuCharacterGUI.h"
#include "TransitionOverlayGUI.h"
#include "EnvironmentRenderer.h"
#include "GameGUIFacade.h"
#include "GameScene.h"

using namespace udsdx;

extern EnvironmentParameters g_defaultEnvironmentParam;

MainScene::MainScene() : Scene()
{
}

void MainScene::OnAttach()
{
    auto res = INSTANCE(Resource);

    {
        m_environmentObject = std::make_shared<SceneObject>();
        auto environmentRenderer = m_environmentObject->AddComponent<EnvironmentRenderer>();
        environmentRenderer->Initialize(g_defaultEnvironmentParam);

        AddObject(m_environmentObject);

        m_environmentLightObj = std::make_shared<SceneObject>();
        auto playerLight = m_environmentLightObj->AddComponent<LightDirectional>();
        Vector3 n = Vector3::Transform(Vector3::Up, Quaternion::CreateFromAxisAngle(Vector3(1.0f, 0.0f, -1.0f), 75.0f - 105.0f * 0.5f));
        m_environmentLightObj->GetTransform()->SetLocalRotation(Quaternion::CreateFromYawPitchRoll(-PIDIV4, PIDIV4, 0) * Quaternion::CreateFromAxisAngle(n, 0.0f));

        AddObject(m_environmentLightObj);
    }

    {
        auto skyboxObj = std::make_shared<SceneObject>();
        auto skyboxRenderer = skyboxObj->AddComponent<InlineMeshRenderer>();
        skyboxRenderer->SetMaterial(udsdx::Material(res->Load<Shader>(RESOURCE_PATH(L"skybox.hlsl")), res->Load<udsdx::Texture>(RESOURCE_PATH(L"Skybox.jpg"))));
        skyboxRenderer->SetVertexCount(6);
        skyboxRenderer->SetCastShadow(false);

        AddObject(skyboxObj);
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

    {
        m_interfaceGroup = std::make_shared<SceneObject>();
        AddObject(m_interfaceGroup);

        m_popupGUIManager = m_interfaceGroup->AddComponent<PopupGUIManager>();
        auto popupInputHandler = m_popupGUIManager->AddComponent<InputHandler>();
        popupInputHandler->AddKeyFunc(Keyboard::Escape, KET_TAP, [this]() { m_popupGUIManager->Pop(); });

        m_mainMenuObj = std::make_shared<SceneObject>();
        auto mainMenuComp = m_mainMenuObj->AddComponent<MainMenuGUI>();
        mainMenuComp->SetEnterGameCallback([this]() { m_popupGUIManager->Append(m_channelSwitchObj); });
        mainMenuComp->SetExitGameCallback([this]() { ExitGame(); });
        m_interfaceGroup->AddChild(m_mainMenuObj);

        m_channelSwitchObj = std::make_shared<SceneObject>();
        auto channelSwitchComp = m_channelSwitchObj->AddComponent<ChannelSwitchGUI>();
        channelSwitchComp->SetPanelGraphic(true);
        channelSwitchComp->SetChannelSelectedCallback([this](int channelID) {
            m_currentChannelID = channelID;
            m_popupGUIManager->Pop();
            EnterCharacterSelection();
            });
        m_channelSwitchObj->SetActive(false);
        m_interfaceGroup->AddChild(m_channelSwitchObj);

        m_playerSelectObj = std::make_shared<SceneObject>();
        m_playerSelectObj->SetActive(false);
        auto mainMenuCharacterComp = m_playerSelectObj->AddComponent<MainMenuCharacterGUI>();
        mainMenuCharacterComp->SetCharacterShowCallback([this](unsigned int character) {
            m_mainMenuCameraObject->GetComponent<PlayerSelect>()->SetShowingCharacter(character);
            });
        mainMenuCharacterComp->SetEnterGameCallback([this](unsigned int character) { EnterGame(character); });
        m_interfaceGroup->AddChild(m_playerSelectObj);

        auto transitionOverlayObj = std::make_shared<SceneObject>();
        m_transitionOverlayGUI = transitionOverlayObj->AddComponent<TransitionOverlayGUI>();
        m_transitionOverlayGUI->BeginFadeOut();
        m_interfaceGroup->AddChild(transitionOverlayObj);

        INSTANCE(GameGUIFacade)->TransitionOverlay = m_transitionOverlayGUI;
    }
}

void MainScene::EnterCharacterSelection()
{
    m_mainMenuObj->SetActive(false);
    m_mainMenuCameraObject->GetComponent<CameraPerspective>()->SetClipOffset(Vector2::Zero);
    m_mainMenuCameraObject->GetComponent<BezierMovement>()->SetActive(false);
    m_mainMenuCameraObject->AddComponent<PlayerSelect>();
    m_playerSelectObj->SetActive(true);
}

void MainScene::EnterGame(unsigned int character)
{
    std::shared_ptr<GameScene> gameScene = std::make_shared<GameScene>();
    INSTANCE(Core)->SetScene(gameScene);
    gameScene->EnterGame(gameScene, character, m_currentChannelID);
}

void MainScene::ExitGame()
{
    UpdownStudio::Quit();
}
