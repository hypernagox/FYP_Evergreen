#pragma once
#include "PacketSession.h"
#include "PartyQuestSystem.h"

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
	bool CreatePartySystem() {
		if (m_cur_my_party_system.load())return false;
		m_party_quest_system.m_member[0] = SharedFromThis<ClientSession>();
		m_cur_my_party_system.store(&m_party_quest_system);
		return true;
	}
	const auto GetCurPartySystem()const noexcept { return m_cur_my_party_system.load(); }
	bool HasParty()const noexcept { return GetCurPartySystem(); }
	ClientSession* GetCurPartySystemLeader()const noexcept {
		const auto party = m_cur_my_party_system.load();
		if (!party)return nullptr;
		return party->GetPartyLeader();
	}
	bool AcceptNewPlayer(ClientSession* const other) {
		if (!IsPartyLeader())return false;
		if (other->HasParty())return false;
		other->m_cur_my_party_system.store(&m_party_quest_system);
		return m_party_quest_system.AcceptNewMember(other->SharedFromThis<ClientSession>());
	}
	bool AcceptNewPlayer(const S_ptr<PacketSession>& session) {
		return AcceptNewPlayer(static_cast<ClientSession*>(session.get()));
	}
	bool IsPartyLeader()const noexcept { return m_cur_my_party_system.load() == &m_party_quest_system; }
public:
	NagoxAtomic::Atomic<PartyQuestSystem*> m_cur_my_party_system{ nullptr };
	PartyQuestSystem m_party_quest_system;
public:
	//template<typename T = ClientSession>
	//NagiocpX::S_ptr<T> SharedFromThis()const noexcept { return NagiocpX::S_ptr<T>{this}; }
private:

};

