#pragma once
#include "ServerCorePch.h"

namespace ServerCore
{
	class MoveBroadcaster;

	class BroadcastHelper
	{
	public:
		virtual bool Filter4Session(const ContentsEntity* const a, const ContentsEntity* const b)const noexcept = 0;
		virtual bool Filter4NPC(const ContentsEntity* const a, const ContentsEntity* const b)const noexcept = 0;
		virtual S_ptr<SendBuffer> CreateAddPacket(const ContentsEntity* const entity_ptr)const noexcept = 0;
		virtual S_ptr<SendBuffer> CreateRemovePacket(const uint32_t obj_id)const noexcept = 0;
		virtual S_ptr<SendBuffer> CreateMovePacket(const ContentsEntity* const entity_ptr)const noexcept = 0;
	};

	class BroadcastHandler
	{
	public:
		virtual void HandleBroadcast(MoveBroadcaster* const broad_caster)const noexcept = 0;
	};

}