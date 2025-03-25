#pragma once
#include "pch.h"

// TODO: 자동화
struct EntityBuilderBase
{
	Nagox::Enum::GROUP_TYPE group_type;
	uint8_t obj_type;
	uint32_t obj_id;


public:
	static inline std::shared_ptr<udsdx::SceneObject> CreateObject(EntityBuilderBase* builder)noexcept {
		const auto create_func = g_createObjectFunc[builder->group_type][builder->obj_type];
		if (!create_func)
		{
			//TODO:: 적당한 예외처리 nullptr 뱉는게 의미가 없고 그냥 터뜨리고 다시 빌드하는게 맞을 것 같음
		}
		return create_func(builder);
	}


#pragma region CREATE_FUNC_LIST
private:
#pragma region PLAEYER
	static std::shared_ptr<udsdx::SceneObject> Create_Warrior(EntityBuilderBase* builder);
	// 앞으로 법사, 궁수를 여기에추가
#pragma endregion


#pragma region MONSTER
	// 몬스터를 여기에 추가
	static std::shared_ptr<udsdx::SceneObject> Create_Monster(EntityBuilderBase* builder);
#pragma endregion

#pragma region NPC
	static std::shared_ptr<udsdx::SceneObject> Create_NPC(EntityBuilderBase* builder);
	
#pragma endregion

#pragma region DROP_ITEM
	// 드랍아이템을 여기에 추가
	static std::shared_ptr<udsdx::SceneObject> Create_DropItem(EntityBuilderBase* builder);
#pragma endregion

#pragma endregion


private:
	static const bool InitTable()noexcept
	{
		// 테이블 초기화 함수
#pragma region INIT_FUNC_LIST

		// 실수방지를 위해 한번 참조로 받고 시작하였음
#pragma region PLAEYER
		auto& player_func_table = g_createObjectFunc[Nagox::Enum::GROUP_TYPE::GROUP_TYPE_PLAYER];

		player_func_table[PLAYER_TYPE_INFO::WARRIOR] = Create_Warrior;

#pragma endregion

#pragma region MONSTER
		auto& monster_func_table = g_createObjectFunc[Nagox::Enum::GROUP_TYPE::GROUP_TYPE_MONSTER];

		monster_func_table[MONSTER_TYPE_INFO::FOX] = Create_Monster;


#pragma endregion

#pragma region NPC
		auto& npc_func_table = g_createObjectFunc[Nagox::Enum::GROUP_TYPE::GROUP_TYPE_NPC];

		npc_func_table[0] = Create_NPC;
#pragma endregion

#pragma region DROP_ITEM
		auto& drop_item_func_table = g_createObjectFunc[Nagox::Enum::GROUP_TYPE::GROUP_TYPE_DROP_ITEM];

		drop_item_func_table[0] = Create_DropItem;
#pragma endregion

#pragma endregion

		return true;
	}
private:

	using CreateObjectFunc = std::shared_ptr<udsdx::SceneObject>(*)(EntityBuilderBase*);

	static inline std::unordered_map<Nagox::Enum::GROUP_TYPE, std::unordered_map<uint8_t, CreateObjectFunc>> g_createObjectFunc;


	static inline const bool g_initFlag = InitTable();
};

struct DefaultEntityBuilder
	:public EntityBuilderBase
{
	std::string obj_name;
	Vector3 obj_pos;
};

struct MissileBuilder
	:public EntityBuilderBase
{
	Vector3 dir;
	// 예시 코드
};


