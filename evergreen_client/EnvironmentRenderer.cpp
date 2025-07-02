#include "pch.h"
#include "EnvironmentRenderer.h"
#include <HeightMap.h>
#include "TerrainData.h"
#include "TerrainInstanceRenderer.h"
#include "InteractiveEntity.h"

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

EnvironmentRenderer::EnvironmentRenderer(const std::shared_ptr<SceneObject>& object) : Component(object)
{
}

void EnvironmentRenderer::Initialize(HeightMap* heightMap, TerrainData* terrainData, TerrainDetail* terrainDetail)
{
    auto res = INSTANCE(Resource);

    const float TerrainSize = GET_DATA(float, "GlobalValues", "TerrainSize", "Value");
    const Vector3 terrainPos = Vector3(-TerrainSize * 0.5f, 0, -TerrainSize * 0.5f);
    const Vector3 terrainScale = Vector3::One * TerrainSize;

    m_terrainMesh = CreateMeshFromHeightMap(heightMap, 512, 512, 1.0f);
    m_terrainMesh->UploadBuffers(INSTANCE(Core)->GetDevice(), INSTANCE(Core)->GetCommandList());

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

    {
        for (auto texture : INSTANCE(Resource)->LoadAll<udsdx::Texture>())
            m_textureMap[texture->GetName().data()] = texture;

        for (const auto& directory : std::filesystem::recursive_directory_iterator(RESOURCE_PATH(L"environment")))
        {
            // if the file is not a regular file(e.g. if it is a directory), skip it
            if (!directory.is_regular_file())
                continue;

            std::string filename = directory.path().filename().stem().string();
            std::wstring suffix = directory.path().extension().wstring();
            std::transform(suffix.begin(), suffix.end(), suffix.begin(), ::tolower);

            if (suffix != L".yms")
                continue;
            if (terrainData->GetPrototypeInstanceCount(filename) > 0)
                AddTerrainInstances(directory.path(), terrainData);
            m_prefabMap[filename] = directory.path();
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

        GetSceneObject()->AddChild(m_terrainObj);
    }
}

std::shared_ptr<udsdx::SceneObject> EnvironmentRenderer::AddTerrainInstances(std::filesystem::path path, TerrainData* terrainData)
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

        if (auto it = m_textureMap.find(aKey); it != m_textureMap.end())
            material->SetSourceTexture(it->second);
        else
            material->SetSourceTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"Sprite-0001.png")));
        if (auto it = m_textureMap.find(nKey); it != m_textureMap.end())
            material->SetSourceTexture(it->second, 1);
        else
            material->SetSourceTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"Sprite-0001.png")), 1);

        m_instanceMaterials.emplace_back(material);
        terrainInstanceRenderer->SetMaterial(material.get(), static_cast<int>(i));
    }

    terrainInstanceRenderer->SetTerrainData(terrainData, path.filename().stem().string());
    terrainInstanceRenderer->SetMesh(mesh);
    terrainInstanceRenderer->SetShader(INSTANCE(Resource)->Load<udsdx::Shader>(RESOURCE_PATH(L"colorinstanced.hlsl")));

    GetSceneObject()->AddChild(terrainInstance);
    return terrainInstance;
}

std::shared_ptr<udsdx::SceneObject> EnvironmentRenderer::AddHarvestObject(const nlohmann::json& instance)
{
    const float terrainScale = 1.0f;
    const float instanceScale = 0.01f;

    std::shared_ptr<SceneObject> harvestObj = std::make_shared<SceneObject>();

    Vector3 position = Vector3(instance["position"]["x"], instance["position"]["y"], instance["position"]["z"]);
    Quaternion rotation = Quaternion(instance["rotation"]["x"], instance["rotation"]["y"], instance["rotation"]["z"], instance["rotation"]["w"]);
    Vector3 scale = Vector3(instance["scale"]["x"], instance["scale"]["y"], instance["scale"]["z"]);
    scale *= Vector3(-1.0f, 1.0f, -1.0f) * instanceScale;
    position *= terrainScale;

    // Add models
    for (auto& model : instance["models"])
    {
        std::string filename = model["prefab"];
        if (m_prefabMap.find(filename) == m_prefabMap.end())
            continue;
        std::filesystem::path path = m_prefabMap[filename];

        auto harvestModelObj = std::make_shared<SceneObject>();

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

            if (auto it = m_textureMap.find(aKey); it != m_textureMap.end())
                material->SetSourceTexture(it->second);
            else
                material->SetSourceTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"Sprite-0001.png")));
            if (auto it = m_textureMap.find(nKey); it != m_textureMap.end())
                material->SetSourceTexture(it->second, 1);
            else
                material->SetSourceTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"Sprite-0001.png")), 1);

            m_instanceMaterials.emplace_back(material);
            materials.emplace_back(material);
        }

        Vector3 position = Vector3(model["position"]["x"], model["position"]["y"], model["position"]["z"]);
        Quaternion rotation = Quaternion(model["rotation"]["x"], model["rotation"]["y"], model["rotation"]["z"], model["rotation"]["w"]);
        Vector3 scale = Vector3(model["scale"]["x"], model["scale"]["y"], model["scale"]["z"]);

        // Set relative transform for the model.
        harvestModelObj->GetTransform()->SetLocalPosition(position);
        harvestModelObj->GetTransform()->SetLocalRotation(rotation);
		harvestModelObj->GetTransform()->SetLocalScale(scale);

        auto harvestRenderer = harvestModelObj->AddComponent<MeshRenderer>();
        harvestRenderer->SetMesh(mesh);
        harvestRenderer->SetShader(INSTANCE(Resource)->Load<udsdx::Shader>(RESOURCE_PATH(L"color.hlsl")));
        for (size_t i = 0; i < materials.size(); ++i)
            harvestRenderer->SetMaterial(materials[i].get(), static_cast<int>(i));

        harvestObj->AddChild(harvestModelObj);
    }

    auto interactiveEntity = harvestObj->AddComponent<InteractiveEntity>();
    interactiveEntity->SetInteractionText(L"채집하기");
    interactiveEntity->SetInteractionCallback([]() { Send(Create_c2s_CHANGE_HARVEST_STATE()); });

    harvestObj->GetTransform()->SetLocalPosition(position);
    harvestObj->GetTransform()->SetLocalRotation(rotation);
    harvestObj->GetTransform()->SetLocalScale(scale);
    harvestObj->SetActive(false);

    return harvestObj;
}