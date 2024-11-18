#pragma once
#include "pch.h"

class ServerCore::SendBuffer;
class ServerCore::ContentsEntity;

class ClusterPredicate
	:public ServerCore::Singleton<ClusterPredicate>
{
	friend class Singleton;
	ClusterPredicate();
	~ClusterPredicate();
public:
	static bool ClusterHuristicFunc2Session(const ServerCore::ContentsEntity* const a, const ServerCore::ContentsEntity* const b)noexcept;
	static bool ClusterHuristicFunc2NPC(const ServerCore::ContentsEntity* const a, const ServerCore::ContentsEntity* const b)noexcept;

	static ServerCore::S_ptr<ServerCore::SendBuffer> ClusterAddPacketFunc(const ServerCore::ContentsEntity* const)noexcept;
	static ServerCore::S_ptr<ServerCore::SendBuffer> ClusterRemovePacketFunc(const ServerCore::ContentsEntity* const)noexcept;
	static ServerCore::S_ptr<ServerCore::SendBuffer> ClusterMovePacketFunc(const ServerCore::ContentsEntity* const)noexcept;
public:
	static void TryNotifyNPC(const ServerCore::ContentsEntity* const a, const ServerCore::ContentsEntity* const b)noexcept;
private:

};

