#include "pch.h"
#include "GuideSystem.h"
#include "NavigationMesh.h"
#include "Navigator.h"
#include "GizmoSphereRenderer.h"
#include "ServerObject.h"
#include "NaviAgent.h"
#include "EntityMovement.h"

GuideSystem::GuideSystem()
{
	// 교체 필요
	SetPathObjMaker([](const std::vector<Vector3>& v) {
		for (int i = 0; i < v.size(); ++i) {
			const auto vv = v[i];
			auto s = std::make_shared<udsdx::SceneObject>();
			auto gizmoRenderer = s->AddComponent<GizmoSphereRenderer>();
			gizmoRenderer->SetRadius(1.0f);
			s->GetTransform()->SetLocalPosition(vv);
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

void GuideSystem::UpdateGuideSystem()
{
	if (!m_guide_active_flag)return;
	const auto pos = m_main_hero->GetTransform()->GetLocalPosition();
	Vector3 v1 = {};
	float min_dist = std::numeric_limits<float>::max();
	for (const auto& [id, obj] : m_mapHarvest)
	{
		const auto dist = Vector3::DistanceSquared(pos, obj->GetTransform()->GetLocalPosition());
		if (dist <= min_dist) {
			v1 = obj->GetTransform()->GetLocalPosition();
			min_dist = dist;
		}
	}
	if (v1 != Vector3::Zero)
	{
		ResetGuideObjects();
		SetGuidePathInternal(v1);
	}
}

void GuideSystem::SetGuidePathInternal(const Vector3& target_pos)
{
	m_cur_target_pos = target_pos;
	const auto pos = m_main_hero->GetTransform()->GetLocalPosition();
	const auto navi = m_main_hero->GetComponent<ServerObject>()->m_pNaviAgent;
	const auto prev_pos = m_main_hero->GetComponent<EntityMovement>()->prev_pos;
	Vector3 temp = pos;
	navi->SetCellPos(DT, prev_pos, pos, temp);
	// TODO: 점 사이 사이 간격의 길이가 매직넘버
	const auto& v = NAVIGATION->GetNavMesh(NAVI_MESH_NUM::NUM_0)->GetPathVertices(
		temp, target_pos, 2.f);
	m_path_obj_maker(v);
}
