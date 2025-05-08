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
#include "DropItem.h"
#include "Item.h"
#include "PartyQuestSystem.h"
#include "Interaction.h"
#include "ClusterInfoHelper.h"

using namespace NagiocpX;

thread_local flatbuffers::FlatBufferBuilder buillder{ 256 };

flatbuffers::FlatBufferBuilder* const CreateBuilder()noexcept {
	extern thread_local flatbuffers::FlatBufferBuilder buillder;
	return &buillder;
}

static inline ClientSession* GetClientSession(const S_ptr<PacketSession>& session)noexcept {
	return static_cast<ClientSession*>(session.get());
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
	const auto pos = entity->GetComp<PositionComponent>()->pos;

	Field::GetField(0)->EnterFieldWithFloatXY(pos.x + 512.f, pos.z + 512.f, entity);

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
	const auto& pos_comp = pSession_->GetOwnerEntity()->GetComp<PositionComponent>();
	pos_comp->SetMovementInfo(pkt_);
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
	pSession_->GetOwnerEntity()->GetComp<NagiocpX::ClusterInfoHelper>()->AdjustCluster(
		pos_comp->GetXZWithOffset()
	);
	//pSession_->GetOwnerEntity()->GetComp<NagiocpX::SectorInfoHelper>()->ImmigrationSector(0, 0);
	//const auto en = pSession_->GetOwnerEntity();
	//g_sector->BroadCastParallel(MoveBroadcaster::CreateMovePacket(en), s, en);
	//pSession_->GetCurWorld()->GetStartSector()->BroadCastParallel(MoveBroadcaster::CreateMovePacket(pSession_->GetOwnerEntity()), s, pSession_->GetOwnerEntity(),true);
	return true;
}

