#include "pch.h"
#include "EnvironmentRenderer.h"
#include <HeightMap.h>
#include "TerrainData.h"
#include "TerrainInstanceRenderer.h"
#include "InteractiveEntity.h"
#include "TerrainDetailRenderer.h"

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

void EnvironmentRenderer::Initialize(const EnvironmentParameters& parameters)
{
    auto res = INSTANCE(Resource);

    const float TerrainSize = parameters.TerrainSize;
    const float TerrainHeight = parameters.TerrainHeight;
    const float TerrainOffset = parameters.TerrainOffset;

    const Vector3 terrainPos = Vector3(TerrainOffset, 0, TerrainOffset);
    const Vector3 terrainScale = Vector3::One * TerrainSize;

    m_terrainMesh = CreateMeshFromHeightMap(parameters.HeightMap, 512, 512, TerrainHeight / TerrainSize);
    m_terrainMesh->UploadBuffers(INSTANCE(Core)->GetDevice(), INSTANCE(Core)->GetCommandList());

    udsdx::Material terrainMaterial(res->Load<Shader>(RESOURCE_PATH(L"terrain.hlsl")));

    terrainMaterial.SetSourceTexture(parameters.TerrainSplatMaps[0], 0);
    terrainMaterial.SetSourceTexture(parameters.TerrainSplatMaps[1], 1);

    terrainMaterial.SetSourceTexture(parameters.TerrainDiffuseMaps[0], 2);
    terrainMaterial.SetSourceTexture(parameters.TerrainDiffuseMaps[1], 3);
    terrainMaterial.SetSourceTexture(parameters.TerrainDiffuseMaps[2], 4);
    terrainMaterial.SetSourceTexture(parameters.TerrainDiffuseMaps[3], 5);
    terrainMaterial.SetSourceTexture(parameters.TerrainDiffuseMaps[4], 6);
    terrainMaterial.SetSourceTexture(parameters.TerrainDiffuseMaps[5], 7);
    terrainMaterial.SetSourceTexture(parameters.TerrainDiffuseMaps[6], 8);

    terrainMaterial.SetSourceTexture(parameters.TerrainNormalMaps[0], 9);
    terrainMaterial.SetSourceTexture(parameters.TerrainNormalMaps[1], 10);
    terrainMaterial.SetSourceTexture(parameters.TerrainNormalMaps[2], 11);
    terrainMaterial.SetSourceTexture(parameters.TerrainNormalMaps[3], 12);
    terrainMaterial.SetSourceTexture(parameters.TerrainNormalMaps[4], 13);
    terrainMaterial.SetSourceTexture(parameters.TerrainNormalMaps[5], 14);
    terrainMaterial.SetSourceTexture(parameters.TerrainNormalMaps[6], 15);

    if (parameters.TerrainDetail != nullptr)
    {
        std::shared_ptr<SceneObject> terrainDetailObj = std::make_shared<SceneObject>();
        auto terrainDetailRenderer = terrainDetailObj->AddComponent<TerrainDetailRenderer>();
        terrainDetailRenderer->SetTerrainDetail(parameters.TerrainDetail);
        terrainDetailRenderer->SetMaterial(udsdx::Material(res->Load<udsdx::Shader>(RESOURCE_PATH(L"detailbillboard.hlsl")), res->Load<udsdx::Texture>(RESOURCE_PATH(L"environment\\Grass.tga"))));

        terrainDetailObj->GetTransform()->SetLocalPosition(terrainPos);
        terrainDetailObj->GetTransform()->SetLocalScale(Vector3(TerrainSize, TerrainHeight, TerrainSize));

        GetSceneObject()->AddChild(terrainDetailObj);
    }

    if (parameters.TerrainData != nullptr)
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
            if (parameters.TerrainData->GetPrototypeInstanceCount(filename) > 0)
                AddTerrainInstances(directory.path(), parameters.TerrainData);
            m_prefabMap[filename] = directory.path();
        }
    }

    if (parameters.HeightMap != nullptr)
    {
        m_terrainObj = std::make_shared<SceneObject>();
        m_terrainObj->GetTransform()->SetLocalPosition(terrainPos);
        m_terrainObj->GetTransform()->SetLocalScale(terrainScale);
        auto terrainRenderer = m_terrainObj->AddComponent<MeshRenderer>();
        terrainRenderer->SetMesh(m_terrainMesh.get());
        terrainRenderer->SetMaterial(terrainMaterial);

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
        udsdx::Material material(INSTANCE(Resource)->Load<udsdx::Shader>(RESOURCE_PATH(L"colorinstanced.hlsl")));
        std::string aKey = subMeshes[i].DiffuseTexturePath;
        std::string nKey = subMeshes[i].NormalTexturePath;
        std::transform(aKey.begin(), aKey.end(), aKey.begin(), ::tolower);
        std::transform(nKey.begin(), nKey.end(), nKey.begin(), ::tolower);

        // if the extension is .psd, replace it with .tga
        if (aKey.ends_with(".psd"))
            aKey.replace(aKey.end() - 4, aKey.end(), ".tga");
        if (nKey.ends_with(".psd"))
            nKey.replace(nKey.end() - 4, nKey.end(), ".tga");

        const char* branchKeywords[] = {
            "branch",
            "leaf",
            "grass",
            "weed",
            "flower",
            "plant",
            "lavender",
            "lily",
            "lilly",
            "papyrus"
        };
        for (const auto& keyword : branchKeywords)
        {
            if (aKey.find(keyword) != std::string::npos || nKey.find(keyword) != std::string::npos)
            {
				material.SetShader(INSTANCE(Resource)->Load<udsdx::Shader>(RESOURCE_PATH(L"colorbranchinstanced.hlsl")));
				break;
			}
		}

        if (auto it = m_textureMap.find(aKey); it != m_textureMap.end())
            material.SetSourceTexture(it->second);
        else
            material.SetSourceTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"Sprite-0001.png")));
        if (auto it = m_textureMap.find(nKey); it != m_textureMap.end())
            material.SetSourceTexture(it->second, 1);
        else
            material.SetSourceTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"Sprite-0001.png")), 1);

        terrainInstanceRenderer->SetMaterial(material, static_cast<int>(i));
    }

    terrainInstanceRenderer->SetTerrainData(terrainData, path.filename().stem().string());
    terrainInstanceRenderer->SetMesh(mesh);

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
        std::vector<udsdx::Material> materials;

        const auto& subMeshes = mesh->GetSubmeshes();
        for (size_t i = 0; i < subMeshes.size(); ++i)
        {
            udsdx::Material material(INSTANCE(Resource)->Load<udsdx::Shader>(RESOURCE_PATH(L"color.hlsl")));
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
                material.SetSourceTexture(it->second);
            else
                material.SetSourceTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"Sprite-0001.png")));
            if (auto it = m_textureMap.find(nKey); it != m_textureMap.end())
                material.SetSourceTexture(it->second, 1);
            else
                material.SetSourceTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"Sprite-0001.png")), 1);

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
        for (size_t i = 0; i < materials.size(); ++i)
            harvestRenderer->SetMaterial(materials[i], static_cast<int>(i));

        harvestObj->AddChild(harvestModelObj);
    }

    auto interactiveEntity = harvestObj->AddComponent<InteractiveEntity>();
    interactiveEntity->SetInteractionText(L"채집하기");

    harvestObj->GetTransform()->SetLocalPosition(position);
    harvestObj->GetTransform()->SetLocalRotation(rotation);
    harvestObj->GetTransform()->SetLocalScale(scale);
    harvestObj->SetActive(false);

    return harvestObj;
}