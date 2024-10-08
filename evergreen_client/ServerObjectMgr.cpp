#include "pch.h"
#include "ServerObjectMgr.h"
#include "ServerObject.h"
#include "component.h"
#include "MoveInterpolator.h"
#include "EntityBuilder.h"

ServerObjectMgr::ServerObjectMgr()
{
}

ServerObjectMgr::~ServerObjectMgr()
{
}

void ServerObjectMgr::AddObject(EntityBuilderBase* b)
{
	// TODO: 나중엔 없으면 아예 새로만들어야겠네? 이미 있던거면 Active만 시켜야겠네? 구조가 필요하다.
	// Object Pooling이 필요함. 사용하고 나서 비활성화 상태가 되면 다시 풀링 컨테이너로 들어가야함.
	// ServerObjectMgr에 의해 네트워크로부터 생성된 객체의 생명주기가 관리되어지는 조건으로 간주

	// 씬이 지정되어 있지 않다면 추가할 수 없다.
	if (targetScene == nullptr)
	{
		std::cout << "Target Scene is not set" << std::endl;
		return;
	}

	if (false == m_mapServerObj.contains(b->obj_id))
	{
		auto instance = EntityBuilderBase::CreateObject(b); // 등록된 함수가 알아서 생성해서 뱉는다.

		// 빌더에서 만들어진 씬 오브젝트는 '무조건' ServerObject Component를 추가한 상태이어야 한다.
		m_mapServerObj.emplace(b->obj_id, instance->GetComponent<ServerObject>()); // 서버 오브젝트를 추가한다.
		targetScene->AddObject(instance);
	}
}

void ServerObjectMgr::RemoveObject(const uint64_t id)
{
	// 생각보다 씬에서 넣다 뺏다 할 일이 많을 것 같다 (더미 클라이언트 특성)
	// 현재 씬에서 리니어 서치는 곤란하다.
	// 더미 클라이언트 지금 천개씩 넣다 뻈다 하는중임

	// 씬이 지정되어 있지 않다면 삭제할 수 없다.

	const auto iter = m_mapServerObj.find(id);
	if (m_mapServerObj.cend() != iter)
	{
		iter->second->GetSceneObject()->RemoveFromParent();
		m_mapServerObj.erase(iter);
	}

	// 없는데 지우라고 오면 뭔가 이상한 상황이다.
	// 뭔가 없는데 지우라고 했다거나 이런 기대하지 않은 동작에 대한 예외처리가
	// 앞으로의 대부분의 로직 대부분에 필요하다

	if (targetScene == nullptr)
	{
		std::cout << "Target Scene is not set" << std::endl;
		return;
	}

	
}

Component* const ServerObjectMgr::GetServerObjComp(const uint64_t id) const noexcept
{
	return GetServerObj(id);
}

SceneObject* const ServerObjectMgr::GetServerObjRoot(const uint64_t id) const noexcept
{
	return GetServerObj(id)->GetSceneObject().get();
}

void ServerObjectMgr::SetTargetScene(const std::shared_ptr<Scene>& scene) noexcept
{
	// ServerObjectMgr에서 AddObject가 되었을 때 추가해야할 씬을 지정해 주어야 한다.
	// 만약 씬이 지정되어 있지 않다면 AddObject가 실패해야 한다.

	// 로비 화면으로 다시 전환하는 상황 등을 고려할 때, 다른 인스턴스의 씬을 다시 지정할 수 있는데,
	// 그런 경우 이전에 지정되었던 씬에서 사용했던 정보를 모두 지우거나, 새로운 씬에서 그 정보를 그대로 가져오게끔 할 수 있게 해야한다.
	// 무결성 유지에 유의

	targetScene = scene;
}
