#pragma once
#include "PacketSession.h"
#include "QuestRoom.h"

class PartyQuestSystem;

class ClientSession
	:public NagiocpX::PacketSession
{
public:
	ClientSession()noexcept;
	~ClientSession();
public:
	virtual void OnConnected() override;
	virtual void OnSend(c_int32 len)noexcept override {}
	virtual void OnDisconnected(const NagiocpX::Cluster* const curCluster_)noexcept override;
public:
	std::shared_ptr<PartyQuestSystem> CreatePartySystem();
	const auto GetCurPartySystem()const noexcept { return m_cur_my_party_system.load(); }
	std::shared_ptr<PartyQuestSystem> QueryPartyLeader()const noexcept;

	S_ptr<ContentsEntity> GetCurPartySystemLeader()const noexcept;
	PARTY_ACCEPT_RESULT AcceptNewPlayer(ContentsEntity* const other);
	PARTY_ACCEPT_RESULT AcceptNewPlayer(const S_ptr<PacketSession>& session);
	std::atomic<std::shared_ptr<PartyQuestSystem>>m_cur_my_party_system{ nullptr };
public:
	//template<typename T = ClientSession>
	//NagiocpX::S_ptr<T> SharedFromThis()const noexcept { return NagiocpX::S_ptr<T>{this}; }
private:

};