const bool Handle_c2s_PLAYER_ATTACK(const NagiocpX::S_ptr<NagiocpX::PacketSession>& pSession_, const Nagox::Protocol::c2s_PLAYER_ATTACK& pkt_)
{
	DO_BENCH_GLOBAL_THIS_FUNC;
	
	// TODO: ���尡 �޶����ٸ� �丮��Ʈ�� ������ �ʿ�
	// TODO: �������ͷ� ������ ����
	// ������ ���̵� ���ų� ����带 ����
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
	pos_comp->pos = ::ToDxVec(pkt_.atk_pos());
	Common::Fan fan{ pos_comp->pos ,rotatedForward,30.f,8.f };
	fan.m_offSet = rotatedForward * 2;

	//if (const auto sector = pOwner->GetCurCluster())
	{
		const auto& mon_list = pOwner->GetComp<MoveBroadcaster>()->GetViewListNPC();
		const auto& player_list = pOwner->GetComp<MoveBroadcaster>()->GetViewListSession();
		for (const auto session : player_list)
		{
			//if (const auto pmon = Mgr(FieldMgr)->GetNPC(mon_id))
			{
				const auto session_ptr = GetSessionEntity(session.first);
				if (!session_ptr)continue;
				if (const auto pCol = session_ptr->GetComp<Collider>())
				{
					const auto owner = pCol->GetOwnerEntity();
					if (fan.IsIntersect(pCol->GetCollider()))
					{
						if (!pOwner->GetClientSession()->HasParty())continue;
						std::cout << "Player Hit\n";
						//if (!owner->GetClientSession()->HasParty())
						{
							auto pkt = Create_s2c_INVITE_PARTY_QUEST(pOwner->GetObjectID(), pOwner->GetClientSession()->m_party_quest_system->m_curQuestID
							);
							if (pOwner != owner.get() && !owner->GetClientSession()->HasParty())
							{
								///pOwner->GetClientSession()->AcceptNewPlayer(
								///	session_ptr->GetClientSession());
								 owner->GetClientSession()->SendAsync(pkt);
								 break;
							}
							//pOwner->GetClientSession()->SendAsync(pkt);
							//session_ptr->GetClientSession()->SendAsync(pkt);
						}
						
					}
				}

			}
		}
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
	// TODO: ������ �浹üũ ��ȿ�� �˻��ϱ� + ������ ���̵�
	// ������ Ŭ���� + ����� �Ұ�
	// �������� �ƿ� �ٸ� �����̳ʿ� �־ �����ϴ°� ���� �Ͱ���
	// ������ ������� ��� ���� ������ << �ؾߵ�
	
	const auto owner = pSession_->GetOwnerEntity();
	const auto pos = owner->GetComp<PositionComponent>()->pos;
	const auto& item_list = pSession_->GetOwnerEntity()->GetComp<NagiocpX::MoveBroadcaster>()->GetViewListNPC();
	for (const auto item : item_list)
	{
		const auto pos_comp = item->GetComp<PositionComponent>();
		if (!pos_comp)continue;
		if (!item->IsValid())continue;
		const auto item_pos = pos_comp->pos;
		if (const auto item_ptr = item->GetComp<DropItem>())
		{
			if (!CommonMath::IsInDistanceDX(pos, item_pos, 5.f))continue;
			owner->GetComp<NagiocpX::ClusterInfoHelper>()->BroadcastCluster(
				Create_s2c_ACQUIRE_ITEM(pSession_->GetSessionID(), item->GetObjectID(), item->GetDetailType(), item_ptr->GetNumOfItemStack()));
			const_cast<ContentsEntity*>(item)->TryOnDestroy();

			if (const auto inv = pSession_->GetOwnerEntity()->GetComp<Inventory>())
			{
				// inv->AddItem2Inventory()
				if (const auto add_item = inv->AddDropItem(item_ptr))
				{
					//std::cout << std::format("ȹ�� ������ ID:{} , ���� ����:{}�� \n", add_item->m_itemDetailType, add_item->m_numOfItemStack);
				}
				else
				{
					std::cout << "���� ����\n";
				}

			}
			else
			{
				std::cout << "���� ����\n";
			}
			break;
		}
	}
	//const auto cluster = pSession_->GetCurCluster();
	//if (const auto item = cluster->GetAllEntites()[Nagox::Enum::GROUP_TYPE_DROP_ITEM].FindItem((uint32_t)pkt_.item_id()))
	//{
	//	if (!item->IsValid())return true;
	//	if (const auto item_ptr = item->GetComp<DropItem>())
	//	{
	//		cluster->Broadcast(Create_s2c_ACQUIRE_ITEM(pSession_->GetSessionID(), item->GetObjectID(), item->GetDetailType(), item_ptr->GetNumOfItemStack()));
	//		item->TryOnDestroy();
	//
	//		if (const auto inv = pSession_->GetOwnerEntity()->GetComp<Inventory>())
	//		{
	//			// inv->AddItem2Inventory()
	//			if (const auto add_item = inv->AddDropItem(item_ptr))
	//			{
	//				std::cout << std::format("ȹ�� ������ ID:{} , ���� ����:{}�� \n", add_item->m_itemDetailType, add_item->m_numOfItemStack);
	//			}
	//			else
	//			{
	//				std::cout << "���� ����\n";
	//			}
	//			
	//		}
	//		else
	//		{
	//			std::cout << "���� ����\n";
	//		}
	//	}
	//}
	
	
	return true;
}

const bool Handle_c2s_REQUEST_QUICK_SLOT(const NagiocpX::S_ptr<NagiocpX::PacketSession>& pSession_, const Nagox::Protocol::c2s_REQUEST_QUICK_SLOT& pkt_)
{
	const auto owner = pSession_->GetOwnerEntity();
	if (const auto inv = owner->GetComp<Inventory>())
	{
		// TODO: ��ȯ������ �˷����ϳ�?
		std::cout << std::format("��� ������ ID:{} , �ε���:{} \n", pkt_.item_id(), pkt_.quick_slot_idx());
		if (!inv->SetQuickSlotItem(pkt_.item_id(), pkt_.quick_slot_idx()))
		{
			//std::cout << "�� ���µ� ����ħ\n";
		}
	}
	return true;
}

const bool Handle_c2s_USE_QUICK_SLOT_ITEM(const NagiocpX::S_ptr<NagiocpX::PacketSession>& pSession_, const Nagox::Protocol::c2s_USE_QUICK_SLOT_ITEM& pkt_)
{
	const auto owner = pSession_->GetOwnerEntity();
	if (const auto inv = owner->GetComp<Inventory>())
	{
		const auto item_id = inv->UseQuickSlotItem(pkt_.quick_slot_idx());
		if (-1 == item_id)
		{
			std::cout << "�� ����\n";
		}
		else
		{
			owner->GetComp<HP>()->PostDoHeal(1);
			pSession_->SendAsync(Create_s2c_USE_QUICK_SLOT_ITEM
			(owner->GetObjectID(),
				item_id,
				pkt_.quick_slot_idx())
			);
			std::cout << "���\n";
		}
	}
	return true;
}

const bool Handle_c2s_CRAFT_ITEM(const NagiocpX::S_ptr<NagiocpX::PacketSession>& pSession_, const Nagox::Protocol::c2s_CRAFT_ITEM& pkt_)
{
	const auto recipe_id = pkt_.recipe_id();
	const auto& recipe_info = DATA_TABLE->GetItemRecipe(recipe_id);
	if (const auto inv = pSession_->GetOwnerEntity()->GetComp<Inventory>())
	{
		if (inv->CraftItem(recipe_info))
		{
			inv->AddItem(recipe_info.resultItemID, recipe_info.numOfResultItem);
			pSession_->SendAsync(Create_s2c_CRAFT_ITEM(recipe_id));
			std::cout << "���� ��� ID: " << recipe_info.resultItemID << " ����: " << recipe_info.numOfResultItem << '\n';
		}
		else
		{
			std::cout << "����\n";
		}
	}
	
	return true;
}

const bool Handle_c2s_REGISTER_PARTY_QUEST(const NagiocpX::S_ptr<NagiocpX::PacketSession>& pSession_, const Nagox::Protocol::c2s_REGISTER_PARTY_QUEST& pkt_)
{
	// Ŭ���� ��Ƽ ���� ��û �Ǵ� ��Ƽ�� �ִµ� �� ����
	const auto session = GetClientSession(pSession_);
	session->CreatePartySystem();
	if (session->IsPartyLeader())
	{
		session->GetCurPartySystem()->m_curQuestID = pkt_.quest_id();
		session->SendAsync(Create_s2c_REGISTER_PARTY_QUEST(pkt_.quest_id()));
	}
	else
	{
		// ��Ƽ���� �ƴ�
	}
	return true;
}

const bool Handle_c2s_ACQUIRE_PARTY_LIST(const NagiocpX::S_ptr<NagiocpX::PacketSession>& pSession_, const Nagox::Protocol::c2s_ACQUIRE_PARTY_LIST& pkt_)
{
	const auto pOwner = pSession_->GetOwnerEntity();
	const auto& player_list = pOwner->GetComp<MoveBroadcaster>()->GetViewListSession();
	XVector<uint32_t> quest_leaders;
	// ��ó�� �ִ� ����� �� ��Ƽ���̸鼭 ���� ����Ʈ ���̵� ��û�ѰŶ� ��������� ���̵� ��Ƽ� ������
	for (const auto session : player_list)
	{
		const auto session_ptr = GetSessionEntity(session.first);
		if (!session_ptr)continue;
		if (const auto s = session_ptr->GetClientSession())
		{
			if (s->IsPartyLeader() && s->m_party_quest_system->m_curQuestID == pkt_.target_quest_id())
			{
				quest_leaders.emplace_back(session_ptr->GetObjectID());
			}
		}
	}

	const auto session = GetClientSession(pSession_);
	if (session->IsPartyLeader() && session->m_party_quest_system->m_curQuestID == pkt_.target_quest_id())
	{
		quest_leaders.emplace_back(pSession_->GetSessionID());
	}
	pSession_->SendAsync(Create_s2c_ACQUIRE_PARTY_LIST(std::move(quest_leaders), pkt_.target_quest_id()));
	return true;
}

const bool Handle_c2s_INVITE_PARTY_QUEST(const NagiocpX::S_ptr<NagiocpX::PacketSession>& pSession_, const Nagox::Protocol::c2s_INVITE_PARTY_QUEST& pkt_)
{
	const auto target_user_id = pkt_.target_user_id();
	const auto target_user = GetSessionEntity(target_user_id);
	const auto session = GetClientSession(pSession_);
	if (!target_user)return true;
	if (!session->IsPartyLeader())return true;
	// ��Ƽ���� Ÿ�� �������� �ڽ��� ���̵�� ���� ����Ʈ ���̵� ������
	target_user->GetSession()->SendAsync(Create_s2c_INVITE_PARTY_QUEST(pSession_->GetSessionID(), session->m_party_quest_system->m_curQuestID));
	return true;
}

const bool Handle_c2s_INVITE_PARTY_RESULT(const NagiocpX::S_ptr<NagiocpX::PacketSession>& pSession_, const Nagox::Protocol::c2s_INVITE_PARTY_RESULT& pkt_)
{
	const auto party_leader = GetSessionEntity(pkt_.target_party_leader_id());
	auto pkt = Create_s2c_INVITE_PARTY_RESULT(pkt_.target_party_leader_id(), pSession_->GetSessionID(), pkt_.is_accept());
	if (party_leader->GetClientSession()->IsPartyLeader())
	{
		// ��Ƽ�忡�� ok / no ������ ����
		party_leader->GetClientSession()->SendAsync(pkt);
		if (pkt_.is_accept())
		{
			// �����ߴٸ� Ÿ�ٿ��Ե� ���� + ��Ƽ������
			party_leader->GetClientSession()->AcceptNewPlayer(GetClientSession(pSession_));
			pSession_->SendAsync(pkt);
		}
	}
	return true;
}

const bool Handle_c2s_PARTY_JOIN_REQUEST(const NagiocpX::S_ptr<NagiocpX::PacketSession>& pSession_, const Nagox::Protocol::c2s_PARTY_JOIN_REQUEST& pkt_)
{
	const auto party_leader = GetSessionEntity(pkt_.target_party_leader_id());
	const auto party_leader_session = party_leader->GetClientSession();
	if (!party_leader)return true;
	if (party_leader_session->IsPartyLeader() && !GetClientSession(pSession_)->HasParty())
	{
		auto pkt = Create_s2c_PARTY_JOIN_REQUEST(pSession_->GetSessionID());
		party_leader_session->SendAsync(std::move(pkt));
	}
	return true;
}

const bool Handle_c2s_PARTY_JOIN_REQUEST_RESULT(const NagiocpX::S_ptr<NagiocpX::PacketSession>& pSession_, const Nagox::Protocol::c2s_PARTY_JOIN_REQUEST_RESULT& pkt_)
{
	const auto party_leader = GetSessionEntity(pkt_.target_party_leader_id());
	const auto target_member = GetSessionEntity(pkt_.target_user_id());
	if (!party_leader) {
		// ��Ƽ���� ���� ���
		return true;
	}
	if (!target_member) {
		// �����ڰ� ��û���ϰ� �����������
		return true;
	}
	const auto party_leader_session = party_leader->GetClientSession();
	const auto target_member_session = target_member->GetClientSession();
	if (target_member_session->HasParty()) {
		// �����ڰ� �̹� ��Ƽ�� ���ִ� ������ ���
		return true;
	}
	if (party_leader->GetClientSession()->IsPartyLeader())
	{
		auto pkt = Create_s2c_PARTY_JOIN_REQUEST_RESULT(target_member->GetObjectID(), pkt_.request_result());

		// �����ڿ��Դ� ��¶�ų� �� ����� ������ ������.
		target_member_session->SendAsync(pkt);

		// ��Ƽ����� �����ߴٸ� ��Ƽ�� �ְ� ��Ƽ�忡�Ե� �ش� ����� �˸���.
		if (pkt_.request_result())
		{
			// + ��Ƽ���, �ش� ������ ���� �ٸ� �༮���� ���� �Դٸ� �˸��� �� �߰�
			// + ���� �ڸ��� ���ٴ��� ���� ������ �־ ��Ƽ�� �ִ� �Ϳ� �����ߴٸ� ���� ó�� ..
			if (PARTY_ACCEPT_RESULT::INVALID != party_leader_session->AcceptNewPlayer(target_member_session))
			{
				party_leader_session->SendAsync(std::move(pkt));
			}

		}
	}
	return true;
}

const bool Handle_c2s_QUEST_START(const NagiocpX::S_ptr<NagiocpX::PacketSession>& pSession_, const Nagox::Protocol::c2s_QUEST_START& pkt_)
{
	const auto party_leader = GetClientSession(pSession_);
	if (!party_leader->IsPartyLeader()) {
		return true;
	}
	
	if (!party_leader->m_party_quest_system->MissionStart()) {
		std::cout << party_leader->m_party_quest_system->m_started << std::endl;
		std::cout << party_leader->m_party_quest_system->m_runFlag << std::endl;
		std::cout << party_leader->m_party_quest_system->m_curQuestRoomInstance.UseCount() << std::endl;
	}
	return true;
}

const bool Handle_c2s_QUEST_END(const NagiocpX::S_ptr<NagiocpX::PacketSession>& pSession_, const Nagox::Protocol::c2s_QUEST_END& pkt_)
{
	const auto party_leader = GetClientSession(pSession_);
	if (!party_leader->IsPartyLeader()) {
		std::cout << "Not leader\n";
		return true;
	}
	const auto session = GetClientSession(pSession_);
	if (!session->HasParty()) {
		std::cout << "No party\n";
		return true;
	}
	const auto cur_party = session->GetCurPartySystem();
	if (!cur_party) {
		std::cout << "No party\n";
		return true;
	}
	if (pSession_->GetOwnerEntity()->GetClusterFieldInfo().clusterInfo.fieldID != -1) {
		std::cout << "Not Party Quest\n";
		return true;
	}
	if (!cur_party->m_curQuestRoomInstance->IsClear()) {
		std::cout << "Not Clear \n";
		return true;
	}
	party_leader->m_party_quest_system->MissionEnd();
	return true;
}

const bool Handle_c2s_PARTY_OUT(const NagiocpX::S_ptr<NagiocpX::PacketSession>& pSession_, const Nagox::Protocol::c2s_PARTY_OUT& pkt_)
{
	const auto session = GetClientSession(pSession_);
	if (!session->HasParty()) { 
		std::cout << "No party\n";
		return true;
	}
	const auto cur_party = session->GetCurPartySystem();
	if (!cur_party) { 
		std::cout << "No party\n";
		return true;
	}
	if (cur_party->m_started) {
		// TODO: ���� ���� ��������, �Ժη� ��������.
		std::cout << "run \n";
		return true;
	}
	if (const auto p = session->GetCurPartySystem())
	{
		p->OutMember(session->GetSessionID());
	}
	//if (session->IsPartyLeader())
	//{
	//	session->m_party_quest_system.ResetPartyQuestSystem();
	//}
	//else
	//{
	//	if (const auto p = session->GetCurPartySystem())
	//	{
	//		p->OutMember(session->GetSessionID());
	//	}
	//}
	return true;
}

const bool Handle_c2s_CHANGE_HARVEST_STATE(const NagiocpX::S_ptr<NagiocpX::PacketSession>& pSession_, const Nagox::Protocol::c2s_CHANGE_HARVEST_STATE& pkt_)
{
	const auto owner = pSession_->GetOwnerEntity();
	const auto& harvests = pSession_->GetCurCluster()->GetEntities(Nagox::Enum::GROUP_TYPE_HARVEST);
	const auto player_pos = owner->GetComp<PositionComponent>()->pos;
	for (const auto& h : harvests)
	{
		const auto& harvest_pos = h->GetComp<PositionComponent>()->pos;
		if (CommonMath::IsInDistanceDX(player_pos, harvest_pos, HARVEST_INTERACTION_DIST))
		{
			if (h->GetComp<Interaction>()->DoInteraction(owner))
			{
				//break;
			}
		}
	}
	return true;
}


