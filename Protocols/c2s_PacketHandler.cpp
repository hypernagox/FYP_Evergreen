#include "pch.h"
#include "c2s_PacketHandler.h"
#include "CreateBuffer4Server.h"
#include "../evergreen_server/ClientSession.h"
#include "../evergreen_server/PositionComponent.h"
#include "Queueabler.h"
#include "World.h"
#include "../evergreen_server/EntityFactory.h"

using namespace ServerCore;

flatbuffers::FlatBufferBuilder* const CreateBuilder()noexcept {
	//thread_local TCMallocAllocator tc_alloc;
	thread_local flatbuffers::FlatBufferBuilder buillder{ 256 };
	return &buillder;
}

static inline Vec3 ToOriginVec3(const Nagox::Struct::Vec3* const v)noexcept {
	return Vec3{ v->x(),v->y(),v->z() };
}

const bool Handle_c2s_LOGIN(const ServerCore::S_ptr<ServerCore::PacketSession>& pSession_, const Nagox::Protocol::c2s_LOGIN& pkt_)
{
	pSession_ << Create_s2c_LOGIN((uint32_t)pSession_->GetSessionID());
	return true;
}

const bool Handle_c2s_ENTER(const ServerCore::S_ptr<ServerCore::PacketSession>& pSession_, const Nagox::Protocol::c2s_ENTER& pkt_)
{
	auto entity = pSession_->GetOwnerEntity();

	//pSession_->SetEntity(entity);
	//entity->AddIocpComponent<Queueabler>();
	entity->AddComp<PositionComponent>()->pos = ToOriginVec3(pkt_.pos());

	//Mgr(WorldMgr)->GetWorld(0) ->GetStartSector()->BroadCastParallel(Create_s2c_APPEAR_OBJECT(pSession_->GetOwnerObjectID(), *pkt_.pos(), Nagox::Enum::OBJECT_TYPE_PLAYER));
	Mgr(WorldMgr)->GetWorld(0)->EnterWorld(entity);
	
	//g_sector->BroadCastParallel(Create_s2c_APPEAR_OBJECT(pSession_->GetOwnerObjectID(), *pkt_.pos(), Nagox::Enum::OBJECT_TYPE_PLAYER)
	//	, s
	//	, entity
	//);
	std::cout << "enter" << std::endl;
	//auto pbuff = Create_s2c_APPEAR_OBJECT(pSession_->GetOwnerObjectID(), *pkt_.pos(), Nagox::Enum::OBJECT_TYPE_PLAYER);
	//Mgr(WorldMgr)->GetWorld(0)->GetSector({ 0,0 })->BroadCastEnqueue(std::move(pbuff));
	//
	//auto pbuff2 = pbuff;
	EntityBuilder b;
	b.group_type = Nagox::Enum::GROUP_TYPE::GROUP_TYPE_MONSTER;
	b.obj_type = MONSTER_TYPE_INFO::FOX;
	
	b.x = 14.6667;
	b.y = 0.0833333;
	b.z = 14.6667;
	static bool b2 = true;
	if (b2)
	{
		const auto m = EntityFactory::CreateMonster(b);
		m->GetComp<PositionComponent>()->pos.x = 14.6667f;
		m->GetComp<PositionComponent>()->pos.y = 0.0833333f;
		m->GetComp<PositionComponent>()->pos.z = 14.6667f;
		
		Mgr(WorldMgr)->GetWorld(0)->EnterWorldNPC(m);
		b2 = false;
	}
	return true;
}

const bool Handle_c2s_MOVE(const ServerCore::S_ptr<ServerCore::PacketSession>& pSession_, const Nagox::Protocol::c2s_MOVE& pkt_)
{
	//if(pSession_->GetObjectID()!=1)
	//std::cout << "move" << std::endl;
	const auto& pEntity = pSession_->GetOwnerEntity()->GetComp<PositionComponent>();
	pEntity->pos = ToOriginVec3(pkt_.pos());
	pEntity->vel = ToOriginVec3(pkt_.vel());
	pEntity->accel = ToOriginVec3(pkt_.accel());
	pEntity->body_angle = pkt_.body_angle();
	pEntity->time_stamp = pkt_.time_stamp();
	Vector<Sector*> s{ pSession_->GetCurSector() };

	//g_sector->BroadCastEnqueue(Create_s2c_MOVE(GetBuilder(), pEntity->GetObjectID(),
	//	ToFlatVec3(pEntity->pos),
	//	ToFlatVec3(pEntity->vel),
	//	ToFlatVec3(pEntity->accel),
	//	pEntity->body_angle,
	//	pEntity->time_stamp));
	//g_sector->MoveBroadCast(pSession_, s);
	pSession_->MoveBroadcastEnqueue(0, 0, std::move(s));
	//const auto en = pSession_->GetOwnerEntity();
	//g_sector->BroadCastParallel(MoveBroadcaster::CreateMovePacket(en), s, en);
	//pSession_->GetCurWorld()->GetStartSector()->BroadCastParallel(MoveBroadcaster::CreateMovePacket(pSession_->GetOwnerEntity()), s, pSession_->GetOwnerEntity(),true);
	return true;
}



