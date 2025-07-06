#include "pch.h"
#include "ServerObject.h"
#include "ServerComponent.h"
#include "MoveInterpolator.h"
#include "NaviAgent.h"
#include "Navigator.h"

ServerObject::ServerObject(const std::shared_ptr<SceneObject>& object) : Component(object)
{
	// TODO: 앞으로 유동적으로 바뀌어야함
	m_pNaviAgent = new Common::NaviAgent;
	m_pNaviAgent->SetNavMesh(NAVI_MESH_TYPE::MAIN_WORLD);
}

ServerObject::~ServerObject()
{
	delete m_pNaviAgent;
}

void ServerObject::Update(const udsdx::Time& time, udsdx::Scene& scene)
{
	ServerCompUpdateALL();
}
 
void ServerObject::SetNavigationMesh(const NAVI_MESH_TYPE eType) noexcept
{
	m_pNaviAgent->SetNavMesh(eType);
}

ServerComponent* const ServerObject::AddComp(const uint64_t comp_id, ServerComponent* const pComp) noexcept
{
	NET_NAGOX_ASSERT(m_mapServerComp.try_emplace(comp_id, pComp).second);
	return pComp;
}
