#include "flatbuffers/pch/pch.h"
#include "flatbuffers/pch/flatc_pch.h"
#include "flatbuffers/flatbuffers.h"
#include "s2c_PacketHandler.h"
#include "../evergreen_client/ServerObjectMgr.h"
#include "../evergreen_client/ServerObject.h"
#include "../evergreen_client/MoveInterpolator.h"
#include "func.h"
#include "../evergreen_client/EntityBuilder.h"

flatbuffers::FlatBufferBuilder* const CreateBuilder()noexcept {
	thread_local flatbuffers::FlatBufferBuilder buillder{ 256 };
	return &buillder;
}

extern std::shared_ptr<Scene> scene;

#define Mgr(type)	(type::GetInst())

const bool Handle_s2c_LOGIN(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_LOGIN& pkt_)
{
	NetMgr(NetworkMgr)->SetSessionID(pkt_.obj_id());
	return true;
}

const bool Handle_s2c_APPEAR_OBJECT(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_APPEAR_OBJECT& pkt_)
{
	// TODO: 빌더패턴 / 팩토리 패턴 처럼
	// 적당히 enum값이 오면 알아서 만들어서 씬 오브젝트 뱉는 구조가 필요하다.
	// 앞으로도 이렇게 하드코딩해서 객체를 만들 순 없다.

	const auto obj_id = pkt_.obj_id();
	if (Mgr(ServerObjectMgr)->GetServerObj(obj_id))
		return true;

	DefaultEntityBuilder b;
	b.obj_id = pkt_.obj_id();
	b.obj_type = pkt_.obj_type_info();
	b.group_type = pkt_.group_type();
	b.obj_pos = ::ToOriginVec3(pkt_.appear_pos());
	
	Mgr(ServerObjectMgr)->AddObject(&b);
	return true;
}

const bool Handle_s2c_REMOVE_OBJECT(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_REMOVE_OBJECT& pkt_)
{
	Mgr(ServerObjectMgr)->RemoveObject(pkt_.obj_id());
	return true;
}

const bool Handle_s2c_MOVE(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_MOVE& pkt_)
{
	if (const auto obj = ServerObjectMgr::GetInst()->GetServerObj(pkt_.obj_id()))
	{
		if (const auto comp = obj->GetComp<MoveInterpolator>())
		{
			comp->UpdateNewMoveData(pkt_);
		}
	}

    return true;
}

const bool Handle_s2c_MONSTER_ATTACK(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_MONSTER_ATTACK& pkt_)
{
	std::cout << "여우가 당신에게 " << pkt_.dmg() << "데미지를 주었다 !" << std::endl;
	return true;
}

const bool Handle_s2c_MONSTER_AGGRO_START(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_MONSTER_AGGRO_START& pkt_)
{
	std::cout << "여우가 당신을 주시하고있다 ... " << std::endl;
	return true;
}

const bool Handle_s2c_MONSTER_AGGRO_END(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_MONSTER_AGGRO_END& pkt_)
{
	std::cout << "아무래도 여우는 당신에게 흥미가 없어진 것 같다 ..." << std::endl;
	return true;
}

