#include "pch.h"
#include "GuideSystem.h"
#include "NavigationMesh.h"
#include "Navigator.h"
#include "GizmoSphereRenderer.h"
#include "ServerObject.h"
#include "NaviAgent.h"
#include "EntityMovement.h"
#include "GizmoCylinderRenderer.h"
#include "InteractiveEntity.h"
#include "ServerObjectMgr.h"

using namespace udsdx;

GuideSystem::GuideSystem()
{
	// 교체 필요
	static const auto mesh_res = INSTANCE(udsdx::Resource)->Load<udsdx::Mesh>(RESOURCE_PATH(L"path_arrow.yms"));
	static const auto material_res = INSTANCE(udsdx::Resource)->Load<udsdx::Shader>(RESOURCE_PATH(L"colornotexpath.hlsl"));
	SetPathObjMaker([&](const std::vector<Vector3>& v)noexcept {
		
		const int diff = (int)v.size() - (int)m_guide_objects.size();
		if (0 < diff)
		{
			for (int i = 0; i < diff; ++i) 
			{
				auto s = udsdx::SceneObject::MakeShared();
				auto meshRenderer = s->AddComponent<udsdx::MeshRenderer>();
				meshRenderer->SetCastShadow(false);
				meshRenderer->SetMesh(mesh_res);
				meshRenderer->SetMaterial(material_res);
				GuideSystem::GetInst()->m_guide_objects.emplace_back(s);
				GuideSystem::GetInst()->m_targetScene->AddObject(s);
				m_guide_objects[i]->GetTransform()->SetLocalScale(Vector3::One * 0.5f);
			}
		}
		const auto num = (int)m_guide_objects.size();
		for (int i = 1; i < num; ++i)
		{
			Vector3 wv = wv = v[i] - v[i - 1];
			wv.Normalize();
			Vector3 vv = Vector3::Up;
			Vector3 uv = vv.Cross(wv);
			uv.Normalize();
			vv = wv.Cross(uv);
			const Matrix4x4 m = Matrix4x4(uv, vv, wv);

			const auto position = v[i];
			const auto transform = m_guide_objects[i]->GetTransform();
			transform->SetLocalPosition(position + Vector3::Up * 0.5f);
			transform->SetLocalRotation(Quaternion::CreateFromRotationMatrix(m));
		}
		});
}

GuideSystem::~GuideSystem()
{
}

void GuideSystem::SetGuidePath(const std::shared_ptr<udsdx::SceneObject>& target)
{
	if (false == (m_guide_active_flag = !m_guide_active_flag))
	{
		ResetGuideObjects();
		return;
	}
	SetGuidePathInternal(target->GetTransform()->GetLocalPosition());
}

void GuideSystem::SetGuidePath(const Vector3& target_pos)
{
	if (false == (m_guide_active_flag = !m_guide_active_flag))
	{
		ResetGuideObjects();
		return;
	}
	SetGuidePathInternal(target_pos);
}

void GuideSystem::ResetGuideObjects()
{
	for (const auto& o : m_guide_objects)o->RemoveFromParent();
	m_guide_objects.clear();
}

void GuideSystem::AddHarvestMeshObject(std::shared_ptr<udsdx::SceneObject> obj)
{
	m_mapHarvest.emplace_back(obj);
}

bool GuideSystem::AddHarvest(uint32_t server_id, uint32_t harvest_id, bool is_active)
{
	m_mapHarvestID.emplace(server_id, harvest_id);
	if (m_in_active_list.erase(harvest_id))
	{
		// 만약 이전에 채집물 비활성 패킷을 먼저 받은 기록이 있다면
		is_active = false;
	}
	else if (m_active_list.erase(harvest_id))
	{
		// 만약 이전에 채집물 활성 패킷을 먼저 받은 기록이 있다면
		is_active = true;
	}
	else
	{
		// 그냥 정상적으로 받은 패킷이라면 패킷에 적힌 활성 여부를 그대로 뱉음
	}

	const auto harvest = GetHarvest(server_id);
	if (nullptr != harvest)
	{
		harvest->SetActive(true);
		SetHarvestState(server_id, harvest_id, is_active);
	}

	return is_active;
}

void GuideSystem::RemoveHarvest(uint32_t server_id)
{
	// 채집물이 내 시야에서 사라질 땐 그냥 관련정보 다 날리고 다시 시작
	const auto iter = m_mapHarvestID.find(server_id);
	if (m_mapHarvestID.end() != iter)
	{
		const auto harvest_id = iter->second;

		m_active_list.erase(harvest_id);
		m_in_active_list.erase(harvest_id);
	}

	const auto harvest = GetHarvest(server_id);
	if (nullptr != harvest)
	{
		harvest->SetActive(false);
	}

	m_mapHarvestID.erase(server_id);
}

void GuideSystem::ClearHarvest() noexcept
{
	m_mapHarvest.clear();
	m_mapHarvestID.clear();
	m_active_list.clear();
	m_in_active_list.clear();
}

