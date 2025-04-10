#include "pch.h"
#include <flatbuffers/flatbuffers.h>
#include "s2c_PacketHandler.h"
#include "../evergreen_client/ServerObjectMgr.h"
#include "../evergreen_client/ServerObject.h"
#include "../evergreen_client/MoveInterpolator.h"
#include "func.h"
#include "../evergreen_client/EntityBuilder.h"
#include "PlayerRenderer.h"
#include "Monster.h"
#include "ServerTimeMgr.h"
#include "GizmoSphereRenderer.h"
#include "Projectile.h"
#include "AuthenticPlayer.h"
#include "PlayerStatusGUI.h"
#include "PlayerQuickSlotGUI.h"

thread_local flatbuffers::FlatBufferBuilder buillder{ 256 };

flatbuffers::FlatBufferBuilder* const CreateBuilder()noexcept {
	extern thread_local flatbuffers::FlatBufferBuilder buillder;
	return &buillder;
}

extern std::shared_ptr<Scene> scene;
extern std::shared_ptr<SceneObject> g_heroObj;

#define Mgr(type)	(type::GetInst())

const bool Handle_s2c_LOGIN(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_LOGIN& pkt_)
{
	NetMgr(NetworkMgr)->SetSessionID(pkt_.obj_id());
	// TODO: ���̵� ����
	// g_heroObj->GetComponent<ServerObject>()->SetObjID(pkt_.obj_id());
	NetMgr(ServerTimeMgr)->UpdateServerTimeStamp(pkt_.server_time_stamp());
	return true;
}

const bool Handle_s2c_PING_PONG(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_PING_PONG& pkt_)
{
	//std::cout <<"RECV::::::"<< pkt_.server_time_stamp() << std::endl;
	NetMgr(ServerTimeMgr)->UpdateServerTimeStamp(pkt_.server_time_stamp());
	return true;
}

static uint32_t g_npcid = 0;
const bool Handle_s2c_APPEAR_OBJECT(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_APPEAR_OBJECT& pkt_)
{
	// TODO: �������� / ���丮 ���� ó��
	// ������ enum���� ���� �˾Ƽ� ���� �� ������Ʈ ��� ������ �ʿ��ϴ�.
	// �����ε� �̷��� �ϵ��ڵ��ؼ� ��ü�� ���� �� ����.
	const auto obj_id = pkt_.obj_id();
	
	// �ش� ������Ʈ�� HP ����.
	// �̰� �ٵ� HP �����̾��� ��) NPC������ �־ �׷��ִ� -1 �ְ��ִµ� �� HP ������Ʈ �ް� 1�� �ְ� ���ݺҰ��� �ұ�
	const auto obj_max_hp = pkt_.obj_max_hp();
	const auto obj_cur_hp = pkt_.obj_cur_hp();

	if (Mgr(ServerObjectMgr)->GetServerObj(obj_id))
		return true;

	// if (pkt_.group_type() == 0)return true; // ��Ʈ���� �׽�Ʈ�� �ּ� (�ʹ� ������ ������ ��Ʋ�� ����Ұ�)

	if (pkt_.group_type() == Nagox::Enum::GROUP_TYPE_NPC)
	{
		g_npcid = pkt_.obj_id();
		std::cout << "NPC ����\n";
	}
	else if (pkt_.group_type() == Nagox::Enum::GROUP_TYPE_DROP_ITEM)
	{
		std::cout << "������ ����\n";
	}
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
	if (pkt_.obj_id() == g_npcid)
	{
		std::cout << "NPC ����\n";
	}
	Mgr(ServerObjectMgr)->RemoveObject(pkt_.obj_id());
	return true;
}

