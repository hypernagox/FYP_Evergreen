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
	// TODO: 아이디 통일
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
	// TODO: 빌더패턴 / 팩토리 패턴 처럼
	// 적당히 enum값이 오면 알아서 만들어서 씬 오브젝트 뱉는 구조가 필요하다.
	// 앞으로도 이렇게 하드코딩해서 객체를 만들 순 없다.
	const auto obj_id = pkt_.obj_id();
	
	// 해당 오브젝트의 HP 정보.
	// 이거 근데 HP 개념이없는 예) NPC같은거 있어서 그런애는 -1 주고있는데 걍 HP 컴포넌트 달고 1씩 넣고 공격불가로 할까
	const auto obj_max_hp = pkt_.obj_max_hp();
	const auto obj_cur_hp = pkt_.obj_cur_hp();

	if (Mgr(ServerObjectMgr)->GetServerObj(obj_id))
		return true;

	// if (pkt_.group_type() == 0)return true; // 스트레스 테스트용 주석 (너무 많으면 렌더링 바틀넥 감당불가)

	if (pkt_.group_type() == Nagox::Enum::GROUP_TYPE_NPC)
	{
		g_npcid = pkt_.obj_id();
		std::cout << "NPC 등장\n";
	}
	else if (pkt_.group_type() == Nagox::Enum::GROUP_TYPE_DROP_ITEM)
	{
		std::cout << "아이템 등장\n";
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
		std::cout << "NPC 퇴장\n";
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
	// 몬스터 공격 종류 및 애니메이션
	const auto obj = ServerObjectMgr::GetInst()->GetServerObj(pkt_.obj_id());
	if (nullptr != obj)
	{
		Monster* monsterComp = obj->GetComponent<Monster>();
		if (monsterComp)
		{
			monsterComp->OnAttackToPlayer();
		}
	}

	//std::cout << "여우가 당신에게 " << pkt_.dmg() << "데미지를 주었다 !" << std::endl;
	return true;
}

const bool Handle_s2c_NOTIFY_HIT_DMG(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_NOTIFY_HIT_DMG& pkt_)
{
	// 어떤 오브젝트가 몇 데미지 받았는가
	const auto hit_obj_id = pkt_.hit_obj_id(); // 맞은 애 아이디
	const auto hit_after_hp = pkt_.hit_after_hp();
	const auto hit_obj_ptr = Mgr(ServerObjectMgr)->GetServerObj(hit_obj_id);
	if (!hit_obj_ptr)
	{
		std::cout << std::format("Invalid hit Object ID from :{}", __FUNCTION__) << std::endl;
		return true;
	}
	if (const auto monster = hit_obj_ptr->GetComponent<Monster>())
	{
		// TODO: 현재체력과 힛 애프터의 차이가 필요,
		// 이 수치를 기록하고 관리할 클래스 있어야함
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
	std::cout << "여우가 당신을 주시하고있다 ... " << std::endl;
	return true;
}

const bool Handle_s2c_MONSTER_AGGRO_END(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_MONSTER_AGGRO_END& pkt_)
{
	std::cout << "아무래도 여우는 당신에게 흥미가 없어진 것 같다 ..." << std::endl;
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
		std::cout << "사망\n";
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
	std::cout << "퀘스트 수락 !" << std::endl;
	return true;
}

const bool Handle_s2c_CLEAR_QUEST(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_CLEAR_QUEST& pkt_)
{
	static int temp_count = 0;
	if (pkt_.is_clear())
	{
		temp_count = 0;
		std::cout << "퀘스트 클리어 !" << std::endl;
	}
	else
	{
		std::cout << "잡은 여우 수: " << ++temp_count << std::endl;
	}
	return true;
}

const bool Handle_s2c_FIRE_PROJ(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_FIRE_PROJ& pkt_)
{
	// TODO: 개 쌉 하드코딩 + 매넘
	const auto shoot_obj_id = pkt_.shoot_obj_id();
	const auto proj_type = pkt_.proj_type(); // TODO: 투사체의 타입 (아직 없음)
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
	// TODO: 만약 내가 아닌 다른 플레이어가 아이템 먹은걸 알아야 한다면 (예: XX님이 YY를 획득!) 누가 먹었는지 ID도 필요
	// 기획의 영역..
	
	// ID는 정수이나, json 테이블에서 해당 ID에 대응하는 아이템 정보를 획득하기 위해선 문자열로의 변환이 필요하다.
	Mgr(ServerObjectMgr)->RemoveObject(pkt_.item_obj_id());
	std::cout << std::format("아이템 획득함! 아이템 ID: {} 먹은 User ID: {} , 개수: {}\n", pkt_.item_detail_id(), pkt_.get_user_id(), pkt_.item_stack_size());

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
