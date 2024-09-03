#pragma once
#include "pch.h"

class ServerCore::SendBuffer;
class ServerCore::ContentsEntity;

class SectorPredicate
	:public ServerCore::Singleton<SectorPredicate>
{
	friend class Singleton;
	SectorPredicate();
	~SectorPredicate();
public:
	static bool SectorHuristicFunc2Session(const ServerCore::ContentsEntity* const a, const ServerCore::ContentsEntity* const b)noexcept;
	static bool SectorHuristicFunc2NPC(const ServerCore::ContentsEntity* const a, const ServerCore::ContentsEntity* const b)noexcept;

	static ServerCore::S_ptr<ServerCore::SendBuffer> SectorAddPacketFunc(const ServerCore::ContentsEntity* const)noexcept;
	static ServerCore::S_ptr<ServerCore::SendBuffer> SectorRemovePacketFunc(const ServerCore::ContentsEntity* const)noexcept;
	static ServerCore::S_ptr<ServerCore::SendBuffer> SectorMovePacketFunc(const ServerCore::ContentsEntity* const)noexcept;
public:
	static void TryNotifyNPC(const ServerCore::ContentsEntity* const a, const ServerCore::ContentsEntity* const b)noexcept;
private:

};

