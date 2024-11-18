#include "pch.h"
#include "c2s_PacketHandler.h"
#include "CreateBuffer4Server.h"
#include "../evergreen_server/ClientSession.h"
#include "../evergreen_server/PositionComponent.h"
#include "Queueabler.h"
#include "../evergreen_server/EntityFactory.h"
#include "Navigator.h"
#include "NaviCell.h"
#include "MoveBroadcaster.h"
#include "NaviAgent_Common.h"
#include "Collider_Common.h"
#include "Field.h"
#include "HP.h"


using namespace ServerCore;

flatbuffers::FlatBufferBuilder* const CreateBuilder()noexcept {
	thread_local flatbuffers::FlatBufferBuilder buillder{ 256 };
	return &buillder;
}

static inline Vector3 ToOriginVec3(const Nagox::Struct::Vec3* const v)noexcept {
	return Vector3{ v->x(),v->y(),v->z() };
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
	entity->AddComp<Collider>()->SetBox(entity->GetComp<PositionComponent>(), { 1,2,1.5f });
	//Mgr(WorldMgr)->GetWorld(0) ->GetStartSector()->BroadCastParallel(Create_s2c_APPEAR_OBJECT(pSession_->GetOwnerObjectID(), *pkt_.pos(), Nagox::Enum::OBJECT_TYPE_PLAYER));
	//Mgr(WorldMgr)->GetWorld(0)->EnterWorld(entity);
	
	Mgr(FieldMgr)->GetField(0)->EnterField(entity);

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
	
	//b.x = 14.6667f;
	//b.y = 0.0833333f;
	//b.z = 14.6667f;
	//static std::atomic_bool b2 = true;
	//
	//if (b2.exchange(false))
	//{
	//	const auto m = EntityFactory::CreateMonster(b);
	//	const auto cell = NAVIGATION->GetNavMesh(NUM_0)->FindCellWithClosestCenter({});
	//
	//	//m->GetComp<PositionComponent>()->pos.x = cell->GetCellCenter().x;
	//	//m->GetComp<PositionComponent>()->pos.y = cell->GetCellCenter().y;
	//	//m->GetComp<PositionComponent>()->pos.z = cell->GetCellCenter().z;
	//
	//	m->GetComp<Common::NaviAgent>()->Init(cell->GetCellCenter(), NAVIGATION->navMesh);
	//
	//	Mgr(WorldMgr)->GetWorld(0)->EnterWorldNPC(m);
	//}
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
	//Vector<Sector*> s{ pSession_->GetCurCluster() };

	//g_sector->BroadCastEnqueue(Create_s2c_MOVE(GetBuilder(), pEntity->GetObjectID(),
	//	ToFlatVec3(pEntity->pos),
	//	ToFlatVec3(pEntity->vel),
	//	ToFlatVec3(pEntity->accel),
	//	pEntity->body_angle,
	//	pEntity->time_stamp));
	//g_sector->MoveBroadCast(pSession_, s);
	
	pSession_->GetOwnerEntity()->GetComp<ServerCore::MoveBroadcaster>()->BroadcastMove();
	//pSession_->GetOwnerEntity()->GetComp<ServerCore::SectorInfoHelper>()->ImmigrationSector(0, 0);
	//const auto en = pSession_->GetOwnerEntity();
	//g_sector->BroadCastParallel(MoveBroadcaster::CreateMovePacket(en), s, en);
	//pSession_->GetCurWorld()->GetStartSector()->BroadCastParallel(MoveBroadcaster::CreateMovePacket(pSession_->GetOwnerEntity()), s, pSession_->GetOwnerEntity(),true);
	return true;
}

const bool Handle_c2s_PLAYER_ATTACK(const ServerCore::S_ptr<ServerCore::PacketSession>& pSession_, const Nagox::Protocol::c2s_PLAYER_ATTACK& pkt_)
{
	const auto pOwner = pSession_->GetOwnerEntity();

	const auto pos_comp = pOwner->GetComp<PositionComponent>();
	constexpr Vector3 forward(0.0f, 0.0f, 1.0f);

	const DirectX::SimpleMath::Matrix rotationMatrix = DirectX::SimpleMath::Matrix::CreateRotationY(pkt_.body_angle());

	const Vector3 rotatedForward = Vector3::Transform(forward, rotationMatrix);

	const auto& box = pOwner->GetComp<Collider>()->GetBox(rotatedForward);

	if (const auto sector = pOwner->GetCurCluster())
	{
		const auto& mon_list = sector->GetEntities(1);
		
		auto b = mon_list.data();
		const auto e = b + mon_list.size();
		while (e != b)
		{
			const auto pCol = (*b++)->GetComp<Collider>();
			const auto owner = pCol->GetOwnerEntity();
			if (pCol->IsCollision(box))
			{
				//NAVIGATION->GetNavMesh(NAVI_MESH_NUM::NUM_0)->GetCrowd()->getEditableAgent(owner->GetComp<NaviAgent>()->m_my_idx)->active = false;
				//
				//owner->TryOnDestroy();
				owner->GetComp<HP>()->PostDoDmg(1);
			}
		}
	}
	return true;
}

const bool Handle_c2s_PLAYER_DEATH(const ServerCore::S_ptr<ServerCore::PacketSession>& pSession_, const Nagox::Protocol::c2s_PLAYER_DEATH& pkt_)
{
	const auto owner = pSession_->GetOwnerEntity();

	const auto hp = owner->GetComp<HP>();
	hp->CompleteRebirth(5);

	return true;
}


