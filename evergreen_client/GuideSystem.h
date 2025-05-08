#pragma once


// �ڿ�, �� , NPC �� Ŭ�����忡���� ���̵� �ý���(�� �ȳ�)
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

	bool AddHarvest(uint32_t id, std::shared_ptr<udsdx::SceneObject> obj, bool is_active);

	void RemoveHarvest(uint32_t id) {
		// ä������ �� �þ߿��� ����� �� �׳� �������� �� ������ �ٽ� ����
		m_mapHarvest.erase(id);
		m_active_list.erase(id);
		m_in_active_list.erase(id);
	}

	udsdx::SceneObject*  GetHarvest(const uint32_t id)const noexcept {
		const auto iter = m_mapHarvest.find(id);
		return m_mapHarvest.end() != iter ? iter->second.get() : nullptr;
	}

	udsdx::SceneObject* GetHarvest(const uint32_t id)noexcept {
		const auto iter = m_mapHarvest.find(id);
		return m_mapHarvest.end() != iter ? iter->second.get() : nullptr;
	}

	const bool SetHarvestState(const uint32_t id, const bool is_active)noexcept;

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
	// ������ �ϴ� ����� ���� ���� (���׷��� �� �� �˶�)
	std::function<void(
		const std::vector<Vector3>&
	)> m_path_obj_maker;
	std::vector<std::shared_ptr<udsdx::SceneObject>> m_guide_objects;
	std::shared_ptr<udsdx::SceneObject> m_main_hero;
	std::shared_ptr<udsdx::Scene> m_targetScene;
	Vector3 m_cur_target_pos = {};
	bool m_guide_active_flag = false;
	std::map<uint32_t, std::shared_ptr<udsdx::SceneObject>> m_mapHarvest;
	std::unordered_set<uint32_t> m_in_active_list;
	std::unordered_set<uint32_t> m_active_list;
};

