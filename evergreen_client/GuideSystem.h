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
	// Ŭ���̾�Ʈ�� �������� ��, �޽� ������Ʈ�� �߰��ϴ� �ܰ�
	void AddHarvestMeshObject(std::shared_ptr<udsdx::SceneObject> obj);
	bool AddHarvest(uint32_t server_id, uint32_t harvest_id, bool is_active);
	void RemoveHarvest(uint32_t server_id);

	udsdx::SceneObject* GetHarvest(const uint32_t server_id)const noexcept {
		const auto iter = m_mapHarvestID.find(server_id);
		if (m_mapHarvestID.end() == iter)
			return nullptr;
		const auto harvest_id = iter->second;
		if (0 > harvest_id || m_mapHarvest.size() <= static_cast<size_t>(harvest_id))
			return nullptr;
		return m_mapHarvest[harvest_id].get();
	}

	udsdx::SceneObject* GetHarvest(const uint32_t server_id)noexcept {
		const auto iter = m_mapHarvestID.find(server_id);
		if (m_mapHarvestID.end() == iter)
			return nullptr;
		const auto harvest_id = iter->second;
		if (0 > harvest_id || m_mapHarvest.size() <= static_cast<size_t>(harvest_id))
			return nullptr;
		return m_mapHarvest[harvest_id].get();
	}

	const bool SetHarvestState(const uint32_t harvest_id, const bool is_active)noexcept;

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

	// Harvest ID�� �ʻ��� Harvest Object ID�� ���� (�ε����� ID)
	std::vector<std::shared_ptr<udsdx::SceneObject>> m_mapHarvest;
	// Mapping Server Object ID to Harvest ID
	std::map<uint32_t, uint32_t> m_mapHarvestID;

	std::unordered_set<uint32_t> m_in_active_list;
	std::unordered_set<uint32_t> m_active_list;
};

