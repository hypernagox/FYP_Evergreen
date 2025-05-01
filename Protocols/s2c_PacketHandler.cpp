#include "pch.h"
#include <flatbuffers/flatbuffers.h>
#include "s2c_PacketHandler.h"
#include "../evergreen_client/MoveInterpolator.h"
#include "../evergreen_client/ServerObjectMgr.h"
#include "../evergreen_client/EntityBuilder.h"
#include "../evergreen_client/ServerObject.h"
#include "func.h"
#include "PlayerRenderer.h"
#include "Monster.h"
#include "ServerTimeMgr.h"
#include "GizmoSphereRenderer.h"
#include "Projectile.h"
#include "AuthenticPlayer.h"
#include "PlayerStatusGUI.h"
#include "PlayerQuickSlotGUI.h"
#include "GameGUIFacade.h"
#include "PartyListGUI.h"
#include "LogFloatGUI.h"
#include "RequestPopupGUI.h"
#include "PartyStatusGUI.h"
#include "GuideSystem.h"

thread_local flatbuffers::FlatBufferBuilder buillder{ 256 };

flatbuffers::FlatBufferBuilder* const CreateBuilder()noexcept {
	extern thread_local flatbuffers::FlatBufferBuilder buillder;
	return &buillder;
}

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
		//const auto et = NetMgr(ServerTimeMgr)->GetElapsedTime("MOVE_PKT");
		//if (e_cnt % 10 == 0)
		//	std::cout << std::format("Delay: {}ms\n", et);
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
		// 이제는 빠른삭제조치를 위해 여러번 삭제 패킷이 올 수 있음
	//	std::cout << std::format("Invalid hit Object ID from :{}", __FUNCTION__) << std::endl;
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
	//std::cout << std::format("HIT ID: {}, DMG: {}\n", hit_obj_id, 1);
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
		auto heroObject = Mgr(ServerObjectMgr)->GetMainHero();
		heroObject->GetTransform()->SetLocalPosition(::ToOriginVec3(pkt_.rebirth_pos()));
		heroObject->GetComponent<PlayerRenderer>()->Death();
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

const bool Handle_s2c_CRAFT_ITEM(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_CRAFT_ITEM& pkt_)
{
	// TODO: 레시피 사용 요청에 대한 답변패킷이 여기로 옴
	// 서버에서는 이미 인벤토리에 이런저런 수정사항이 반영 된 상태,
	// 사용한 레시피 아이디 하나만 주면 그 레시피를 찾아서 내가 지금 뭐가 몇 개 없어지고
	// 뭐가 몇 개 생겨야 할 지 알 수 있어잉
	const auto& recipe_info = DATA_TABLE->GetItemRecipe(pkt_.recipe_id());
	const auto recipe_id = recipe_info.recipeID;
	const auto& item_info = recipe_info.itemElements;
	std::cout << recipe_id << '\n';
	std::cout << DATA_TABLE->GetRecipeName(recipe_id) << '\n';

	// 레시피에서 읽은 아이템 리스트 정보를 통해 플레이어의 인벤토리를 수정한다.
	if (auto playerComp = Mgr(ServerObjectMgr)->GetMainHero()->GetComponent<AuthenticPlayer>())
	{
		const auto& combine_list = GET_RECIPE(DATA_TABLE->GetRecipeName(recipe_id));
		for (const auto& [itemName, itemId, numOfRequire] : combine_list.itemElements)
			playerComp->OnModifyInventory(itemId, -numOfRequire);
		playerComp->OnModifyInventory(combine_list.resultItemID, combine_list.numOfResultItem);
	}

	return true;
}

const bool Handle_s2c_REGISTER_PARTY_QUEST(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_REGISTER_PARTY_QUEST& pkt_)
{
	// 클라이언트의 파티퀘스트 생성 또는 파티는 있는데 대상 퀘스트를 바꾸는 시도에 대한 답변
	std::cout << "현재 파티퀘스트 ID: " << pkt_.quest_id() << std::endl;

	std::vector<uint32_t> partyMemberIDs;
	partyMemberIDs.emplace_back(pSession_->GetSessionID());
	INSTANCE(GameGUIFacade)->PartyStatus->InitializeContents(partyMemberIDs);

	return true;
}

