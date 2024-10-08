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
	// TODO: ���߿� ������ �ƿ� ���θ����߰ڳ�? �̹� �ִ��Ÿ� Active�� ���Ѿ߰ڳ�? ������ �ʿ��ϴ�.
	// Object Pooling�� �ʿ���. ����ϰ� ���� ��Ȱ��ȭ ���°� �Ǹ� �ٽ� Ǯ�� �����̳ʷ� ������.
	// ServerObjectMgr�� ���� ��Ʈ��ũ�κ��� ������ ��ü�� �����ֱⰡ �����Ǿ����� �������� ����

	// ���� �����Ǿ� ���� �ʴٸ� �߰��� �� ����.
	if (targetScene == nullptr)
	{
		std::cout << "Target Scene is not set" << std::endl;
		return;
	}

	if (false == m_mapServerObj.contains(b->obj_id))
	{
		auto instance = EntityBuilderBase::CreateObject(b); // ��ϵ� �Լ��� �˾Ƽ� �����ؼ� ��´�.

		// �������� ������� �� ������Ʈ�� '������' ServerObject Component�� �߰��� �����̾�� �Ѵ�.
		m_mapServerObj.emplace(b->obj_id, instance->GetComponent<ServerObject>()); // ���� ������Ʈ�� �߰��Ѵ�.
		targetScene->AddObject(instance);
	}
}

void ServerObjectMgr::RemoveObject(const uint64_t id)
{
	// �������� ������ �ִ� ���� �� ���� ���� �� ���� (���� Ŭ���̾�Ʈ Ư��)
	// ���� ������ ���Ͼ� ��ġ�� ����ϴ�.
	// ���� Ŭ���̾�Ʈ ���� õ���� �ִ� �Q�� �ϴ�����

	// ���� �����Ǿ� ���� �ʴٸ� ������ �� ����.

	const auto iter = m_mapServerObj.find(id);
	if (m_mapServerObj.cend() != iter)
	{
		iter->second->GetSceneObject()->RemoveFromParent();
		m_mapServerObj.erase(iter);
	}

	// ���µ� ������ ���� ���� �̻��� ��Ȳ�̴�.
	// ���� ���µ� ������ �ߴٰų� �̷� ������� ���� ���ۿ� ���� ����ó����
	// �������� ��κ��� ���� ��κп� �ʿ��ϴ�

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
	// ServerObjectMgr���� AddObject�� �Ǿ��� �� �߰��ؾ��� ���� ������ �־�� �Ѵ�.
	// ���� ���� �����Ǿ� ���� �ʴٸ� AddObject�� �����ؾ� �Ѵ�.

	// �κ� ȭ������ �ٽ� ��ȯ�ϴ� ��Ȳ ���� ����� ��, �ٸ� �ν��Ͻ��� ���� �ٽ� ������ �� �ִµ�,
	// �׷� ��� ������ �����Ǿ��� ������ ����ߴ� ������ ��� ����ų�, ���ο� ������ �� ������ �״�� �������Բ� �� �� �ְ� �ؾ��Ѵ�.
	// ���Ἲ ������ ����

	targetScene = scene;
}
