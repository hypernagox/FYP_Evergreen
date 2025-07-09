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

INT_PTR CALLBACK LoginDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static std::string* pIdPassword = nullptr;

    switch (message)
    {
    case WM_INITDIALOG:
        pIdPassword = reinterpret_cast<std::string*>(lParam);
        return TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK)
        {
            char id[100] = {};
            char password[100] = {};

            GetDlgItemTextA(hDlg, IDC_EDIT_ID, id, 100);
            GetDlgItemTextA(hDlg, IDC_EDIT_PASSWORD, password, 100);

            // 저장
            pIdPassword[0] = id;
            pIdPassword[1] = password;

            EndDialog(hDlg, IDOK);
            return TRUE;
        }
        else if (LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, IDCANCEL);
            return TRUE;
        }
        break;
    }
    return FALSE;
}

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
        mainMenuComp->SetEnterGameCallback([this]() {
            if constexpr (g_bUseNetWork)
            {
                std::string credentials[2]; // [0]: ID, [1]: Password
                INT_PTR ret = DialogBoxParam(
                    INSTANCE(Core)->GetInstance(),
                    MAKEINTRESOURCE(IDD_LOGIN_DIALOG),
                    INSTANCE(Core)->GetMainWindow(),
                    LoginDialogProc,
                    reinterpret_cast<LPARAM>(&credentials)
                );

                if (ret == IDOK)
                {
                    // 사용자가 입력 완료
                    std::string userId = credentials[0];
                    std::string userPw = credentials[1];

                    // 여기서 로그인 처리 로직 실행
                    //

                    m_popupGUIManager->Append(m_channelSwitchObj);
                }
            }
            else
            {
                EnterCharacterSelection();
            }
            });
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
