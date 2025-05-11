#include "pch.h"
#include "s2c_DummyPacketHandler.h"
#include "CreateBuffer4Dummy.h"
#include "enum_generated.h"
#include "struct_generated.h"
#include "protocol_generated.h"
#include "ServerSession.h"
#include "Queueabler.h"
#include  "Navigator.h"

Vector3 O_VEC3(const Nagox::Struct::Vec3* const v) {
	return Vector3{ v->x(),v->y(),v->z() };
}

Nagox::Struct::Vec3 F_VEC3(const Vector3& v) {
	return  Nagox::Struct::Vec3{ v.x,v.y,v.z };
}

constexpr const Vector3 dir[4]
{
	Vector3(-1,0,0),
	Vector3(1,0,0),
	Vector3(0,0,-1),
	Vector3(0,0,1),
};

flatbuffers::FlatBufferBuilder* const CreateBuilder()noexcept {
	thread_local flatbuffers::FlatBufferBuilder buillder{ 256 };
	return &buillder;	
}

const bool Handle_s2c_LOGIN(const NagiocpX::S_ptr<NagiocpX::PacketSession>& pSession_, const Nagox::Protocol::s2c_LOGIN& pkt_)
{
	const auto session = NagiocpX::StaticCast<ServerSession>(pSession_);

	static std::mt19937_64  rng{ std::random_device{}() };
	static std::uniform_int_distribution<int> dist1024(0, 1023);

	// 함수 내에서 사용
	const float pos_x = dist1024(rng) - 512.f;  // 0 ~ 1023 균일 분포
	const float pos_z = dist1024(rng) - 512.f;
	Vector3 pos{ pos_x,0,pos_z };
	NAVIGATION->GetNavMesh(NUM_0)->GetNaviCell(pos);
	//const auto vv = Vector3{ -30.0f, 0.0f, -30.0f } + dir[NagiocpX::my_rand() % 4] * 10.f;
	//const auto vv = Vector3{ -30.0f, 0.0f, -30.0f } + dir[NagiocpX::my_rand() % 4] * 10.f;
	//const auto v = Nagox::Struct::Vec3{ 75.0f, 0.0f, 25.0f };
	pSession_ << Create_c2s_ENTER(F_VEC3(pos),Nagox::Enum::PLAYER_TYPE_WARRIOR);
	session->m_id = pkt_.obj_id();
	session->pos = pos;

	session->StartMovePacket();

	
	return true;
}

const bool Handle_s2c_PING_PONG(const NagiocpX::S_ptr<NagiocpX::PacketSession>& pSession_, const Nagox::Protocol::s2c_PING_PONG& pkt_)
{
	return true;
}

const bool Handle_s2c_APPEAR_OBJECT(const NagiocpX::S_ptr<NagiocpX::PacketSession>& pSession_, const Nagox::Protocol::s2c_APPEAR_OBJECT& pkt_)
{
	if (pkt_.group_type() != Nagox::Enum::GROUP_TYPE_HARVEST)return true;
	const auto session = static_cast<ServerSession*>(pSession_.get());
	session->UpdateHarvest(pkt_.obj_id(), O_VEC3(pkt_.appear_pos()), HARVEST_STATE::AVAILABLE == static_cast<HARVEST_STATE>(pkt_.obj_type_info()));
	return true;
}

const bool Handle_s2c_REMOVE_OBJECT(const NagiocpX::S_ptr<NagiocpX::PacketSession>& pSession_, const Nagox::Protocol::s2c_REMOVE_OBJECT& pkt_)
{
	const auto session = NagiocpX::StaticCast<ServerSession>(pSession_);
	//session->m_accDelayMs = 0;
	//session->m_moveCount = 1;
	return true;
}

const bool Handle_s2c_MOVE(const NagiocpX::S_ptr<NagiocpX::PacketSession>& pSession_, const Nagox::Protocol::s2c_MOVE& pkt_)
{
	
	const auto session = NagiocpX::StaticCast<ServerSession>(pSession_);
	//auto& ts = session->m_timeStamp;
	if (session->m_id == pkt_.obj_id())
	{
		session->UpdateTimeStamp(pkt_.time_stamp());
	}

	//std::cout << pkt_.obj_id() << std::endl;
	//if (::GetTickCount64() >= ts + 500)
	//{
	//	const auto val = NagiocpX::my_rand() % 4;
	//	//const Vector3 next_pos = session->pos + dir[val] * .5f;
	//	const Vector3 next_pos = session->pos;
	//	const auto v = Nagox::Struct::Vec3{ next_pos.x,next_pos.y,next_pos.z };
	//	//std::cout << "MOVE" << std::endl;
	//	//session->pos = next_pos;
	//	//const auto v = Nagox::Struct::Vec3{ pkt_.pos()->x(),pkt_.pos()->y() + 0.2f,pkt_.pos()->z() };
	//	pSession_ << Create_c2s_MOVE(v, *pkt_.vel(), *pkt_.accel(), pkt_.body_angle(), pkt_.time_stamp());
	//	ts = ::GetTickCount64();
	//}
	//else
	//{
	//	//std::cout << "!!" << std::endl;
	//}
	return true;
}

const bool Handle_s2c_MONSTER_ATTACK(const NagiocpX::S_ptr<NagiocpX::PacketSession>& pSession_, const Nagox::Protocol::s2c_MONSTER_ATTACK& pkt_)
{
	return true;
}

const bool Handle_s2c_NOTIFY_HIT_DMG(const NagiocpX::S_ptr<NagiocpX::PacketSession>& pSession_, const Nagox::Protocol::s2c_NOTIFY_HIT_DMG& pkt_)
{
	return true;
}

