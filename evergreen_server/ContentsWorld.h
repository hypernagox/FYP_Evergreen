#pragma once
#include "World.h"

class ContentsWorld
	:public ServerCore::World
{
public:
	static constexpr inline const uint8_t NUM_OF_GROUPS = static_cast<uint8_t>(Nagox::Enum::GROUP_TYPE_MAX + 1);
public:
	virtual void InitWorld()noexcept;
	virtual void Dispatch(ServerCore::IocpEvent* const iocpEvent_, const int32_t numOfBytes)noexcept override {}
	virtual void MigrationWolrdAfterBehavior(const ServerCore::S_ptr<ServerCore::World> prevWorld, ServerCore::ContentsEntity* const pEntity_)noexcept override {}
private:

};