const bool GuideSystem::SetHarvestState(const uint32_t server_id, const uint32_t harvest_id, const bool is_active) noexcept
{
	// APPEAR OBJECT 패킷에 액티브상태라고 표시후 보내기직전에
	// 누군가 채집해서 인액티브가되어서 인액티브사실을 먼저 알리는 패킷이 도착해버림
	// 아직 이 클라는 채집물이 없는 상황
	// 채집물 인액티브 패킷은 무시되고, 어피어오브젝트를 받았을 때 패킷 제작 시 넣은 정보가 액티브여서 오차 발생
	// 만약 내가 모르는 채집물인데 채집물 상태변화 패킷이 와버렸다면 버리지말고 기억
	//std::cout << "SetHarvestState: server_id: " << server_id
	//	<< ", harvest_id: " << harvest_id
	//	<< ", is_active: " << is_active << std::endl;
	if (0 > harvest_id || m_mapHarvest.size() <= static_cast<size_t>(harvest_id))
		return false; // 유효하지 않은 harvest_id
	const auto harvest = m_mapHarvest[harvest_id].get();

	if (true == harvest->GetActive())
	{
		// TODO: 상태를 바꾼다.
		// harvest->GetComponent<GizmoCylinderRenderer>()->SetActive(false);
		auto interactiveEntity = harvest->GetComponent<InteractiveEntity>();
		interactiveEntity->SetActive(is_active);
		interactiveEntity->SetInteractionCallback([server_id]() {
			// TODO: 플레이어가 상호작용을 통해 채집물을 채집했을 때의 코드 영역
			// 프로토콜에서 채집한 채집물의 server_id를 추가적으로 정의해주어야 한다.
			// 여기서 server_id는 채집물 매핑 id가 아닌 server object 고유 id이다.
			// 
			 Send(Create_c2s_CHANGE_HARVEST_STATE(server_id));
			});

		Shader* shader = nullptr;
		if (is_active)
			shader = INSTANCE(Resource)->Load<udsdx::Shader>(RESOURCE_PATH(L"colorhighlight.hlsl"));
		else
			shader = INSTANCE(Resource)->Load<udsdx::Shader>(RESOURCE_PATH(L"color.hlsl"));
		for (MeshRenderer* renderer : harvest->GetComponentsInChildren<MeshRenderer>())
		{
			size_t size = renderer->GetMesh()->GetSubmeshes().size();
			for (size_t i = 0; i < size; ++i)
			{
				udsdx::Material material = renderer->GetMaterial(static_cast<int>(i));
				material.SetShader(shader);
				renderer->SetMaterial(material, static_cast<int>(i));
			}
		}
		return true;
	}
	else
	{
		// 채집물이 아직 이 클라에 안보이지만 누가 건드려서 상태변경 패킷이 먼저 온 경우
		// 그 히스토리를 저장함
		if (is_active)
		{
			m_active_list.emplace(harvest_id);
		}
		else
		{
			m_in_active_list.emplace(harvest_id);
		}
		return false;
	}
}

void GuideSystem::UpdateGuideSystem()
{
	if (!m_guide_active_flag)return;
	const auto pos = m_main_hero->GetTransform()->GetLocalPosition();
	Vector3 v1 = {};
	float min_dist = std::numeric_limits<float>::max();
	for (const auto& obj : m_mapHarvest)
	{
		if (const auto interaction = obj->GetComponent<InteractiveEntity>())
		{
			if (!interaction->GetActive())continue;
			const auto obj_pos = obj->GetTransform()->GetLocalPosition();
			const auto dist = Vector3::DistanceSquared(pos, obj_pos);
			if (dist < min_dist)
			{
				v1 = obj_pos;
				min_dist = dist;
			}
		}
	}
	if (temp_force_pos != Vector3::Zero)
	{
		v1 = temp_force_pos;
	}
	const auto path_len_sq = Vector3::DistanceSquared(pos, v1);
	if (20.f * 20.f < path_len_sq)
	{
		m_pathFactor = 4.f;
	}
	else
	{
		m_pathFactor = 1.f;
	}
	if (v1 != Vector3::Zero)
	{
		ResetGuideObjects();
		SetGuidePathInternal(v1);
	}
}

void GuideSystem::SetClearTreeInteraction(const bool active_flag) noexcept
{
	m_clear_tree_obj->GetComponent<InteractiveEntity>()->SetActive(active_flag);
}

void GuideSystem::SetGuidePathInternal(const Vector3& target_pos)
{
	if (temp_force_pos == Vector3::Zero)
		m_cur_target_pos = target_pos;
	else
		m_cur_target_pos = temp_force_pos;
	const auto pos = m_main_hero->GetTransform()->GetLocalPosition();
	const auto navi = m_main_hero->GetComponent<ServerObject>()->GetNaviAgent();
	const auto prev_pos = m_main_hero->GetComponent<EntityMovement>()->prev_pos;
	Vector3 temp = pos;
	navi->SetCellPos(DT, pos, pos, temp);
	// TODO: 점 사이 사이 간격의 길이가 매직넘버
	if (const auto main_hero = ServerObjectMgr::GetInst()->GetMainHero())
	{
		if (const auto nav_mesh = main_hero->GetNaviAgent()->GetNavMesh())
		{
			const auto& v = nav_mesh->GetPathVertices(
				pos, target_pos, m_pathFactor);
			m_path_obj_maker(v);
		}
	}
}