const bool Handle_s2c_ACQUIRE_PARTY_LIST(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_ACQUIRE_PARTY_LIST& pkt_)
{
	std::cout << "파티퀘스트 ID: " << pkt_.target_quest_id() << " 의 목록들\n";
	for (const auto v : *pkt_.party_leader_ids())
	{
		std::cout << "파티장 ID: " << v << std::endl;
	}

	auto partyListGUI = INSTANCE(GameGUIFacade)->PartyList;
	std::vector<uint32_t> partyList(pkt_.party_leader_ids()->cbegin(), pkt_.party_leader_ids()->cend());
	partyListGUI->UpdateContents(partyList);

	return true;
}

const bool Handle_s2c_INVITE_PARTY_QUEST(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_INVITE_PARTY_QUEST& pkt_)
{
	// 다른 사람으로부터 온 파티 초대
	// 여기서 다른 파티장으로 부터 파티 초대가 온다.

	// 팝업 메시지 송출
	INSTANCE(GameGUIFacade)->RequestPopup->ShowPopup(
		L"파티 초대 알림",
		L"ID " + std::to_wstring(pkt_.target_party_leader_id()) + L" 님이 파티에 초대하였습니다.",
		[id = pkt_.target_party_leader_id()]() {
			Send(Create_c2s_INVITE_PARTY_RESULT(id, true));
		},
		[id = pkt_.target_party_leader_id()]() {
			Send(Create_c2s_INVITE_PARTY_RESULT(id, false));
		}
	);

	return true;
}

const bool Handle_s2c_INVITE_PARTY_RESULT(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_INVITE_PARTY_RESULT& pkt_)
{
	// TODO: 내가 파티장이면 수락 여부에 대한 정보가,
	// 내가 초대 당한 사람이면 거절하면 안옴
	// 아직은 그냥 가서 떄리면 무조건 신청이고, 신청당한 사람은 무조건 수락

	if (pSession_->GetSessionID() == pkt_.target_party_leader_id())
	{
		if (pkt_.invite_result())
		{
			std::cout << "아이디 " << pkt_.target_user_id() << "가 수락" << std::endl;
		}
		else
		{
			std::cout << "아이디 " << pkt_.target_user_id() << "가 거절" << std::endl;
		}
	}
	else
	{
		if(pkt_.invite_result())
			std::cout << "파티장 ID " << pkt_.target_party_leader_id() << " 인 파티 신청을 내가 받아줌\n";
		else
			std::cout << "파티장 ID " << pkt_.target_party_leader_id() << " 인 파티 신청 안받아줌 ㅅㄱ\n";
	}

	std::wstring name = std::to_wstring(pkt_.target_user_id());
	std::wstring leaderName = std::to_wstring(pkt_.target_party_leader_id());
	// 파티장일 경우
	if (pSession_->GetSessionID() == pkt_.target_party_leader_id())
	{
		if (pkt_.invite_result())
			INSTANCE(GameGUIFacade)->LogFloat->AddText(name + L" 님이 파티에 가입하였습니다.");
		else
			INSTANCE(GameGUIFacade)->LogFloat->AddText(name + L" 님이 파티에 가입을 거절하였습니다.");
	}
	// 초대를 받은 유저일 경우
	else
	{
		if (pkt_.invite_result())
			INSTANCE(GameGUIFacade)->LogFloat->AddText(L"파티(파티장 " + leaderName + L")에 가입하였습니다.");
		else
			INSTANCE(GameGUIFacade)->LogFloat->AddText(L"파티(파티장 " + leaderName + L")의 가입 초대를 거절하였습니다.");
	}

	return true;
}

const bool Handle_s2c_PARTY_JOIN_REQUEST(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_PARTY_JOIN_REQUEST& pkt_)
{
	// 내가 파티장 일 때 다른 사람의 파티요청이 여기로 온다

	// 팝업 메시지 송출
	INSTANCE(GameGUIFacade)->RequestPopup->ShowPopup(
		L"가입 요청 알림",
		L"ID " + std::to_wstring(pkt_.target_user_id()) + L" 님이 파티에 가입 요청을 하였습니다.",
		[lid = pSession_->GetSessionID(), id = pkt_.target_user_id()]() {
			Send(Create_c2s_PARTY_JOIN_REQUEST_RESULT(lid, id, true));
		},
		[lid = pSession_->GetSessionID(), id = pkt_.target_user_id()]() {
			Send(Create_c2s_PARTY_JOIN_REQUEST_RESULT(lid, id, false));
		}
	);

	return true;
}

