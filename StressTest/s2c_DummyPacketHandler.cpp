#include "pch.h"
#include "s2c_DummyPacketHandler.h"
#include "CreateBuffer4Dummy.h"
#include "enum_generated.h"
#include "struct_generated.h"
#include "protocol_generated.h"
#include "ServerSession.h"

static inline Vector3 O_VEC3(const Nagox::Struct::Vec3* const v) {
	return Vector3{ v->x(),v->y(),v->z() };
}

static inline Nagox::Struct::Vec3 F_VEC3(const Vector3& v) {
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
	const auto vv = Vector3{ 75.0f, 0.0f, 25.0f } + dir[ServerCore::my_rand() % 4] * 10.f;
	//const auto v = Nagox::Struct::Vec3{ 75.0f, 0.0f, 25.0f };
	pSession_ << Create_c2s_ENTER(F_VEC3(vv));
	
	session->pos = vv;
	return true;
}

const bool Handle_s2c_APPEAR_OBJECT(const ServerCore::S_ptr<ServerCore::PacketSession>& pSession_, const Nagox::Protocol::s2c_APPEAR_OBJECT& pkt_)
{
	return true;
}

const bool Handle_s2c_REMOVE_OBJECT(const ServerCore::S_ptr<ServerCore::PacketSession>& pSession_, const Nagox::Protocol::s2c_REMOVE_OBJECT& pkt_)
{
	return true;
}

const bool Handle_s2c_MOVE(const ServerCore::S_ptr<ServerCore::PacketSession>& pSession_, const Nagox::Protocol::s2c_MOVE& pkt_)
{
	
	const auto session = ServerCore::StaticCast<ServerSession>(pSession_);
	auto& ts = session->m_timeStamp;
	if (session->m_id == pkt_.obj_id())
		std::cout << "!!" << std::endl;

	//std::cout << pkt_.obj_id() << std::endl;
	if (::GetTickCount64() >= ts + 500)
	{
		const auto val = ServerCore::my_rand() % 4;
		const Vector3 next_pos = session->pos + dir[val] * .5f;
		const auto v = Nagox::Struct::Vec3{ next_pos.x,next_pos.y,next_pos.z };
		//std::cout << "MOVE" << std::endl;
		session->pos = next_pos;
		//const auto v = Nagox::Struct::Vec3{ pkt_.pos()->x(),pkt_.pos()->y() + 0.2f,pkt_.pos()->z() };
		pSession_ << Create_c2s_MOVE(v, *pkt_.vel(), *pkt_.accel(), pkt_.body_angle(), pkt_.time_stamp());
		ts = ::GetTickCount64();
	}
	else
	{
		//std::cout << "!!" << std::endl;
	}
	return true;
}