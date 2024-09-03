#pragma once
#include "pch.h"
#include "Singleton.hpp"
#include "EntityBuilder.h"

class ServerObject;
using udsdx::Scene;
using udsdx::Component;
using udsdx::SceneObject;

class ServerObjectMgr
	:public NetHelper::Singleton<ServerObjectMgr>
{
	friend class Singleton;
	ServerObjectMgr();
	~ServerObjectMgr();
public:
	// TODO: ����� ����� �����ϰ� �ٸ��� �̰� �����ε� �� �� ���
	void AddObject(EntityBuilderBase* b);
	void RemoveObject(const uint64_t id);
	ServerObject* const GetServerObj(const uint64_t id) const {
		const auto iter = m_mapServerObj.find(id);
		return m_mapServerObj.cend() != iter ? iter->second : nullptr;
	}
	Component* const GetServerObjComp(const uint64_t id) const noexcept;
	SceneObject* const GetServerObjRoot(const uint64_t id) const noexcept;

	void SetTargetScene(const std::shared_ptr<Scene>& scene) noexcept;

private:
	std::shared_ptr<Scene> targetScene;
	std::unordered_map<uint64_t, ServerObject*> m_mapServerObj;
};

