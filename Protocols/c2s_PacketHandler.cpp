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
#include "QuestSystem.h"
#include "Quest.h"
#include "ClusterPredicate.h"
#include "Projectile.h"
#include "Inventory.h"

using namespace NagiocpX;

thread_local flatbuffers::FlatBufferBuilder buillder{ 256 };

flatbuffers::FlatBufferBuilder* const CreateBuilder()noexcept {
	extern thread_local flatbuffers::FlatBufferBuilder buillder;
	return &buillder;
}

const bool Handle_c2s_LOGIN(const NagiocpX::S_ptr<NagiocpX::PacketSession>& pSession_, const Nagox::Protocol::c2s_LOGIN& pkt_)
{
	pSession_ << Create_s2c_LOGIN((uint32_t)pSession_->GetSessionID(), Mgr(TimeMgr)->GetServerTimeStamp());
	return true;
}

const bool Handle_c2s_PING_PONG(const NagiocpX::S_ptr<NagiocpX::PacketSession>& pSession_, const Nagox::Protocol::c2s_PING_PONG& pkt_)
{
	//std::cout << Mgr(TimeMgr)->GetServerTimeStamp() << std::endl;
	pSession_ << Create_s2c_PING_PONG(Mgr(TimeMgr)->GetServerTimeStamp());
	return true;
}

