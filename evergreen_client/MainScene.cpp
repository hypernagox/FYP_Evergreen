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
        {
            HWND hParent = GetParent(hDlg);
            if (!hParent)
                hParent = GetDesktopWindow();

            RECT parentRect;
            GetWindowRect(hParent, &parentRect);

            RECT dlgRect;
            GetWindowRect(hDlg, &dlgRect);

            int dlgWidth = dlgRect.right - dlgRect.left;
            int dlgHeight = dlgRect.bottom - dlgRect.top;

            int parentWidth = parentRect.right - parentRect.left;
            int parentHeight = parentRect.bottom - parentRect.top;

            int newX = parentRect.left + (parentWidth - dlgWidth) / 2;
            int newY = parentRect.top + (parentHeight - dlgHeight) / 2;

            SetWindowPos(hDlg, HWND_TOP, newX, newY, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
        }

        pIdPassword = reinterpret_cast<std::string*>(lParam);
        return TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDLOGIN || LOWORD(wParam) == IDREGISTER)
        {
            char id[100] = {};
            char password[100] = {};

            GetDlgItemTextA(hDlg, IDC_EDIT_ID, id, 100);
            GetDlgItemTextA(hDlg, IDC_EDIT_PASSWORD, password, 100);

            // 저장
            pIdPassword[0] = id;
            pIdPassword[1] = password;

            EndDialog(hDlg, LOWORD(wParam));
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
        m_environmentObject = SceneObject::MakeShared();
        auto environmentRenderer = m_environmentObject->AddComponent<EnvironmentRenderer>();
        environmentRenderer->Initialize(g_defaultEnvironmentParam);

        AddObject(m_environmentObject);

        m_environmentLightObj = SceneObject::MakeShared();
        auto playerLight = m_environmentLightObj->AddComponent<LightDirectional>();
        Vector3 n = Vector3::Transform(Vector3::Up, Quaternion::CreateFromAxisAngle(Vector3(1.0f, 0.0f, -1.0f), 75.0f - 105.0f * 0.5f));
        m_environmentLightObj->GetTransform()->SetLocalRotation(Quaternion::CreateFromYawPitchRoll(-PIDIV4, PIDIV4, 0) * Quaternion::CreateFromAxisAngle(n, 0.0f));

        AddObject(m_environmentLightObj);
    }

    {
        auto skyboxObj = SceneObject::MakeShared();
        auto skyboxRenderer = skyboxObj->AddComponent<InlineMeshRenderer>();
        skyboxRenderer->SetMaterial(udsdx::Material(res->Load<Shader>(RESOURCE_PATH(L"skybox.hlsl")), res->Load<udsdx::Texture>(RESOURCE_PATH(L"Skybox.jpg"))));
        skyboxRenderer->SetVertexCount(6);
        skyboxRenderer->SetCastShadow(false);

        AddObject(skyboxObj);
    }

    {
        m_mainMenuCameraObject = SceneObject::MakeShared();
        m_mainMenuCameraObject->GetTransform()->SetLocalPosition(Vector3(0.0f, 120.0f, 0.0f));
        m_mainMenuCameraObject->GetTransform()->SetLocalRotation(Quaternion::CreateFromYawPitchRoll(PIDIV4, PIDIV4, 0));

        m_characterSelectObject = SceneObject::MakeShared();
        auto playerSelect = m_characterSelectObject->AddComponent<PlayerSelect>();
        playerSelect->SetTargetTransform(m_mainMenuCameraObject->GetTransform());
        m_mainMenuCameraObject->AddChild(m_characterSelectObject);

        auto camera = m_mainMenuCameraObject->AddComponent<CameraPerspective>();
        camera->SetFov(PI / 4.0f);

        auto bezierMovement = m_mainMenuCameraObject->AddComponent<BezierMovement>();
        bezierMovement->LoadSpline(RESOURCE_PATH(L"environment\\CameraPathSpline.json"));
        bezierMovement->SetSpeed(16.0f);

        AddObject(m_mainMenuCameraObject);
    }

    {
        m_interfaceGroup = SceneObject::MakeShared();
        AddObject(m_interfaceGroup);

        m_popupGUIManager = m_interfaceGroup->AddComponent<PopupGUIManager>();
        auto popupInputHandler = m_popupGUIManager->AddComponent<InputHandler>();
        popupInputHandler->AddKeyFunc(Keyboard::Escape, KET_TAP, [this]() { m_popupGUIManager->Pop(); });

        m_mainMenuObj = SceneObject::MakeShared();
        auto mainMenuComp = m_mainMenuObj->AddComponent<MainMenuGUI>();
        mainMenuComp->SetEnterGameCallback([this]() { OnLoginDialog(); });
        mainMenuComp->SetExitGameCallback([this]() { ExitGame(); });
        m_interfaceGroup->AddChild(m_mainMenuObj);

        m_channelSwitchObj = SceneObject::MakeShared();
        auto channelSwitchComp = m_channelSwitchObj->AddComponent<ChannelSwitchGUI>();
        channelSwitchComp->SetPanelGraphic(true);
        channelSwitchComp->SetChannelSelectedCallback([this](int channelID) {
            m_currentChannelID = channelID;
            if (m_needCharacterSelection)
            {
                m_popupGUIManager->Pop();
                EnterCharacterSelection(true);
            }
            else
                TransitionEnterGame();
            });
        m_channelSwitchObj->SetActive(false);
        m_interfaceGroup->AddChild(m_channelSwitchObj);

        m_playerSelectObj = SceneObject::MakeShared();
        m_playerSelectObj->SetActive(false);
        auto mainMenuCharacterComp = m_playerSelectObj->AddComponent<MainMenuCharacterGUI>();
        mainMenuCharacterComp->SetCharacterShowCallback([this](unsigned int character) {
            m_characterSelectObject->GetComponent<PlayerSelect>()->SetShowingCharacter(character);
            });
        mainMenuCharacterComp->SetEnterGameCallback([this](unsigned int character) {
            m_currentCharacterType = character;
            if (WaitRegisterResult())
                TransitionEnterGame();
            else
                EnterCharacterSelection(false);
            });
        m_interfaceGroup->AddChild(m_playerSelectObj);

        auto transitionOverlayObj = SceneObject::MakeShared();
        m_transitionOverlayGUI = transitionOverlayObj->AddComponent<TransitionOverlayGUI>();
        m_transitionOverlayGUI->BeginFadeOut();
        m_interfaceGroup->AddChild(transitionOverlayObj);

        INSTANCE(GameGUIFacade)->TransitionOverlay = m_transitionOverlayGUI;
    }

    INSTANCE(Core)->GetRenderOptionsRef().FogDensity = 15.85f;
    INSTANCE(Core)->GetRenderOptionsRef().FogHeightFalloff = 0.08f;
    INSTANCE(Core)->GetRenderOptionsRef().FogDistanceStart = 60.0f;

    EnterCharacterSelection(false);
}

