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
#include "GizmoBoxRenderer.h"
#include "Projectile.h"

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
	//g_heroObj->GetComponent<ServerObject>()->SetObjID(pkt_.obj_id());
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
	
	if (Mgr(ServerObjectMgr)->GetServerObj(obj_id))
		return true;

	// if (pkt_.group_type() == 0)return true; // 스트레스 테스트용 주석 (너무 많으면 렌더링 바틀넥 감당불가)

	if (pkt_.group_type() == Nagox::Enum::GROUP_TYPE_NPC)
	{
		g_npcid = pkt_.obj_id();
		std::cout << "NPC 등장\n";
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
	const auto obj = ServerObjectMgr::GetInst()->GetServerObj(pkt_.obj_id());
	if (!obj)return true;
	Monster* monsterComp = obj->GetComponent<Monster>();
	if (monsterComp)
	{
		monsterComp->OnAttackToPlayer();
	}

	const auto player = Mgr(ServerObjectMgr)->GetServerObj(pkt_.player_id());
	if (player)
	{
		if(player->GetComponent<PlayerRenderer>()->TrySetState(PlayerRenderer::AnimationState::Hit))
			player->GetComponent<PlayerRenderer>()->Hit();
	}

	else if (NetMgr(NetworkMgr)->GetSessionID() == pkt_.player_id())
	{

		if (g_heroObj->GetComponent<PlayerRenderer>()->TrySetState(PlayerRenderer::AnimationState::Hit))
			g_heroObj->GetComponent<PlayerRenderer>()->Hit();
	}

	//std::cout << "여우가 당신에게 " << pkt_.dmg() << "데미지를 주었다 !" << std::endl;
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
	auto s = std::make_shared<SceneObject>();
	s->GetTransform()->SetLocalPosition(::ToOriginVec3(pkt_.pos()));

	auto gizmoRenderer = s->AddComponent<GizmoBoxRenderer>();
	gizmoRenderer->SetSize(Vector3(1.5f, 3.5f, 1.5f));
	gizmoRenderer->SetOffset(Vector3());

	auto so = s->AddComponent<ServerObject>();
	const auto proj = so->AddComp<Projectile>();
	proj->m_speed = ::ToOriginVec3(pkt_.vel());
	so->SetObjID((uint32_t)pkt_.proj_id());
	Mgr(ServerObjectMgr)->AddObject(s);

	return true;
}

