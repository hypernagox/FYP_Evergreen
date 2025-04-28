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
	bool CreatePartySystem();
	const auto GetCurPartySystem()const noexcept { return m_cur_my_party_system.load(); }
	bool HasParty()const noexcept { return GetCurPartySystem(); }
	ClientSession* GetCurPartySystemLeader()const noexcept;
		
	PARTY_ACCEPT_RESULT AcceptNewPlayer(ClientSession* const other);
		
	
	PARTY_ACCEPT_RESULT AcceptNewPlayer(const S_ptr<PacketSession>& session) {
		return AcceptNewPlayer(static_cast<ClientSession*>(session.get()));
	}
	bool IsPartyLeader()const noexcept { return m_cur_my_party_system.load() == m_party_quest_system; }
public:
	NagoxAtomic::Atomic<PartyQuestSystem*> m_cur_my_party_system{ nullptr };
	PartyQuestSystem* m_party_quest_system = nullptr;
public:
	//template<typename T = ClientSession>
	//NagiocpX::S_ptr<T> SharedFromThis()const noexcept { return NagiocpX::S_ptr<T>{this}; }
private:

};