const bool Handle_s2c_MOVE(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_MOVE& pkt_)
{
	if (pSession_->GetSessionID() == pkt_.obj_id()) 
	{
		constinit static uint64_t e_cnt = 0;
		++e_cnt;
		const auto et = NetMgr(ServerTimeMgr)->GetElapsedTime("MOVE_PKT");
		if (e_cnt % 10 == 0)
			std::cout << std::format("Delay: {}ms\n", et);
		return true;
	}
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
	// ���� ���� ���� �� �ִϸ��̼�
	const auto obj = ServerObjectMgr::GetInst()->GetServerObj(pkt_.obj_id());
	if (nullptr != obj)
	{
		Monster* monsterComp = obj->GetComponent<Monster>();
		if (monsterComp)
		{
			monsterComp->OnAttackToPlayer();
		}
	}

	//std::cout << "���찡 ��ſ��� " << pkt_.dmg() << "�������� �־��� !" << std::endl;
	return true;
}

const bool Handle_s2c_NOTIFY_HIT_DMG(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_NOTIFY_HIT_DMG& pkt_)
{
	// � ������Ʈ�� �� ������ �޾Ҵ°�
	const auto hit_obj_id = pkt_.hit_obj_id(); // ���� �� ���̵�
	const auto hit_after_hp = pkt_.hit_after_hp();
	const auto hit_obj_ptr = Mgr(ServerObjectMgr)->GetServerObj(hit_obj_id);
	if (!hit_obj_ptr)
	{
		std::cout << std::format("Invalid hit Object ID from :{}", __FUNCTION__) << std::endl;
		return true;
	}
	if (const auto monster = hit_obj_ptr->GetComponent<Monster>())
	{
		// TODO: ����ü�°� �� �������� ���̰� �ʿ�,
		// �� ��ġ�� ����ϰ� ������ Ŭ���� �־����
		monster->OnHit(hit_after_hp);
	}
	if (const auto player = hit_obj_ptr->GetComponent<PlayerRenderer>())
	{
		if (player->GetComponent<PlayerRenderer>()->TrySetState(PlayerRenderer::AnimationState::Hit))
			player->GetComponent<PlayerRenderer>()->Hit();
	}
	if (const auto player = hit_obj_ptr->GetComponent<AuthenticPlayer>())
	{
		player->OnHit(pkt_.hit_after_hp());
	}
	std::cout << std::format("HIT ID: {}, DMG: {}\n", hit_obj_id, 1);
	return true;
}

const bool Handle_s2c_MONSTER_AGGRO_START(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_MONSTER_AGGRO_START& pkt_)
{
	std::cout << "���찡 ����� �ֽ��ϰ��ִ� ... " << std::endl;
	return true;
}

const bool Handle_s2c_MONSTER_AGGRO_END(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_MONSTER_AGGRO_END& pkt_)
{
	std::cout << "�ƹ����� ����� ��ſ��� ��̰� ������ �� ���� ..." << std::endl;
	return true;
}

const bool Handle_s2c_PLAYER_ATTACK(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_PLAYER_ATTACK& pkt_)
{
	//if (g_heroObj->GetComponent<ServerObject>()->GetObjID() == pkt_.atk_player_id())return true;
	const auto atk_player = Mgr(ServerObjectMgr)->GetServerObj(pkt_.atk_player_id());
	if (!atk_player)return true;

	atk_player->GetTransform()->SetLocalRotation(Quaternion::CreateFromYawPitchRoll(pkt_.body_angle() * DEG2RAD + PI, 0.0f, 0.0f));
	atk_player->GetTransform()->SetLocalPosition(::ToOriginVec3(pkt_.atk_pos()));
	//atk_player->GetComponent<PlayerRenderer>()->Attack();
	atk_player->GetComponent<PlayerRenderer>()->TrySetState(PlayerRenderer::AnimationState::Attack);
	return true;
}

const bool Handle_s2c_PLAYER_DEATH(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_PLAYER_DEATH& pkt_)
{
	if (NetMgr(NetworkMgr)->GetSessionID() == pkt_.player_id())
	{
		std::cout << "���\n";
		g_heroObj->GetTransform()->SetLocalPosition(::ToOriginVec3(pkt_.rebirth_pos()));
		g_heroObj->GetComponent<PlayerRenderer>()->Death();
		NetMgr(NetworkMgr)->Send(Create_c2s_PLAYER_DEATH());
	}
	else
	{
		if (const auto obj = ServerObjectMgr::GetInst()->GetServerObj(pkt_.player_id()))
		{
			obj->GetComponent<PlayerRenderer>()->Death();
			obj->GetTransform()->SetLocalPosition(::ToOriginVec3(pkt_.rebirth_pos()));
		}
	}
	return true;
}

