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

using namespace udsdx;

GuideSystem::GuideSystem()
{
	// ��ü �ʿ�
	SetPathObjMaker([](const std::vector<Vector3>& v) {
		
		for (int i = 1; i < v.size(); ++i) {
			Vector3 wv = wv = v[i] - v[i - 1];
			wv.Normalize();
			Vector3 vv = Vector3::Up;
			Vector3 uv = vv.Cross(wv);
			uv.Normalize();
			vv = wv.Cross(uv);
			Matrix4x4 m = Matrix4x4(uv, vv, wv);

			const auto position = v[i];
			auto s = std::make_shared<udsdx::SceneObject>();
			auto meshRenderer = s->AddComponent<udsdx::MeshRenderer>();
			meshRenderer->SetCastShadow(false);
			meshRenderer->SetMesh(INSTANCE(udsdx::Resource)->Load<udsdx::Mesh>(RESOURCE_PATH(L"path_arrow.obj")));
			meshRenderer->SetShader(INSTANCE(udsdx::Resource)->Load<udsdx::Shader>(RESOURCE_PATH(L"colornotexpath.hlsl")));
			auto transform = s->GetTransform();
			transform->SetLocalPosition(position + Vector3::Up * 0.5f);
			transform->SetLocalRotation(Quaternion::CreateFromRotationMatrix(m));
			transform->SetLocalScale(Vector3::One * 0.5f);
			GuideSystem::GetInst()->m_guide_objects.emplace_back(s);
			GuideSystem::GetInst()->m_targetScene->AddObject(s);
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
		// ���� ������ ä���� ��Ȱ�� ��Ŷ�� ���� ���� ����� �ִٸ�
		is_active = false;
	}
	else if (m_active_list.erase(harvest_id))
	{
		// ���� ������ ä���� Ȱ�� ��Ŷ�� ���� ���� ����� �ִٸ�
		is_active = true;
	}
	else
	{
		// �׳� ���������� ���� ��Ŷ�̶�� ��Ŷ�� ���� Ȱ�� ���θ� �״�� ����
	}

	const auto harvest = GetHarvest(server_id);
	if (nullptr != harvest)
	{
		harvest->SetActive(true);
		SetHarvestState(harvest_id, is_active);
	}

	return is_active;
}

void GuideSystem::RemoveHarvest(uint32_t server_id)
{
	// ä������ �� �þ߿��� ����� �� �׳� �������� �� ������ �ٽ� ����
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

const bool GuideSystem::SetHarvestState(const uint32_t harvest_id, const bool is_active) noexcept
{
	// APPEAR OBJECT ��Ŷ�� ��Ƽ����¶�� ǥ���� ������������
	// ������ ä���ؼ� �ξ�Ƽ�갡�Ǿ �ξ�Ƽ������ ���� �˸��� ��Ŷ�� �����ع���
	// ���� �� Ŭ��� ä������ ���� ��Ȳ
	// ä���� �ξ�Ƽ�� ��Ŷ�� ���õǰ�, ���Ǿ������Ʈ�� �޾��� �� ��Ŷ ���� �� ���� ������ ��Ƽ�꿩�� ���� �߻�
	// ���� ���� �𸣴� ä�����ε� ä���� ���º�ȭ ��Ŷ�� �͹��ȴٸ� ���������� ���

	if (0 > harvest_id || m_mapHarvest.size() <= static_cast<size_t>(harvest_id))
		return false; // ��ȿ���� ���� harvest_id
	const auto harvest = m_mapHarvest[harvest_id].get();

	if (true == harvest->GetActive())
	{
		// TODO: ���¸� �ٲ۴�.
		// harvest->GetComponent<GizmoCylinderRenderer>()->SetActive(false);
		harvest->GetComponent<InteractiveEntity>()->SetActive(is_active);
		Shader* shader = nullptr;
		if (is_active)
			shader = INSTANCE(Resource)->Load<udsdx::Shader>(RESOURCE_PATH(L"colorhighlight.hlsl"));
		else
			shader = INSTANCE(Resource)->Load<udsdx::Shader>(RESOURCE_PATH(L"color.hlsl"));
		harvest->GetComponent<MeshRenderer>()->SetShader(shader);
		return true;
	}
	else
	{
		// ä������ ���� �� Ŭ�� �Ⱥ������� ���� �ǵ���� ���º��� ��Ŷ�� ���� �� ���
		// �� �����丮�� ������
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
		const auto dist = Vector3::DistanceSquared(pos, obj->GetTransform()->GetLocalPosition());
		if (dist <= min_dist) {
			v1 = obj->GetTransform()->GetLocalPosition();
			min_dist = dist;
		}
	}
	if (temp_force_pos != Vector3::Zero)
	{
		v1 = temp_force_pos;
	}
	if (v1 != Vector3::Zero)
	{
		ResetGuideObjects();
		SetGuidePathInternal(v1);
	}
}

void GuideSystem::SetGuidePathInternal(const Vector3& target_pos)
{
	if (temp_force_pos == Vector3::Zero)
		m_cur_target_pos = target_pos;
	else
		m_cur_target_pos = temp_force_pos;
	const auto pos = m_main_hero->GetTransform()->GetLocalPosition();
	const auto navi = m_main_hero->GetComponent<ServerObject>()->m_pNaviAgent;
	const auto prev_pos = m_main_hero->GetComponent<EntityMovement>()->prev_pos;
	Vector3 temp = pos;
	navi->SetCellPos(DT, pos, pos, temp);
	// TODO: �� ���� ���� ������ ���̰� �����ѹ�
	const auto& v = NAVIGATION->GetNavMesh(NAVI_MESH_NUM::NUM_0)->GetPathVertices(
		pos, target_pos, 1.0f);
	m_path_obj_maker(v);
}