void MainScene::OnLoginDialog()
{
    if constexpr (g_bUseNetWork)
    {
        if (m_firstLoginAttempt)
        {
            NetMgr(NetworkMgr)->ProcessLogin();
            m_firstLoginAttempt = false;
        }

        std::string credentials[2]; // [0]: ID, [1]: Password
        INT_PTR ret = DialogBoxParam(
            INSTANCE(Core)->GetInstance(),
            MAKEINTRESOURCE(IDD_LOGIN_DIALOG),
            INSTANCE(Core)->GetMainWindow(),
            LoginDialogProc,
            reinterpret_cast<LPARAM>(&credentials)
        );

        if (ret == IDLOGIN || ret == IDREGISTER)
        {
            // 사용자가 입력 완료
            m_userId.clear(); m_userPw.clear(); m_class_type.clear();
            m_userId = credentials[0];
            m_userPw = credentials[1];

            if (m_userId.empty() || m_userPw.empty())
            {
                MessageBox(INSTANCE(Core)->GetMainWindow(), L"ID와 비밀번호를 입력해주세요.", L"입력 오류", MB_OK | MB_ICONWARNING);
            }
            else
            {
                switch (ret)
                {
                case IDLOGIN:
                    m_register_account = false;
                    m_needCharacterSelection = false;
                    // 여기서 로그인 처리 로직 실행
                    Send(Create_c2s_LOGIN(m_userId, m_userPw));
                    break;
                case IDREGISTER:
                    m_register_account = true;
                    m_needCharacterSelection = true;
                    // 여기서 회원가입 처리 로직 실행
                    m_popupGUIManager->Append(m_channelSwitchObj);
                    break;
                }
            }
        }
    }
    else
    {
        EnterCharacterSelection(true);
    }
}

