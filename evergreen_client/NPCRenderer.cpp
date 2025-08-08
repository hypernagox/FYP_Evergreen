#include "pch.h"
#include "NPCRenderer.h"
#include "GizmoSphereRenderer.h"
#include "EntityMovement.h"

using namespace udsdx;

void NPCRenderer::OnInitialize()
{
    auto resource = INSTANCE(Resource);

	m_entityMovement = GetSceneObject()->AddComponent<EntityMovement>();

	m_rendererObject = udsdx::SceneObject::MakeShared();
    m_rendererObject->GetTransform()->SetLocalScale(0.05f);
	m_rendererObject->GetTransform()->SetLocalPosition(0.0f, -0.225f, 0.0f);

    auto npcRenderer = m_rendererObject->AddComponent<RiggedMeshRenderer>();
    npcRenderer->SetMesh(resource->Load<udsdx::RiggedMesh>(RESOURCE_PATH(L"npc\\npc_tpose.yrms")));
    npcRenderer->SetMaterial(udsdx::Material(resource->Load<udsdx::Shader>(RESOURCE_PATH(L"nprcolor.hlsl")), resource->Load<udsdx::Texture>(RESOURCE_PATH(L"npc\\npc_diffuse_0.png"))), 1);
    npcRenderer->SetMaterial(udsdx::Material(resource->Load<udsdx::Shader>(RESOURCE_PATH(L"nprcolor.hlsl")), resource->Load<udsdx::Texture>(RESOURCE_PATH(L"npc\\npc_diffuse_1.png"))), 0);
    npcRenderer->SetMaterial(udsdx::Material(resource->Load<udsdx::Shader>(RESOURCE_PATH(L"nprcolor.hlsl")), resource->Load<udsdx::Texture>(RESOURCE_PATH(L"npc\\npc_diffuse_2.png"))), 2);
    npcRenderer->SetAnimation(resource->Load<udsdx::AnimationClip>(RESOURCE_PATH(L"npc\\npc_Idle.yac")), true, true);

	m_stateMachine = std::make_unique<Common::StateMachine<AnimationState>>(AnimationState::Idle);

	m_stateMachine->AddTransition<Common::FloatStateTransition<AnimationState, std::greater<float>>>(AnimationState::Idle, AnimationState::Run, m_stateMachine->GetConditionRefFloat("Speed"), 1e-3f);
	m_stateMachine->AddTransition<Common::FloatStateTransition<AnimationState, std::less_equal<float>>>(AnimationState::Run, AnimationState::Idle, m_stateMachine->GetConditionRefFloat("Speed"), 1e-3f);

	m_stateMachine->AddOnStateChangeCallback([this](AnimationState from, AnimationState to) { this->OnAnimationStateChange(from, to); });

	GetSceneObject()->AddChild(m_rendererObject);
}

void NPCRenderer::Update(const udsdx::Time& time, udsdx::Scene& scene)
{
	Vector3 delta = m_entityMovement->GetPosition() - m_lastPosition;

	float distance = delta.Length();
	*m_stateMachine->GetConditionRefFloat("Speed") = distance;
	m_lastPosition = m_entityMovement->GetPosition();
	m_stateMachine->Update(time.deltaTime);

	if (distance > 1e-3f)
	{
		float rotation = std::atan2(delta.x, delta.z);
		Quaternion currentRotation = m_rendererObject->GetTransform()->GetLocalRotation();
		Quaternion targetRotation = Quaternion::CreateFromYawPitchRoll(rotation, 0.0f, 0.0f);
		m_rendererObject->GetTransform()->SetLocalRotation(Quaternion::Slerp(currentRotation, targetRotation, time.deltaTime * 4.0f));
	}
}

void NPCRenderer::OnAnimationStateChange(AnimationState from, AnimationState to)
{
	switch (to)
	{
	case NPCRenderer::AnimationState::Idle:
		m_rendererObject->GetComponent<RiggedMeshRenderer>()->SetAnimation(INSTANCE(Resource)->Load<udsdx::AnimationClip>(RESOURCE_PATH(L"npc\\npc_Idle.yac")), true, true);
		break;
	case NPCRenderer::AnimationState::Run:
		m_rendererObject->GetComponent<RiggedMeshRenderer>()->SetAnimation(INSTANCE(Resource)->Load<udsdx::AnimationClip>(RESOURCE_PATH(L"npc\\npc_run.yac")), true, true);
		break;
	}
}