const bool Handle_c2s_ENTER(const NagiocpX::S_ptr<NagiocpX::PacketSession>& pSession_, const Nagox::Protocol::c2s_ENTER& pkt_)
{
	auto entity = pSession_->GetOwnerEntity();

	//pSession_->SetEntity(entity);
	//entity->AddIocpComponent<Queueabler>();
	entity->AddComp<PositionComponent>()->pos = ToDxVec(pkt_.pos());
	entity->AddComp<AABBCollider>()->SetAABB(entity->GetComp<PositionComponent>(), { 1,2,1.5f });

	//Mgr(WorldMgr)->GetWorld(0) ->GetStartSector()->BroadCastParallel(Create_s2c_APPEAR_OBJECT(pSession_->GetOwnerObjectID(), *pkt_.pos(), Nagox::Enum::OBJECT_TYPE_PLAYER));
	//Mgr(WorldMgr)->GetWorld(0)->EnterWorld(entity);
	
	Mgr(FieldMgr)->GetField(0)->EnterField(entity);

	//g_sector->BroadCastParallel(Create_s2c_APPEAR_OBJECT(pSession_->GetOwnerObjectID(), *pkt_.pos(), Nagox::Enum::OBJECT_TYPE_PLAYER)
	//	, s
	//	, entity
	//);
	//NagiocpX::PrintLogEndl("enter");
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

const bool Handle_c2s_MOVE(const NagiocpX::S_ptr<NagiocpX::PacketSession>& pSession_, const Nagox::Protocol::c2s_MOVE& pkt_)
{
	if (pSession_->GetOwnerEntity()->IsPendingClusterEntry())return true;
	DO_BENCH_GLOBAL_THIS_FUNC;
	//if(pSession_->GetObjectID()!=1)
	
	//std::cout << "move" << std::endl;
	const auto& pEntity = pSession_->GetOwnerEntity()->GetComp<PositionComponent>();
	pEntity->SetMovementInfo(pkt_);
	//pEntity->pos = ToDxVec(pkt_.pos());
	//pEntity->vel = ToDxVec(pkt_.vel());
	//pEntity->accel = ToDxVec(pkt_.accel());
	//pEntity->body_angle = pkt_.body_angle();
	//pEntity->time_stamp = pkt_.time_stamp();
	//Vector<Sector*> s{ pSession_->GetCurCluster() };

	//g_sector->BroadCastEnqueue(Create_s2c_MOVE(GetBuilder(), pEntity->GetObjectID(),
	//	ToFlatVec3(pEntity->pos),
	//	ToFlatVec3(pEntity->vel),
	//	ToFlatVec3(pEntity->accel),
	//	pEntity->body_angle,
	//	pEntity->time_stamp));
	//g_sector->MoveBroadCast(pSession_, s);
	ClusterPredicate helper;
	//pSession_->SendAsync(MoveBroadcaster::CreateMovePacket(pSession_->GetOwnerEntity()));
	pSession_->SendAsync(helper.CreateMovePacket(pSession_->GetOwnerEntity()));
	pSession_->GetOwnerEntity()->GetComp<NagiocpX::MoveBroadcaster>()->BroadcastMove(helper);
	//pSession_->GetOwnerEntity()->GetComp<NagiocpX::SectorInfoHelper>()->ImmigrationSector(0, 0);
	//const auto en = pSession_->GetOwnerEntity();
	//g_sector->BroadCastParallel(MoveBroadcaster::CreateMovePacket(en), s, en);
	//pSession_->GetCurWorld()->GetStartSector()->BroadCastParallel(MoveBroadcaster::CreateMovePacket(pSession_->GetOwnerEntity()), s, pSession_->GetOwnerEntity(),true);
	return true;
}

const bool Handle_c2s_PLAYER_ATTACK(const NagiocpX::S_ptr<NagiocpX::PacketSession>& pSession_, const Nagox::Protocol::c2s_PLAYER_ATTACK& pkt_)
{
	DO_BENCH_GLOBAL_THIS_FUNC;
	
	// TODO: 생포인터로 개기지 말자
	// 정수값 아이디만 쓰거나 쉐어드를 쓰자
	const auto pOwner = pSession_->GetOwnerEntity();

	const auto pos_comp = pOwner->GetComp<PositionComponent>();
	constexpr Vector3 forward(0.0f, 0.0f, 1.0f);
	
	const DirectX::SimpleMath::Matrix rotationMatrix = DirectX::SimpleMath::Matrix::CreateRotationY(pkt_.body_angle());

	//std::cout << "MY angle: " << pkt_.body_angle() << '\n';
	//std::cout << "Mypos: ";
	//PrintLogEndl(&pos_comp->pos.x);
	const Vector3 rotatedForward = Vector3::Transform(forward, rotationMatrix);
	auto c = pOwner->GetComp<AABBCollider>()->GetCollider<Common::AABBBox>();
	c->m_offSet = rotatedForward;
	auto box = c->GetAABB();
	bool isHit = false;
	Common::Fan fan{ pos_comp->pos ,rotatedForward,30.f,8.f };
	fan.m_offSet = rotatedForward * 2;
	pos_comp->pos = ::ToDxVec(pkt_.atk_pos());
	//if (const auto sector = pOwner->GetCurCluster())
	{
		const auto& mon_list = pOwner->GetComp<MoveBroadcaster>()->GetViewListNPC();
		//std::cout << std::format("Session ID: {}, Num Of Mon in Viewlist: {}\n", pOwner->GetObjectID(), mon_list.size());
		for (const auto pmon : mon_list)
		{
			//if (const auto pmon = Mgr(FieldMgr)->GetNPC(mon_id))
			{
				if (const auto pCol = pmon->GetComp<Collider>())
				{
					const auto owner = pCol->GetOwnerEntity();
					if(fan.IsIntersect(pCol->GetCollider()))
					//if (pCol->IsCollision(box))
					{
						//NAVIGATION->GetNavMesh(NAVI_MESH_NUM::NUM_0)->GetCrowd()->getEditableAgent(owner->GetComp<NaviAgent>()->m_my_idx)->active = false;
						//
						//owner->TryOnDestroy();
						owner->GetComp<HP>()->PostDoDmg(1, pOwner->SharedFromThis());
						isHit = true;
					}
					else
					{
						//const auto ppp = pCol->GetCollider()->GetPosWithOffset();
						//PrintLogEndl(&ppp.x);
					}
				}
	
			}
		}
	}
	{
		pOwner->GetComp<MoveBroadcaster>()->BroadcastPacket(Create_s2c_PLAYER_ATTACK(pOwner->GetObjectID64(), pkt_.body_angle(), *pkt_.atk_pos()));
	}
	if (isHit)
	{
		//std::cout << "Hit!\n";
	}
	return true;
}

const bool Handle_c2s_PLAYER_DEATH(const NagiocpX::S_ptr<NagiocpX::PacketSession>& pSession_, const Nagox::Protocol::c2s_PLAYER_DEATH& pkt_)
{
	const auto owner = pSession_->GetOwnerEntity();

	const auto hp = owner->GetComp<HP>();
	hp->CompleteRebirth(5);

	return true;
}

const bool Handle_c2s_REQUEST_QUEST(const NagiocpX::S_ptr<NagiocpX::PacketSession>& pSession_, const Nagox::Protocol::c2s_REQUEST_QUEST& pkt_)
{
	const auto owner = pSession_->GetOwnerEntity();
	const auto q = NagiocpX::xnew<KillFoxQuest>(0);
	if (!owner->GetComp<QuestSystem>()->AddQuest(q))
	{
		xdelete<Quest>(q);
	}
	else
	{
		pSession_->SendAsync(Create_s2c_REQUEST_QUEST(0));
	}
	return true;
}

const bool Handle_c2s_FIRE_PROJ(const NagiocpX::S_ptr<NagiocpX::PacketSession>& pSession_, const Nagox::Protocol::c2s_FIRE_PROJ& pkt_)
{
	const auto pOwner = pSession_->GetOwnerEntity();

	const auto pos_comp = pOwner->GetComp<PositionComponent>();
	constexpr Vector3 forward(0.0f, 0.0f, 1.0f);

	const DirectX::SimpleMath::Matrix rotationMatrix = DirectX::SimpleMath::Matrix::CreateRotationY(pkt_.body_angle());

	const Vector3 rotatedForward = Vector3::Transform(forward, rotationMatrix);


	const auto proj = TimerHandler::CreateTimerWithoutHandle<PlayerProjectile>(100);
	proj.timer->m_pos = (pos_comp->pos);
	proj.timer->m_speed = rotatedForward * 40.f;
	proj.timer->SelectObjList(pOwner->GetComp<MoveBroadcaster>()->GetViewListNPC());
	proj.timer->m_owner = pOwner->SharedFromThis();

	return true;
}

const bool Handle_c2s_ACQUIRE_ITEM(const NagiocpX::S_ptr<NagiocpX::PacketSession>& pSession_, const Nagox::Protocol::c2s_ACQUIRE_ITEM& pkt_)
{
	// TODO: 아이템 충돌체크 유효성 검사하기 + 아이템 아이디
	// 아이템 클래스 + 아토믹 불값
	// 아이템을 아예 다른 컨테이너에 넣어서 관리하는게 맞을 것같음
	// 아이템 사라졌단 사실 따로 보내기
	const auto cluster = pSession_->GetCurCluster();
	if (const auto item = cluster->GetAllEntites()[Nagox::Enum::GROUP_TYPE_DROP_ITEM].FindItem((uint32_t)pkt_.item_id()))
	{
		if (!item->IsValid())return true;
		pSession_->SendAsync(Create_s2c_ACQUIRE_ITEM(item->GetDetailType()));
		item->TryOnDestroy();
		
		if (const auto inv = pSession_->GetOwnerEntity()->GetComp<Inventory>())
		{
			// inv->AddItem2Inventory()
			std::cout << "아이템 획득\n";
		}
		else
		{
			std::cout << "문제 있음\n";
		}
	}
	
	
	return true;
}

