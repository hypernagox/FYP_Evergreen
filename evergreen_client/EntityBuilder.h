#pragma once
#include "pch.h"

#define CHECK_NULL_CREATE_FUNC(type) \
    do { \
        if (g_createObjectFunc[type] != nullptr) { \
            printf("Error: Create function for group type %d is already set!\n", static_cast<int>(type)); \
            std::abort(); \
        } \
    } while (0)

// TODO: 자동화
struct EntityBuilderBase
{
	Nagox::Enum::GROUP_TYPE group_type;
	uint8_t obj_type;
	uint32_t obj_id;


public:
	static inline std::shared_ptr<udsdx::SceneObject> CreateObject(EntityBuilderBase* builder)noexcept {
		const auto idx = builder->group_type;
		if (idx > Nagox::Enum::GROUP_TYPE_MAX || idx < Nagox::Enum::GROUP_TYPE_MIN || !g_createObjectFunc[idx])
		{
			std::cout << "Invalid Bulider Enum\n";
			std::abort();
			return {};
		}
		return g_createObjectFunc[idx](builder);
	}


#pragma region CREATE_FUNC_LIST
private:
#pragma region PLAEYER
	static std::shared_ptr<udsdx::SceneObject> Create_Player(EntityBuilderBase* builder);
#pragma endregion

#pragma region MONSTER
	static std::shared_ptr<udsdx::SceneObject> Create_Monster(EntityBuilderBase* builder);
#pragma endregion

#pragma region NPC
	static std::shared_ptr<udsdx::SceneObject> Create_NPC(EntityBuilderBase* builder);
	
#pragma endregion

#pragma region DROP_ITEM
	static std::shared_ptr<udsdx::SceneObject> Create_DropItem(EntityBuilderBase* builder);
#pragma endregion

#pragma region HARVEST
	static std::shared_ptr<udsdx::SceneObject> Create_Harvest(EntityBuilderBase* builder);
#pragma endregion

#pragma endregion


private:
	static const bool InitTable()
	{
		// 테이블 초기화 함수
#pragma region INIT_FUNC_LIST
		std::ranges::fill(g_createObjectFunc, nullptr);
#pragma region PLAEYER
		{
			CHECK_NULL_CREATE_FUNC(Nagox::Enum::GROUP_TYPE_PLAYER);
			g_createObjectFunc[Nagox::Enum::GROUP_TYPE_PLAYER] = Create_Player;
		}
#pragma endregion

#pragma region MONSTER
		{
			CHECK_NULL_CREATE_FUNC(Nagox::Enum::GROUP_TYPE_MONSTER);
			g_createObjectFunc[Nagox::Enum::GROUP_TYPE_MONSTER] = Create_Monster;
		}
#pragma endregion

#pragma region NPC
		{
			CHECK_NULL_CREATE_FUNC(Nagox::Enum::GROUP_TYPE_NPC);
			g_createObjectFunc[Nagox::Enum::GROUP_TYPE_NPC] = Create_NPC;
		}
#pragma endregion

#pragma region DROP_ITEM
		{
			CHECK_NULL_CREATE_FUNC(Nagox::Enum::GROUP_TYPE_DROP_ITEM);
			g_createObjectFunc[Nagox::Enum::GROUP_TYPE_DROP_ITEM] = Create_DropItem;
		}
#pragma endregion

#pragma region HARVEST
		{
			CHECK_NULL_CREATE_FUNC(Nagox::Enum::GROUP_TYPE_HARVEST);
			g_createObjectFunc[Nagox::Enum::GROUP_TYPE_HARVEST] = Create_Harvest;
		}
#pragma endregion
		for (const auto fp : g_createObjectFunc)
		{
			if (!fp) {
				throw std::runtime_error{ "Create Table not init\n" };
			}
		}
#pragma endregion

		return true;
	}
private:

	using CreateObjectFunc = std::shared_ptr<udsdx::SceneObject>(*)(EntityBuilderBase*);

	static inline CreateObjectFunc g_createObjectFunc[Nagox::Enum::GROUP_TYPE::GROUP_TYPE_MAX + 1] = { nullptr };

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


