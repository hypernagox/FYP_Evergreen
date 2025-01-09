// Windows Header Files
#include <windows.h>
// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <updown_studio.h>

using namespace udsdx;

std::shared_ptr<Scene> g_scene;
std::shared_ptr<SceneObject> g_cameraObject;
std::shared_ptr<SceneObject> g_lightObject;
std::shared_ptr<SceneObject> g_object[3];
std::shared_ptr<SceneObject> g_floorObject;
std::shared_ptr<udsdx::Material> g_material;

void Update(const Time& time);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    INSTANCE(Resource)->SetResourceRootPath(L"resource");
    UpdownStudio::Initialize(hInstance);
    UpdownStudio::RegisterUpdateCallback(Update);

    auto res = INSTANCE(Resource);

    g_scene = std::make_shared<Scene>();

    g_cameraObject = std::make_shared<SceneObject>();
    g_cameraObject->GetTransform()->SetLocalPosition(Vector3(0.0f, 2.0f, -5.0f));
    auto camera = g_cameraObject->AddComponent<CameraPerspective>();
    g_scene->AddObject(g_cameraObject);

    g_lightObject = std::make_shared<SceneObject>();
    auto light = g_lightObject->AddComponent<LightDirectional>();
    g_lightObject->GetTransform()->SetLocalRotation(Quaternion::CreateFromYawPitchRoll(0.25f, 1.0f, 0));
    g_scene->AddObject(g_lightObject);

    g_material = std::make_shared<udsdx::Material>();
    g_material->SetMainTexture(res->Load<udsdx::Texture>(L"resource\\UV_checker_Map_byValle.jpg"));

    for (int i = 0; i < 3; ++i)
    {
        g_object[i] = std::make_shared<SceneObject>();
        auto renderer = g_object[i]->AddComponent<MeshRenderer>();
        renderer->SetMesh(res->Load<Mesh>(L"resource\\cube.obj"));
        renderer->SetMaterial(g_material.get());
        renderer->SetShader(res->Load<udsdx::Shader>(L"resource\\color.hlsl"));
        g_scene->AddObject(g_object[i]);
    }

    g_object[0]->GetTransform()->SetLocalPosition(Vector3(0.0f, 0.0f, 2.0f));
    g_object[1]->GetTransform()->SetLocalPosition(Vector3(-2.0f, 0.0f, -2.0f));
    g_object[2]->GetTransform()->SetLocalPosition(Vector3(2.0f, 0.0f, -2.0f));
    g_object[0]->GetTransform()->SetLocalScale(Vector3::One * 1.5f);
    g_object[1]->GetTransform()->SetLocalScale(Vector3::One * 1.5f);

    g_floorObject = std::make_shared<SceneObject>();
    g_floorObject->GetTransform()->SetLocalPosition(Vector3(0.0f, -1.5f, 0.0f));
    g_floorObject->GetTransform()->SetLocalScale(Vector3(20.0f, 0.25f, 20.0f));
    auto floorRenderer = g_floorObject->AddComponent<MeshRenderer>();
    floorRenderer->SetMesh(res->Load<Mesh>(L"resource\\cube.obj"));
    floorRenderer->SetMaterial(g_material.get());
    floorRenderer->SetShader(res->Load<udsdx::Shader>(L"resource\\colornotex.hlsl"));
    g_scene->AddObject(g_floorObject);

    INSTANCE(Core)->GetRenderer()->SetEnvironmentMap(res->Load<udsdx::Texture>(L"resource\\Interior.jpg"));

    return UpdownStudio::Run(g_scene, nCmdShow);
}

void Update(const Time& time)
{
    static int lastMouseX;
    static int lastMouseY;
    static int concatenatedMouseX = 0;
    static int concatenatedMouseY = 500;

    if (INSTANCE(Input)->GetKey(Keyboard::Escape))
    {
        UpdownStudio::Quit();
	}

    // Camera Rotation
    if (INSTANCE(Input)->GetMouseLeftButtonDown())
    {
        lastMouseX = INSTANCE(Input)->GetMouseX();
        lastMouseY = INSTANCE(Input)->GetMouseY();
    }
    if (INSTANCE(Input)->GetMouseLeftButton())
    {
        int mouseX = INSTANCE(Input)->GetMouseX();
        int mouseY = INSTANCE(Input)->GetMouseY();
        concatenatedMouseX += mouseX - lastMouseX;
        concatenatedMouseY += mouseY - lastMouseY;
        lastMouseX = mouseX;
        lastMouseY = mouseY;
    }
    Quaternion rotation = Quaternion::CreateFromYawPitchRoll(static_cast<float>(concatenatedMouseX) * 0.001f, static_cast<float>(concatenatedMouseY) * 0.001f, 0);
    g_cameraObject->GetTransform()->SetLocalRotation(rotation);

    // Camera Translation
    Vector3 translation = Vector3::Zero;
    if (INSTANCE(Input)->GetKey(Keyboard::W))
    {
        translation += Vector3::Backward;
    }
    if (INSTANCE(Input)->GetKey(Keyboard::S))
    {
        translation += Vector3::Forward;
    }
    if (INSTANCE(Input)->GetKey(Keyboard::A))
    {
        translation += Vector3::Left;
    }
    if (INSTANCE(Input)->GetKey(Keyboard::D))
    {
        translation += Vector3::Right;
    }
    if (INSTANCE(Input)->GetKey(Keyboard::Space))
    {
        translation += Vector3::Up;
    }
    if (INSTANCE(Input)->GetKey(Keyboard::LeftShift))
    {
        translation += Vector3::Down;
    }
    Matrix4x4 rotationMat = Matrix4x4::CreateFromQuaternion(rotation);
    translation = Vector3::Transform(translation, rotationMat) * time.deltaTime * 10.0f;
    g_cameraObject->GetTransform()->Translate(translation);
    
    float tParam = time.totalTime * 10.0f;
    float sParam = std::sin(tParam);
    sParam = std::powf(std::abs(sParam), 0.6f) * (sParam > 0 ? 1.0f : -1.0f);
    g_object[0]->GetTransform()->SetLocalPositionX(sParam);
    g_object[1]->GetTransform()->SetLocalRotation(Quaternion::CreateFromYawPitchRoll(tParam, 0.0f, 0.0f));
    g_object[2]->GetTransform()->SetLocalScale(Vector3::One * (sParam * 0.5f + 1.0f));
}