const bool Handle_s2c_PARTY_JOIN_REQUEST_RESULT(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_PARTY_JOIN_REQUEST_RESULT& pkt_)
{
	// TODO: 내가 파티장이라면, 이전 파티 가입 요청에 대해서 승인 했을 경우 그 사람을 알려주기 위해 패킷이온다
	// 내가 파티 지원자라면 파티장님의 수락 여부가 온다.
	std::wstring name = std::to_wstring(pkt_.target_user_id());
	// 요청한 유저일 경우
	if (pSession_->GetSessionID() == pkt_.target_user_id())
	{
		if (pkt_.request_result())
			INSTANCE(GameGUIFacade)->LogFloat->AddText(L"파티에 가입하였습니다.");
		else
			INSTANCE(GameGUIFacade)->LogFloat->AddText(L"파티가 가입 신청을 거절하였습니다.");
	}
	// 파티장일 경우
	else
	{
		if (pkt_.request_result())
			INSTANCE(GameGUIFacade)->LogFloat->AddText(L"ID " + name + L" 님이 파티에 가입하였습니다.");
		else
			INSTANCE(GameGUIFacade)->LogFloat->AddText(L"ID " + name + L" 님의 파티 가입 신청을 거절하였습니다.");
	}

	return true;
}

const bool Handle_s2c_PARTY_JOIN_NEW_PLAYER(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_PARTY_JOIN_NEW_PLAYER& pkt_)
{
	std::wstring name = std::to_wstring(pkt_.target_user_id());
	INSTANCE(GameGUIFacade)->LogFloat->AddText(L"ID " + name + L" 님이 파티에 참여하였습니다.");
	INSTANCE(GameGUIFacade)->PartyStatus->AddPartyMember(pkt_.target_user_id());
	return true;
}

const bool Handle_s2c_PARTY_OUT(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_PARTY_OUT& pkt_)
{
	const bool is_my_id = pSession_->GetSessionID() == pkt_.out_user_id();
	std::wstring output_msg = {};
	auto partyStatusGUI = INSTANCE(GameGUIFacade)->PartyStatus;
	if (pkt_.is_leader())
	{
		if (is_my_id) // 파티장인데 나일경우
		{
			output_msg = L"파티를 해체하였습니다.";
		}
		else // 파티장이 쫑낸경우
		{
			output_msg = L"파티장이 파티를 탈퇴하여 파티가 해체되었습니다.";
		}
		partyStatusGUI->DisablePartyPanel();
	}
	else if (is_my_id)
	{
		// 내가 파티장이 아니고 걍 자발적으로 나온경우
		output_msg = L"파티를 탈퇴하였습니다.";
		partyStatusGUI->DisablePartyPanel();
	}
	else
	{
		output_msg = L"ID " + std::to_wstring(pkt_.out_user_id()) + L" 님이 파티를 탈퇴하였습니다.";
		partyStatusGUI->RemovePartyMember(pkt_.out_user_id());
	}
	if(!output_msg.empty())
		INSTANCE(GameGUIFacade)->LogFloat->AddText(output_msg);
	return true;
}

const bool Handle_s2c_PARTY_QUEST_CLEAR(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_PARTY_QUEST_CLEAR& pkt_)
{
	std::cout << "퀘스트 ID: " << pkt_.party_quest_id() << "클리어 ! 나가려면 N키를 눌러주세요\n";
	return true;
}

const bool Handle_s2c_PARTY_MEMBERS_INFORMATION(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_PARTY_MEMBERS_INFORMATION& pkt_)
{
	for (const auto other_members : *pkt_.party_member_ids())
	{
		std::cout << "파티원들 ID: " << other_members << '\n';
	}

	std::vector<uint32_t> partyMemberIDs(pkt_.party_member_ids()->cbegin(), pkt_.party_member_ids()->cend());
	partyMemberIDs.emplace_back(pSession_->GetSessionID());
	INSTANCE(GameGUIFacade)->PartyStatus->InitializeContents(partyMemberIDs);
	return true;
}

const bool Handle_s2c_GET_HARVEST(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_GET_HARVEST& pkt_)
{
	std::cout << "채집물 획득 ! ID: " << pkt_.harvest_id() << std::endl;
	GuideSystem::GetInst()->RemoveHarvest(pkt_.harvest_id());
	return true;
}
