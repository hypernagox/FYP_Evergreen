#pragma once
#include "pch.h"
#include "BroadcastHelper.h"

class ServerCore::SendBuffer;
class ServerCore::ContentsEntity;

class ClusterPredicate
	:public ServerCore::BroadcastHelper
{
public:
	ClusterPredicate();
	~ClusterPredicate();
public:
	virtual bool Filter4Session(const ContentsEntity* const a, const ContentsEntity* const b)const noexcept;
	virtual bool Filter4NPC(const ContentsEntity* const a, const ContentsEntity* const b)const noexcept;
	virtual S_ptr<SendBuffer> CreateAddPacket(const ContentsEntity* const entity_ptr)const noexcept;
	virtual S_ptr<SendBuffer> CreateRemovePacket(const uint32_t obj_id)const noexcept;
	virtual S_ptr<SendBuffer> CreateMovePacket(const ContentsEntity* const entity_ptr)const noexcept;

private:

};