void MainScene::EnterCharacterSelection(bool enter)
{
    m_mainMenuObj->SetActive(!enter);
    m_mainMenuCameraObject->GetComponent<CameraPerspective>()->SetClipOffset(enter ? Vector2::Zero : Vector2(1.0f / 3.0f, 0.0f));
    m_mainMenuCameraObject->GetComponent<BezierMovement>()->SetActive(!enter);
    m_characterSelectObject->SetActive(enter);
    m_playerSelectObj->SetActive(enter);
}

void MainScene::OnLoginResult(Nagox::Enum::LOGIN_RESULT result, unsigned int characterType)
{
    m_isRegisterSuccess = false;
    m_currentCharacterType = characterType;
   
    switch (result)
    {
    case Nagox::Enum::LOGIN_RESULT_FAIL:
    {
        // TODO: 비번틀림 로그인 시도 다시하기 

        MessageBox(INSTANCE(Core)->GetMainWindow(), L"로그인 실패: ID 또는 비밀번호를 다시 확인해주세요.", L"로그인 실패", MB_OK | MB_ICONWARNING);
        break;
    }
    case Nagox::Enum::LOGIN_RESULT_NONE:
    {
        // TODO: 비번틀림 로그인 시도 다시하기 

        MessageBox(INSTANCE(Core)->GetMainWindow(), L"로그인 실패: 존재하지 않는 ID 입니다.", L"로그인 실패", MB_OK | MB_ICONWARNING);
        break;
    }
    case Nagox::Enum::LOGIN_RESULT_DUPLICATE:
    {
        // TODO: 비번틀림 로그인 시도 다시하기 
        m_isRegisterSuccess = false;
        MessageBox(INSTANCE(Core)->GetMainWindow(), L"계정 생성 실패: 이미 존재하는 ID입니다.", L"계정 생성 실패", MB_OK | MB_ICONWARNING);
        break;
    }
    case Nagox::Enum::LOGIN_RESULT_SUCCESS:
    {
        // TODO: 인게임 내에서 자기직업이 뭔지 혹시 알아야할수도있어서 일단 해둠
        switch (characterType)
        {
        case 0:
        {
            m_class_type = "Warrior";
        }
        break;
        case 1:
        {
            m_class_type = "Priest";
        }
        break;
        case 2:
        {
            m_class_type = "Archer";
        }
        break;
        default:
            break;
        }
        m_isRegisterSuccess = true;
        if (!m_needCharacterSelection)
        {
            m_popupGUIManager->Append(m_channelSwitchObj);
        }
        break;
    }
    default:
        break;
    }
}

bool MainScene::WaitRegisterResult()
{
    if (m_register_account)
    {
        switch (m_currentCharacterType)
        {
        case 0:
        {
            m_class_type = "Warrior";
        }
        break;
        case 1:
        {
            m_class_type = "Priest";
        }
        break;
        case 2:
        {
            m_class_type = "Archer";
        }
        break;
        default:
            break;
        }
        Send(Create_c2s_REGISTER_ACCOUNT(m_userId, m_userPw, m_class_type));
        while (NetMgr(NetworkMgr)->GetSessionID() == 0)
        {
            NetMgr(NetworkMgr)->DoNetworkIO();
        }
        // TODO: 중복아이디라면 실패하고 다시 돌아가야함
        if (false == m_isRegisterSuccess)
        {
            return false;
        }
    }
    return true;
}

void MainScene::TransitionEnterGame()
{
    INSTANCE(GameGUIFacade)->TransitionOverlay->AppendTransition([this]() { EnterGame(); }, std::format(L"서버 입장 중 ...") );
}

void MainScene::EnterGame()
{
    std::shared_ptr<GameScene> gameScene = std::make_shared<GameScene>();
    INSTANCE(Core)->SetScene(gameScene);
    gameScene->EnterGame(gameScene, m_currentCharacterType, m_currentChannelID, m_userId);
}

void MainScene::ExitGame()
{
    UpdownStudio::Quit();
}
