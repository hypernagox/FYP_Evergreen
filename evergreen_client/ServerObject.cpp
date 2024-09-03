#include "pch.h"
#include "ServerObject.h"
#include "ServerComponent.h"
#include "MoveInterpolator.h"

ServerObject::ServerObject(const std::shared_ptr<SceneObject>& object) : Component(object)
{
}

void ServerObject::Update(const udsdx::Time& time, udsdx::Scene& scene)
{
	ServerCompUpdate<MoveInterpolator>();
}

ServerComponent* const ServerObject::AddComp(const uint64_t comp_id, ServerComponent* const pComp) noexcept
{
	NET_NAGOX_ASSERT(m_mapServerComp.try_emplace(comp_id, pComp).second);
	return pComp;
}
