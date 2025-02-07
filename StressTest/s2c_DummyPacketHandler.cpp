#include "pch.h"
#include "s2c_DummyPacketHandler.h"
#include "CreateBuffer4Dummy.h"
#include "enum_generated.h"
#include "struct_generated.h"
#include "protocol_generated.h"
#include "ServerSession.h"
#include "Queueabler.h"

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

const bool Handle_s2c_LOGIN(const ServerCore::S_ptr<ServerCore::PacketSession>& pSession_, const Nagox::Protocol::s2c_LOGIN& pkt_)
{
	const auto session = ServerCore::StaticCast<ServerSession>(pSession_);
	const auto vv = Vector3{ -30.0f, 0.0f, -30.0f } + dir[ServerCore::my_rand() % 4] * 10.f;
	//const auto v = Nagox::Struct::Vec3{ 75.0f, 0.0f, 25.0f };
	pSession_ << Create_c2s_ENTER(F_VEC3(vv));
	session->m_id = pkt_.obj_id();
	session->pos = vv;

	session->StartMovePacket();

	
	return true;
}

const bool Handle_s2c_PING_PONG(const ServerCore::S_ptr<ServerCore::PacketSession>& pSession_, const Nagox::Protocol::s2c_PING_PONG& pkt_)
{
	return true;
}

const bool Handle_s2c_APPEAR_OBJECT(const ServerCore::S_ptr<ServerCore::PacketSession>& pSession_, const Nagox::Protocol::s2c_APPEAR_OBJECT& pkt_)
{
	return true;
}

const bool Handle_s2c_REMOVE_OBJECT(const ServerCore::S_ptr<ServerCore::PacketSession>& pSession_, const Nagox::Protocol::s2c_REMOVE_OBJECT& pkt_)
{
	const auto session = ServerCore::StaticCast<ServerSession>(pSession_);
	//session->m_accDelayMs = 0;
	//session->m_moveCount = 1;
	return true;
}

const bool Handle_s2c_MOVE(const ServerCore::S_ptr<ServerCore::PacketSession>& pSession_, const Nagox::Protocol::s2c_MOVE& pkt_)
{
	
	const auto session = ServerCore::StaticCast<ServerSession>(pSession_);
	//auto& ts = session->m_timeStamp;
	if (session->m_id == pkt_.obj_id())
	{
		session->UpdateTimeStamp(pkt_.time_stamp());
	}

	//std::cout << pkt_.obj_id() << std::endl;
	//if (::GetTickCount64() >= ts + 500)
	//{
	//	const auto val = ServerCore::my_rand() % 4;
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

const bool Handle_s2c_MONSTER_ATTACK(const ServerCore::S_ptr<ServerCore::PacketSession>& pSession_, const Nagox::Protocol::s2c_MONSTER_ATTACK& pkt_)
{
	return true;
}

const bool Handle_s2c_MONSTER_AGGRO_START(const ServerCore::S_ptr<ServerCore::PacketSession>& pSession_, const Nagox::Protocol::s2c_MONSTER_AGGRO_START& pkt_)
{
	return true;
}

const bool Handle_s2c_MONSTER_AGGRO_END(const ServerCore::S_ptr<ServerCore::PacketSession>& pSession_, const Nagox::Protocol::s2c_MONSTER_AGGRO_END& pkt_)
{
	return true;
}

const bool Handle_s2c_PLAYER_ATTACK(const ServerCore::S_ptr<ServerCore::PacketSession>& pSession_, const Nagox::Protocol::s2c_PLAYER_ATTACK& pkt_)
{
	return true;
}

const bool Handle_s2c_PLAYER_DEATH(const ServerCore::S_ptr<ServerCore::PacketSession>& pSession_, const Nagox::Protocol::s2c_PLAYER_DEATH& pkt_)
{
	return true;
}

const bool Handle_s2c_REQUEST_QUEST(const ServerCore::S_ptr<ServerCore::PacketSession>& pSession_, const Nagox::Protocol::s2c_REQUEST_QUEST& pkt_)
{
	return true;
}

const bool Handle_s2c_CLEAR_QUEST(const ServerCore::S_ptr<ServerCore::PacketSession>& pSession_, const Nagox::Protocol::s2c_CLEAR_QUEST& pkt_)
{
	return true;
}

const bool Handle_s2c_FIRE_PROJ(const ServerCore::S_ptr<ServerCore::PacketSession>& pSession_, const Nagox::Protocol::s2c_FIRE_PROJ& pkt_)
{
	return true;
}
