#pragma once


// 자원, 퀘 , NPC 등 클라입장에서의 가이드 시스템(길 안내)
class GuideSystem
	:public NetHelper::Singleton<GuideSystem>
{
	friend class Singleton;
	GuideSystem();
	~GuideSystem();
public:
	void SetGuidePath(const std::shared_ptr<udsdx::SceneObject>& target);
	void SetGuidePath(const Vector3& target_pos);

	void ResetGuideObjects();
public:
	void SetPathObjMaker(std::function<void(const std::vector<Vector3>&)> maker_) {
		m_path_obj_maker.swap(maker_);
	}
	void SetMainPlayer(std::shared_ptr<udsdx::SceneObject> player) { m_main_hero.swap(player); }
	void SetTargetScene(std::shared_ptr<udsdx::Scene> scene) { m_targetScene.swap(scene); }

public:

	void AddHarvest(uint64_t id, std::shared_ptr<udsdx::SceneObject> obj) {
		m_mapHarvest.emplace(id, obj);
	}
	void RemoveHarvest(uint64_t id) {
		m_mapHarvest.erase(id);
	}

	void UpdateGuideSystem();
	void ToggleFlag() {
		if (false == (m_guide_active_flag = !m_guide_active_flag)) {
			ResetGuideObjects();
		}
	}
	Vector3 temp_force_pos = {};
private:
	void SetGuidePathInternal(const Vector3& target_pos);
private:
	// 렌더링 하는 방법에 대한 함포 (어디그려야 할 지 알때)
	std::function<void(
		const std::vector<Vector3>&
	)> m_path_obj_maker;
	std::vector<std::shared_ptr<udsdx::SceneObject>> m_guide_objects;
	std::shared_ptr<udsdx::SceneObject> m_main_hero;
	std::shared_ptr<udsdx::Scene> m_targetScene;
	Vector3 m_cur_target_pos = {};
	bool m_guide_active_flag = false;
	std::map<uint32_t, std::shared_ptr<udsdx::SceneObject>> m_mapHarvest;
};

