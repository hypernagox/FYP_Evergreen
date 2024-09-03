#pragma once
#include "ServerCorePch.h"
#include "MoveBroadcaster.h"
#include "ID_Ptr.hpp"

class ServerCore::Sector;
class ServerCore::ContentsEntity;
class ServerCore::PacketSession;

enum SECTOR_BEHAVIOR : uint8_t 
{
	NOTHING = 0,
	INIT_ENTER = 1 << 0,
	MIGRATION = 1 << 1,
	LEAVE = 1 << 2,

	ALL = INIT_ENTER | MIGRATION | LEAVE,

	END,
};

namespace ServerCore
{
	class SectorBehavior
	{
	public:
		SectorBehavior(const SECTOR_BEHAVIOR eSectorBehaviorBit)noexcept :m_bitFlag{ eSectorBehaviorBit } {}
		virtual ~SectorBehavior()noexcept = default;
	public:
		inline void InitEnterBehavior(const ServerCore::ContentsEntity* const pOwner_, ServerCore::Sector* const sector)noexcept {
			if (INIT_ENTER & m_bitFlag)InitEnterBehaviorInternal(pOwner_, sector);
		}
		inline void MigrationAfterBehavior(const ServerCore::ContentsEntity* const pOwner_, ServerCore::Sector* const before_sector, ServerCore::Sector* const cur_sector)noexcept {
			if (MIGRATION & m_bitFlag)MigrationAfterBehaviorInternal(pOwner_, before_sector, cur_sector);
		}
		inline void LeaveAfterBehavior(const ServerCore::ContentsEntity* const pOwner_, ServerCore::Sector* const sector)noexcept {
			if (LEAVE & m_bitFlag)LeaveAfterBehaviorInternal(pOwner_, sector);
		}
	private:
		virtual void InitEnterBehaviorInternal(const ServerCore::ContentsEntity* const pOwner_, ServerCore::Sector* const sector)noexcept {}
		virtual void MigrationAfterBehaviorInternal(const ServerCore::ContentsEntity* const pOwner_, ServerCore::Sector* const before_sector, ServerCore::Sector* const cur_sector)noexcept {}
		virtual void LeaveAfterBehaviorInternal(const ServerCore::ContentsEntity* const pOwner_, ServerCore::Sector* const sector)noexcept {}
	private:
		const uint8_t m_bitFlag;
	};
}