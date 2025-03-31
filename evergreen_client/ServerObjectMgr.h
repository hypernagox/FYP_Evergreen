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
	// TODO: 만드는 방법이 현저하게 다르면 이걸 오버로딩 할 까 고민
	void AddObject(EntityBuilderBase* b);
	void AddObject(std::shared_ptr<SceneObject> scene_obj);
	void RemoveObject(const uint64_t id);
	ServerObject* const GetServerObj(const uint64_t id) const {
		const auto iter = m_mapServerObj.find(id);
		if (m_mapServerObj.cend() != iter)
			return iter->second->GetComponent<ServerObject>();
		return nullptr;
	}

	ServerObject* const GetServerObjExceptHero(const uint64_t id) const {
		if (id == m_mainHeroID)return nullptr;
		return GetServerObj(id);
	}

	Component* const GetServerObjComp(const uint64_t id) const noexcept;
	SceneObject* const GetServerObjRoot(const uint64_t id) const noexcept;

	void SetTargetScene(const std::shared_ptr<Scene>& scene) noexcept;
	void Clear()noexcept { m_mapServerObj.clear(); }
	void SetMainHero(const uint32_t id, std::shared_ptr<udsdx::SceneObject> hero) {
		if (m_mainHero) {
			throw std::runtime_error{ "Hero already exist" };
		}
		m_mainHeroID = id;
		m_mapServerObj.try_emplace(id, hero);
		m_mainHero = std::move(hero);
	}
	const auto GetMainHero()const noexcept { return m_mainHero->GetComponent<ServerObject>(); }
private:
	std::shared_ptr<Scene> targetScene;
	std::unordered_map<uint64_t, std::shared_ptr<udsdx::SceneObject>> m_mapServerObj;
	std::shared_ptr<udsdx::SceneObject> m_mainHero;
	uint64_t m_mainHeroID = 0;
};

