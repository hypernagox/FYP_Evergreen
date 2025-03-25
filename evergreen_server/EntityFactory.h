#pragma once
#include "pch.h"

namespace NagiocpX
{
	struct EntityBuilder
	{
		uint32_t id;
		Nagox::Enum::GROUP_TYPE group_type;
		uint8_t obj_type;
		float x, y, z;
	};

	class EntityFactory
	{
	public:

		static S_ptr<ContentsEntity> CreateMonster(const EntityBuilder& b)noexcept;

		static S_ptr<ContentsEntity> CreateNPC(const EntityBuilder& b)noexcept;

		static S_ptr<ContentsEntity> CreateRangeMonster(const EntityBuilder& b)noexcept;

		static S_ptr<ContentsEntity> CreateDropItem(const EntityBuilder& b)noexcept;
	};
}

using NagiocpX::EntityBuilder;
using NagiocpX::EntityFactory;