const bool Handle_s2c_REQUEST_QUEST(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_REQUEST_QUEST& pkt_)
{
	std::cout << "����Ʈ ���� !" << std::endl;
	return true;
}

const bool Handle_s2c_CLEAR_QUEST(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_CLEAR_QUEST& pkt_)
{
	static int temp_count = 0;
	if (pkt_.is_clear())
	{
		temp_count = 0;
		std::cout << "����Ʈ Ŭ���� !" << std::endl;
	}
	else
	{
		std::cout << "���� ���� ��: " << ++temp_count << std::endl;
	}
	return true;
}

const bool Handle_s2c_FIRE_PROJ(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_FIRE_PROJ& pkt_)
{
	// TODO: �� �� �ϵ��ڵ� + �ų�
	const auto shoot_obj_id = pkt_.shoot_obj_id();
	const auto proj_type = pkt_.proj_type(); // TODO: ����ü�� Ÿ�� (���� ����)
	auto s = std::make_shared<SceneObject>();
	s->GetTransform()->SetLocalPosition(::ToOriginVec3(pkt_.pos()));

	auto gizmoRenderer = s->AddComponent<GizmoSphereRenderer>();
	gizmoRenderer->SetRadius(1.0f);

	auto so = s->AddComponent<ServerObject>();
	const auto proj = so->AddComp<Projectile>();
	proj->m_speed = ::ToOriginVec3(pkt_.vel());
	so->SetObjID((uint32_t)pkt_.proj_id());
	Mgr(ServerObjectMgr)->AddObject(s);

	return true;
}

const bool Handle_s2c_ACQUIRE_ITEM(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_ACQUIRE_ITEM& pkt_)
{
	// TODO: ���� ���� �ƴ� �ٸ� �÷��̾ ������ ������ �˾ƾ� �Ѵٸ� (��: XX���� YY�� ȹ��!) ���� �Ծ����� ID�� �ʿ�
	// ��ȹ�� ����..
	
	// ID�� �����̳�, json ���̺��� �ش� ID�� �����ϴ� ������ ������ ȹ���ϱ� ���ؼ� ���ڿ����� ��ȯ�� �ʿ��ϴ�.
	Mgr(ServerObjectMgr)->RemoveObject(pkt_.item_obj_id());
	std::cout << std::format("������ ȹ����! ������ ID: {} ���� User ID: {} , ����: {}\n", pkt_.item_detail_id(), pkt_.get_user_id(), pkt_.item_stack_size());

	if (auto targetObject = Mgr(ServerObjectMgr)->GetServerObj(pkt_.get_user_id()))
	{
		if (auto playerComp = targetObject->GetComponent<AuthenticPlayer>())
			playerComp->OnModifyInventory(pkt_.item_detail_id(), pkt_.item_stack_size());
	}

	return true;
}

const bool Handle_s2c_USE_QUICK_SLOT_ITEM(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_USE_QUICK_SLOT_ITEM& pkt_)
{
	const auto use_user_id = pkt_.use_user_id();
	const auto item_id = pkt_.item_id();
	const auto quick_idx = pkt_.quick_slot_idx();
	const auto& gui = Mgr(ServerObjectMgr)->GetMainHero()->GetComponent<AuthenticPlayer>()->GetStatusGUI();
	gui->IncHP(1);

	if (auto targetObject = Mgr(ServerObjectMgr)->GetServerObj(use_user_id))
	{
		if (auto playerComp = targetObject->GetComponent<AuthenticPlayer>())
			playerComp->OnModifyInventory(item_id, -1);
	}

	return true;
}

const bool Handle_s2c_COMBINE_ITEM(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_COMBINE_ITEM& pkt_)
{
	return true;
}