const bool Handle_s2c_MONSTER_AGGRO_START(const NagiocpX::S_ptr<NagiocpX::PacketSession>& pSession_, const Nagox::Protocol::s2c_MONSTER_AGGRO_START& pkt_)
{
	return true;
}

const bool Handle_s2c_MONSTER_AGGRO_END(const NagiocpX::S_ptr<NagiocpX::PacketSession>& pSession_, const Nagox::Protocol::s2c_MONSTER_AGGRO_END& pkt_)
{
	return true;
}

const bool Handle_s2c_PLAYER_ATTACK(const NagiocpX::S_ptr<NagiocpX::PacketSession>& pSession_, const Nagox::Protocol::s2c_PLAYER_ATTACK& pkt_)
{
	return true;
}

const bool Handle_s2c_PLAYER_DEATH(const NagiocpX::S_ptr<NagiocpX::PacketSession>& pSession_, const Nagox::Protocol::s2c_PLAYER_DEATH& pkt_)
{
	return true;
}

const bool Handle_s2c_REQUEST_QUEST(const NagiocpX::S_ptr<NagiocpX::PacketSession>& pSession_, const Nagox::Protocol::s2c_REQUEST_QUEST& pkt_)
{
	return true;
}

const bool Handle_s2c_CLEAR_QUEST(const NagiocpX::S_ptr<NagiocpX::PacketSession>& pSession_, const Nagox::Protocol::s2c_CLEAR_QUEST& pkt_)
{
	return true;
}

const bool Handle_s2c_FIRE_PROJ(const NagiocpX::S_ptr<NagiocpX::PacketSession>& pSession_, const Nagox::Protocol::s2c_FIRE_PROJ& pkt_)
{
	return true;
}

const bool Handle_s2c_ACQUIRE_ITEM(const NagiocpX::S_ptr<NagiocpX::PacketSession>& pSession_, const Nagox::Protocol::s2c_ACQUIRE_ITEM& pkt_)
{
	return true;
}

const bool Handle_s2c_USE_QUICK_SLOT_ITEM(const NagiocpX::S_ptr<NagiocpX::PacketSession>& pSession_, const Nagox::Protocol::s2c_USE_QUICK_SLOT_ITEM& pkt_)
{
	return true;
}

const bool Handle_s2c_CRAFT_ITEM(const NagiocpX::S_ptr<NagiocpX::PacketSession>& pSession_, const Nagox::Protocol::s2c_CRAFT_ITEM& pkt_)
{
	return true;
}

const bool Handle_s2c_REGISTER_PARTY_QUEST(const NagiocpX::S_ptr<NagiocpX::PacketSession>& pSession_, const Nagox::Protocol::s2c_REGISTER_PARTY_QUEST& pkt_)
{
	return true;
}

const bool Handle_s2c_ACQUIRE_PARTY_LIST(const NagiocpX::S_ptr<NagiocpX::PacketSession>& pSession_, const Nagox::Protocol::s2c_ACQUIRE_PARTY_LIST& pkt_)
{
	return true;
}

const bool Handle_s2c_INVITE_PARTY_QUEST(const NagiocpX::S_ptr<NagiocpX::PacketSession>& pSession_, const Nagox::Protocol::s2c_INVITE_PARTY_QUEST& pkt_)
{
	return true;
}

const bool Handle_s2c_INVITE_PARTY_RESULT(const NagiocpX::S_ptr<NagiocpX::PacketSession>& pSession_, const Nagox::Protocol::s2c_INVITE_PARTY_RESULT& pkt_)
{
	return true;
}

const bool Handle_s2c_PARTY_JOIN_REQUEST(const NagiocpX::S_ptr<NagiocpX::PacketSession>& pSession_, const Nagox::Protocol::s2c_PARTY_JOIN_REQUEST& pkt_)
{
	return true;
}

const bool Handle_s2c_PARTY_JOIN_REQUEST_RESULT(const NagiocpX::S_ptr<NagiocpX::PacketSession>& pSession_, const Nagox::Protocol::s2c_PARTY_JOIN_REQUEST_RESULT& pkt_)
{
	return true;
}

const bool Handle_s2c_PARTY_JOIN_NEW_PLAYER(const NagiocpX::S_ptr<NagiocpX::PacketSession>& pSession_, const Nagox::Protocol::s2c_PARTY_JOIN_NEW_PLAYER& pkt_)
{
	return true;
}

const bool Handle_s2c_PARTY_OUT(const NagiocpX::S_ptr<NagiocpX::PacketSession>& pSession_, const Nagox::Protocol::s2c_PARTY_OUT& pkt_)
{
	return true;
}

const bool Handle_s2c_PARTY_QUEST_CLEAR(const NagiocpX::S_ptr<NagiocpX::PacketSession>& pSession_, const Nagox::Protocol::s2c_PARTY_QUEST_CLEAR& pkt_)
{
	return true;
}

const bool Handle_s2c_PARTY_MEMBERS_INFORMATION(const NagiocpX::S_ptr<NagiocpX::PacketSession>& pSession_, const Nagox::Protocol::s2c_PARTY_MEMBERS_INFORMATION& pkt_)
{
	return true;
}

const bool Handle_s2c_CHANGE_HARVEST_STATE(const NagiocpX::S_ptr<NagiocpX::PacketSession>& pSession_, const Nagox::Protocol::s2c_CHANGE_HARVEST_STATE& pkt_)
{

	const auto session = static_cast<ServerSession*>(pSession_.get());
	session->UpdateHarvest(pkt_.harvest_id(), pkt_.is_active());
	return true;
}
