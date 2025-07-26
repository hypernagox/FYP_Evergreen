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
#include "QuestGUI.h"
#include "LogFloatGUI.h"
#include "RequestPopupGUI.h"
#include "PartyStatusGUI.h"
#include "GuideSystem.h"
#include "DamageCountGUI.h"
#include "MovePacketSender.h"
#include "CommonQuestTable.h"
#include "NavigationMesh.h"
#include "Navigator.h"
#include "Naviagent.h"
#include "GameScene.h"
#include "TransitionOverlayGUI.h"
#include "ForcedMovement.h"
#include "MainScene.h"
#include "MonsterBoss.h"
#include "PlayerTagGUI.h"
#include "TutorialUI.h"

thread_local flatbuffers::FlatBufferBuilder buillder{ 256 };

flatbuffers::FlatBufferBuilder* const CreateBuilder()noexcept {
	extern thread_local flatbuffers::FlatBufferBuilder buillder;
	return &buillder;
}

static inline std::string GetOriginString(const auto pkt_string)noexcept {
	std::string str;
	str.reserve(pkt_string->size());
	for (const auto ch : *pkt_string)str.push_back(ch);
	return str;
}

static inline std::wstring GetOriginWString(const auto pkt_string)noexcept {
	return Common::DataRegistry::Str2Wstr(GetOriginString(pkt_string));
}

#define Mgr(type)	(type::GetInst())

const bool Handle_s2c_LOGIN(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_LOGIN& pkt_)
{
	// 히어로의 초기화시점이 생각과 달라서 서버오브젝트 매니저에 값을 기록해뒀다가 씬 진입할 때 사용
	// 추가정보 더 올 예정 (직업 등..)

	const auto res = pkt_.login_result();
	const auto& item_ids = *pkt_.item_ids();
	const auto& item_counts = *pkt_.item_counts();
	
	const auto num = (int)item_ids.size();

	// ServerObjectMgr에서 로그인을 할 때마다 이전 init_item_counts와 init_item_ids를 초기화해야한다.
	// 그렇지 않을경우 이전의 로그인할 때 플레이어게 넣어줘야할 아이템 정보가 누적되어 n배씩(n = MainScene에서 GameScene으로 넘어간 횟수) 아이템 개수가 많아진다.
	Mgr(ServerObjectMgr)->PrepareLoginData(pkt_);

	// TODO: 플레이어가 이미 등록된 경우 PrepareLoginData()와 더불어 캐릭터 타입(직업)을 해당 인자로 넘겨준다. (AuthenticPlayer는 생성과 동시에 초기화되어야함)
	unsigned int characterType = 0;
	std::string class_type;
	for (const auto ch : *pkt_.class_type())class_type.push_back(ch);
	
	if ("Warrior" == class_type)
	{
		characterType = 0;
	}
	else if ("Priest" == class_type)
	{
		characterType = 1;
	}
	else if ("Archer" == class_type)
	{
		// TODO: 신캐릭추가하면 바뀌어야함
		characterType = 2;
	}
	else
	{
		std::cout << "Invalid Class Type\n";
	}
	Mgr(ServerObjectMgr)->GetTargetMainScene()->OnLoginResult(res, characterType);

	std::cout << "도착\n";
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
		//std::cout << "NPC 등장\n";
	}
	else if (pkt_.group_type() == Nagox::Enum::GROUP_TYPE_DROP_ITEM)
	{
		static std::unique_ptr<SoundEffectInstance> g_appearSound;
		g_appearSound = INSTANCE(Resource)->Load<AudioClip>(RESOURCE_PATH(L"audio\\item_drop.wav"))->CreateInstance();
		auto distance = Vector3::Distance(Mgr(ServerObjectMgr)->GetMainHero()->GetTransform()->GetLocalPosition(), ::ToOriginVec3(pkt_.appear_pos()));
		g_appearSound->SetVolume(1.0f / (distance * 0.1f + 2.0f));
		g_appearSound->Play();
	}
	else if (pkt_.group_type() == Nagox::Enum::GROUP_TYPE_HARVEST)
	{
		//std::cout << "채집 ID: " << pkt_.obj_cur_hp() << '\n';

		// 함수 정의 참조.
		// 간발의 차이로 어피어 오브젝트보다 채집물 상태변경 패킷이 먼저 와버린 경우에 대한 대처
		const bool is_active = GuideSystem::GetInst()->AddHarvest(obj_id, obj_cur_hp, HARVEST_STATE::AVAILABLE == static_cast<HARVEST_STATE>(pkt_.obj_type_info()));
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
	const auto obj_id = pkt_.obj_id();
	
	if (GuideSystem::GetInst()->IsHarvest(obj_id))
	{
		uint32_t harvestID = GuideSystem::GetInst()->GetHarvestID(obj_id);
		if (harvestID >= 0) {
			GuideSystem::GetInst()->SetHarvestState(obj_id, harvestID, false);
		}
		return true;
	}
	Mgr(ServerObjectMgr)->RemoveObject(obj_id);
	return true;
}

const bool Handle_s2c_MOVE(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_MOVE& pkt_)
{
	if (pSession_->GetSessionID() == pkt_.obj_id()) 
	{
		//constinit static uint64_t e_cnt = 0;
		//++e_cnt;
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
		const int before_hp = monster->GetHP();
		monster->OnHit(hit_after_hp);
		const auto hit_count = pkt_.hit_count();
		auto damageCount = INSTANCE(GameGUIFacade)->DamageCount;
		// TODO: 스킬의 연타 횟수가 패킷으로 오거나 json으로 몇 연타인지 미리 정의
		damageCount->AddCountObject(hit_obj_ptr->GetTransform()->GetLocalPosition(), static_cast<unsigned int>(before_hp) - hit_after_hp, hit_count);
	}
	if (const auto player = hit_obj_ptr->GetComponent<AuthenticPlayer>())
	{
		player->OnHit(pkt_.hit_after_hp());
	}
	else if (const auto player = hit_obj_ptr->GetComponent<PlayerRenderer>())
	{
		player->GetComponent<PlayerRenderer>()->Hit();
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

	// TODO: 좌클스킬 우클스킬 딱 두개만 있을듯 패킷에 기본스킬인지 우클스킬인지 올 것
	// 좌클스킬인지 우클스킬인지에 따라 다른 애니메이션 또는 이펙트

	const auto skill_type = pkt_.atk_type();
	const auto atk_player = Mgr(ServerObjectMgr)->GetServerObj(pkt_.atk_player_id());
	if (!atk_player || NetMgr(NetworkMgr)->GetSessionID() == pkt_.atk_player_id())
		return true;
	
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
		//heroObject->GetTransform()->SetLocalPosition(::ToOriginVec3(pkt_.rebirth_pos()));
		heroObject->GetComponent<PlayerRenderer>()->Death();
		NetMgr(NetworkMgr)->Send(Create_c2s_PLAYER_DEATH());
	}
	else
	{
		if (const auto obj = ServerObjectMgr::GetInst()->GetServerObj(pkt_.player_id()))
		{
			obj->GetComponent<PlayerRenderer>()->Death();
			//obj->GetTransform()->SetLocalPosition(::ToOriginVec3(pkt_.rebirth_pos()));
		}
	}
	return true;
}

const bool Handle_s2c_REQUEST_QUEST(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_REQUEST_QUEST& pkt_)
{
	if (!pkt_.is_accept())
	{
		std::cout << "이미 받은 퀘스트 입니다.\n";
		return true;
	}
	std::cout << "퀘스트 ID: " << pkt_.quest_id() << " 수락 !\n";
	// TODO: 퀘스트 정보
	// 몹 종류 마리수 다있음
	const auto quest_info = Common::CommonQuestTable::GetCommonQuestInfo(pkt_.quest_id());
	std::wcout << L"퀘스트 이름: " << quest_info.quest_name << std::endl;
	for (const auto& [mon, num] : quest_info.monsters_info)
	{
		std::wcout << mon << L" " << num << L" 마리\n";
	}
	return true;
}

const bool Handle_s2c_PROCESS_COMMON_QUEST(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_PROCESS_COMMON_QUEST& pkt_)
{
	// TODO: 퀘스트 id 와 잡은 몹 종류
	const auto quest_info = Common::CommonQuestTable::GetCommonQuestInfo(pkt_.quest_id());
	const auto mon_type = pkt_.mon_type();
	return true;
}

const bool Handle_s2c_CLEAR_QUEST(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_CLEAR_QUEST& pkt_)
{
	if (pkt_.is_clear())
	{
		std::cout << "퀘스트 클리어 !\n";
		// TODO: 보상정보도 여기에있음 골드 템 다 포함
		const auto quest_info = Common::CommonQuestTable::GetCommonQuestInfo(pkt_.quest_id());
		
		for (const auto& [item_name, item_id, num] : quest_info.reward_info)
		{
			if (const auto playerComp = Mgr(ServerObjectMgr)->GetMainHero()->GetComponent<AuthenticPlayer>())
			{
				playerComp->OnModifyInventory((uint8_t)item_id, num);
			}
		}

	}
	else
	{
		// false 일 경우 포기 실패 등등.. 
	}
	
	return true;
}

const bool Handle_s2c_FIRE_PROJ(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_FIRE_PROJ& pkt_)
{
	// TODO: 개 쌉 하드코딩 + 매넘
	// TODO: 이 패킷은 FIRE_NON_TARGET으로 바뀔듯
	// 0번이 투사체
	// 1번이 지금 보스 범위공격으로 생각중
	const auto proj_rad = pkt_.radius();
	if (pkt_.proj_type() == 0)
	{
		const auto shoot_obj_id = pkt_.shoot_obj_id();
		const auto proj_type = pkt_.proj_type(); // TODO: 투사체의 타입 (아직 없음)
		auto s = SceneObject::MakeShared();
		s->GetTransform()->SetLocalPosition(::ToOriginVec3(pkt_.pos()));
		s->GetTransform()->SetLocalScale(0.25f);

		auto gizmoRenderer = s->AddComponent<MeshRenderer>();
		gizmoRenderer->SetMesh(INSTANCE(Resource)->Load<Mesh>(RESOURCE_PATH(L"sphere.yms")));
		gizmoRenderer->SetMaterial(INSTANCE(Resource)->Load<Shader>(RESOURCE_PATH(L"colornotex.hlsl")));

		auto so = s->AddComponent<ServerObject>();
		const auto proj = so->AddComp<Projectile>();
		proj->m_speed = ::ToOriginVec3(pkt_.vel());
		so->SetObjID((uint32_t)pkt_.proj_id());
		Mgr(ServerObjectMgr)->AddObject(s);
	}
	else if (pkt_.proj_type() == 1)
	{
		const auto shoot_obj_id = pkt_.shoot_obj_id();
		const auto proj_type = pkt_.proj_type(); // TODO: 투사체의 타입 (아직 없음)
		auto s = SceneObject::MakeShared();
		s->GetTransform()->SetLocalPosition(::ToOriginVec3(pkt_.pos()));
		s->GetTransform()->SetLocalScale(proj_rad/2.f);
	
		auto gizmoRenderer = s->AddComponent<MeshRenderer>();
		gizmoRenderer->SetMesh(INSTANCE(Resource)->Load<Mesh>(RESOURCE_PATH(L"sphere.yms")));
		gizmoRenderer->SetMaterial(INSTANCE(Resource)->Load<Shader>(RESOURCE_PATH(L"colornotex.hlsl")));
	
		auto so = s->AddComponent<ServerObject>();
		const auto proj = so->AddComp<Projectile>();
		proj->m_speed = ::ToOriginVec3(pkt_.vel());
		so->SetObjID((uint32_t)pkt_.proj_id());
		Mgr(ServerObjectMgr)->AddObject(s);
	}

	return true;
}

const bool Handle_s2c_ACQUIRE_ITEM(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_ACQUIRE_ITEM& pkt_)
{
	// TODO: 만약 내가 아닌 다른 플레이어가 아이템 먹은걸 알아야 한다면 (예: XX님이 YY를 획득!) 누가 먹었는지 ID도 필요
	// 기획의 영역..
	
	// ID는 정수이나, json 테이블에서 해당 ID에 대응하는 아이템 정보를 획득하기 위해선 문자열로의 변환이 필요하다.
	Mgr(ServerObjectMgr)->RemoveObject(pkt_.item_obj_id());
	//std::cout << std::format("아이템 획득함! 아이템 ID: {} 먹은 User ID: {} , 개수: {}\n", pkt_.item_detail_id(), pkt_.get_user_id(), pkt_.item_stack_size());

	if (auto targetObject = Mgr(ServerObjectMgr)->GetServerObj(pkt_.get_user_id()))
	{
		const auto item_id = pkt_.item_detail_id();
		const auto item_count = pkt_.item_stack_size();
		// 획득한 사람이 현재 클라이언트일 경우
		if (auto playerComp = targetObject->GetComponent<AuthenticPlayer>())
		{
			static std::unique_ptr<SoundEffectInstance> g_menuSound;
			g_menuSound = INSTANCE(Resource)->Load<AudioClip>(RESOURCE_PATH(L"audio\\pickup.wav"))->CreateInstance();
			g_menuSound->SetVolume(0.5f);
			g_menuSound->Play();

			playerComp->OnModifyInventory(item_id, item_count);
			INSTANCE(GameGUIFacade)->LogFloat->AddText(GET_DATA(std::wstring,"Item", DATA_TABLE->GetItemName(item_id), "Name") + L"을(를) " + std::to_wstring(item_count) + L"개 획득하였습니다.");
		}
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
	extern bool g_tutorial_craft_clear;
	g_tutorial_craft_clear = true;
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
	const auto party_quest_id = pkt_.quest_id();
	std::cout << "현재 파티퀘스트 ID: " << party_quest_id << std::endl;

	std::vector<uint32_t> partyMemberIDs;
	partyMemberIDs.emplace_back(pSession_->GetSessionID());

	std::vector<std::wstring> partyMemberNames;
	partyMemberNames.emplace_back(ServerObjectMgr::GetInst()->GetUsername());

	INSTANCE(GameGUIFacade)->PartyStatus->InitializeContents(partyMemberIDs, partyMemberNames);

	// TODO: 튜토리얼 호위퀘스트의 경우 상호작용으로 다른방식으로 파티원이 혼자인 파티퀘스트를 REGISTER하고 그의 답변으로
	// 응답이 왔을 때  내가 0번퀘스트 (튜토리얼 호위퀘스트)를 요청했었다면 그대로 퀘스트 실행 후 파티 인터페이스는 감추기
	if (0 == party_quest_id)
	{
		TutorialUI::StartTutorialGUI();
		INSTANCE(GameGUIFacade)->PartyStatus->RequestQuestStart();
	}
	return true;
}

const bool Handle_s2c_ACQUIRE_PARTY_LIST(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_ACQUIRE_PARTY_LIST& pkt_)
{
	std::cout << "파티퀘스트 ID: " << pkt_.target_quest_id() << " 의 목록들\n";
	for (const auto v : *pkt_.party_leader_ids())
	{
		std::cout << "파티장 ID: " << v << std::endl;
	}

	auto partyListGUI = INSTANCE(GameGUIFacade)->QuestGUI;
	std::vector<uint32_t> partyList(pkt_.party_leader_ids()->cbegin(), pkt_.party_leader_ids()->cend());
	partyListGUI->FetchPartyList(partyList);

	return true;
}

const bool Handle_s2c_INVITE_PARTY_QUEST(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_INVITE_PARTY_QUEST& pkt_)
{
	// 다른 사람으로부터 온 파티 초대
	// 여기서 다른 파티장으로 부터 파티 초대가 온다.

	// 팝업 메시지 송출
	auto party_leader_name = GetOriginWString(pkt_.target_party_leader_name());
	INSTANCE(GameGUIFacade)->RequestPopup->ShowPopup(
		L"파티 초대 알림",
		L"ID " + party_leader_name + L" 님이 파티에 초대하였습니다.",
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
	
	//if (pSession_->GetSessionID() == pkt_.target_party_leader_id())
	//{
	//	if (pkt_.invite_result())
	//	{
	//		std::cout << "아이디 " << pkt_.target_user_id() << "가 수락" << std::endl;
	//	}
	//	else
	//	{
	//		std::cout << "아이디 " << pkt_.target_user_id() << "가 거절" << std::endl;
	//	}
	//}
	//else
	//{
	//	if(pkt_.invite_result())
	//		std::cout << "파티장 ID " << pkt_.target_party_leader_id() << " 인 파티 신청을 내가 받아줌\n";
	//	else
	//		std::cout << "파티장 ID " << pkt_.target_party_leader_id() << " 인 파티 신청 안받아줌 ㅅㄱ\n";
	//}

	const auto target_name = GetOriginWString(pkt_.target_user_name());
	
	// 파티장일 경우
	if (pSession_->GetSessionID() == pkt_.target_party_leader_id())
	{
		if (pkt_.invite_result())
			INSTANCE(GameGUIFacade)->LogFloat->AddText(target_name + L" 님이 파티에 가입하였습니다.");
		else
			INSTANCE(GameGUIFacade)->LogFloat->AddText(target_name + L" 님이 파티에 가입을 거절하였습니다.");
	}
	// 초대를 받은 유저일 경우
	else
	{
		if (pkt_.invite_result())
			INSTANCE(GameGUIFacade)->LogFloat->AddText(L"파티(파티장 " + target_name + L")에 가입하였습니다.");
		else
			INSTANCE(GameGUIFacade)->LogFloat->AddText(L"파티(파티장 " + target_name + L")의 가입 초대를 거절하였습니다.");
	}

	return true;
}

const bool Handle_s2c_PARTY_JOIN_REQUEST(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_PARTY_JOIN_REQUEST& pkt_)
{
	// 내가 파티장 일 때 다른 사람의 파티요청이 여기로 온다

	// 팝업 메시지 송출
	auto target_user_name = GetOriginWString(pkt_.target_user_name());
	INSTANCE(GameGUIFacade)->RequestPopup->ShowPopup(
		L"가입 요청 알림",
		L"ID " + target_user_name + L" 님이 파티에 가입 요청을 하였습니다.",
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
	std::wstring name = GetOriginWString(pkt_.target_user_name());
	INSTANCE(GameGUIFacade)->LogFloat->AddText(L"ID " + name + L" 님이 파티에 참여하였습니다.");
	INSTANCE(GameGUIFacade)->PartyStatus->AddPartyMember(pkt_.target_user_id(), name);
	return true;
}

const bool Handle_s2c_QUEST_END(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_QUEST_END& pkt_)
{
	ServerObjectMgr::GetInst()->GetTargetScene()->ChangeGameScene(GameScene::GameSceneType::Default);
	ServerObjectMgr::GetInst()->GetMainHero()->SetNavigationMesh(NAVI_MESH_TYPE::MAIN_WORLD);
	ServerObjectMgr::GetInst()->GetMainHero()->GetComp<MovePacketSender>()->SetSendInterval(0.1f);
	INSTANCE(GameGUIFacade)->TransitionOverlay->AppendTransition([]() {}, L"퀘스트를 종료하는 중 ...");
	GuideSystem::GetInst()->DisableClearTree();

	// TODO: 0번 퀘스트가 클리어되었다면 (튜토리얼 호위퀘스트였다면 파티창 꺼주기
	Send(Create_c2s_PARTY_OUT());
	
	extern bool g_tutorial_end_clear;
	g_tutorial_end_clear = true;

	return true;
}

const bool Handle_s2c_PARTY_QUEST_START(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_PARTY_QUEST_START& pkt_)
{
	ServerObjectMgr::GetInst()->GetMainHero()->GetComp<MovePacketSender>()->SetSendInterval(0.1f);
	INSTANCE(GameGUIFacade)->TransitionOverlay->AppendTransition([]() {}, L"퀘스트를 시작하는 중 ...");
	return true;
}

const bool Handle_s2c_PARTY_OUT(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_PARTY_OUT& pkt_)
{
	const bool is_my_id = pSession_->GetSessionID() == pkt_.out_user_id();
	std::wstring output_msg = {};
	auto partyStatusGUI = INSTANCE(GameGUIFacade)->PartyStatus;
	const auto cur_leader_id = pkt_.cur_leader_id();
	// TODO: 현재 결과적으로 파티장이 누구인지가 온다.
	// 0이라면 파티에 멤버가 더 이상 남아있지 않은 경우

	if (is_my_id)
	{
		// 내가 파티장이 아니고 걍 자발적으로 나온경우
		output_msg = L"파티를 탈퇴하였습니다.";
		partyStatusGUI->DisablePartyPanel();
	}
	else
	{
		// 나 자신을 제외한 어떤 파티 멤버가 탈퇴한 경우 (탈퇴 멤버가 파티장일 수도 있음)
		auto name = GetOriginWString(pkt_.out_user_name());
		output_msg = L"ID " + name + L" 님이 파티를 탈퇴하였습니다.";
		partyStatusGUI->RemovePartyMember(pkt_.out_user_id());
		partyStatusGUI->SetPartyLeader(pkt_.cur_leader_id());
	}
	if(!output_msg.empty())
		INSTANCE(GameGUIFacade)->LogFloat->AddText(output_msg);
	return true;
}

const bool Handle_s2c_PARTY_QUEST_CLEAR(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_PARTY_QUEST_CLEAR& pkt_)
{
	extern bool g_tutorial_clear;
	if (0 == pkt_.party_quest_id())
	{
		g_tutorial_clear = true;
	}
	// TODO: 여기서 깨진 퀘스트 ID로 보스방인지 아닌지 구분, ID로 어떤 파퀘가 깨졌는지 여기서 체크함
	// if 지금 깬 파티퀘스트가 보스방퀘면? 나가면 네비메시를 바꾼다
	//ServerObjectMgr::GetInst()->GetMainHero()->SetNavigationMesh(NAVI_MESH_TYPE::MAIN_WORLD);
	
	// 퀘스트를 깨면 무조건 메인 월드로 나가게 되므로 항상 마을맵 이동 로직을 수행한다.
	// 이미 마을이더라 하더라도 동작에는 문제가 없으나 추후 더 유연한 맵 이동 로직 요

	static std::unique_ptr<SoundEffectInstance> g_menuSound;
	g_menuSound = INSTANCE(Resource)->Load<AudioClip>(RESOURCE_PATH(L"audio\\quest_clear.wav"))->CreateInstance();
	g_menuSound->SetVolume(0.5f);
	g_menuSound->Play();

	INSTANCE(GameGUIFacade)->PartyStatus->OnQuestClear();
	INSTANCE(GameGUIFacade)->LogFloat->AddText(L"퀘스트 클리어 ! 보상 나무를 따라가 보상을 확인하세요.");
	GuideSystem::GetInst()->ToggleFlag();
	GuideSystem::GetInst()->AppearClearTree(ToOriginVec3(pkt_.clear_tree_pos()));
	//GuideSystem::GetInst()->AppearClearTree(Vector3(-20.861689F, 72.62489F, 46.038242F)
	//);
	//GuideSystem::GetInst()->SetGuidePath(Vector3(-44.4872F, 74.50986F, -59.177734F));
	ServerObjectMgr::GetInst()->GetMainHero()->GetComp<MovePacketSender>()->SetSendInterval(0.1f);
	return true;
}

const bool Handle_s2c_PARTY_MEMBERS_INFORMATION(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_PARTY_MEMBERS_INFORMATION& pkt_)
{
	// TODO: 다른 파티원들의 이름이 온다.
	for (const auto other_members : *pkt_.party_member_ids())
	{
		std::cout << "파티원들 ID: " << other_members << '\n';
	}
	std::vector<std::string> names;
	for (const auto other_member_name : *pkt_.party_member_names())
	{
		auto name = GetOriginWString(other_member_name);
		std::wcout << L"파티원들 Name: " << name << L'\n';
	}
	std::vector<uint32_t> partyMemberIDs(pkt_.party_member_ids()->cbegin(), pkt_.party_member_ids()->cend());
	partyMemberIDs.emplace_back(pSession_->GetSessionID());

	std::vector<std::wstring> partyMemberNames;
	for (const auto& other_member_name : *pkt_.party_member_names())
	{
		auto name = GetOriginWString(other_member_name);
		partyMemberNames.emplace_back(name);
	}
	partyMemberNames.emplace_back(ServerObjectMgr::GetInst()->GetUsername());

	auto partyStatus = INSTANCE(GameGUIFacade)->PartyStatus;
	partyStatus->InitializeContents(partyMemberIDs, partyMemberNames);

	return true;
}

const bool Handle_s2c_CHANGE_HARVEST_STATE(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_CHANGE_HARVEST_STATE& pkt_)
{
	// TODO: 여기서 대상이 될 채집물을 찾아서 주면 서버오버헤드는 감소
	//std::cout << "채집 ID: " << pkt_.harvest_mesh_type() << '\n';
	const auto harvest_id = pkt_.harvest_id();
	if (0 == harvest_id)
	{
		GuideSystem::GetInst()->ToggleFlag();
		GuideSystem::GetInst()->SetClearTreeInteraction(false);
	}
	else
	{
		GuideSystem::GetInst()->SetHarvestState(harvest_id, pkt_.harvest_mesh_type(), pkt_.is_active());
	}
	
	return true;
}

const bool Handle_s2c_NOTIFY_USER_DETAIL_INFO(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_NOTIFY_USER_DETAIL_INFO& pkt_)
{
	std::string user_name;
	for (const auto ch : *pkt_.user_name())user_name.push_back(ch);
	std::cout << "User Name: " << user_name << std::endl;
	if (const auto obj = ServerObjectMgr::GetInst()->GetServerObj(pkt_.obj_id()))
	{
		if (const auto renderer = obj->GetComponent<PlayerRenderer>())
		{
			// TODO: 플레이어 이름 변경을 위한 네임태그 컴포넌트 수집
			renderer->SetPlayerWeapon(pkt_.weapon_id());
			renderer->SetPlayerArmor(pkt_.armor_id());
		}
		if (const auto playerTag = obj->GetComponent<PlayerTagGUI>())
		{
			playerTag->SetText(Common::DataRegistry::Str2Wstr(user_name));
		}
	}
	else
	{
		ServerObjectMgr::GetInst()->m_equipmentMap[pkt_.obj_id()] = std::make_tuple(pkt_.weapon_id(), pkt_.armor_id(), user_name);
	}
	return true;
}

const bool Handle_s2c_FORCED_MOVE(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_FORCED_MOVE& pkt_)
{
	const auto target_id = pkt_.target_user_id();
	if (pSession_->GetSessionID() == target_id)
	{
		ServerObjectMgr::GetInst()->GetMainHero()->GetTransform()->SetLocalPosition(ToOriginVec3(pkt_.target_pos()));
		ServerObjectMgr::GetInst()->GetMainHero()->GetSceneObject()->GetComponent<AuthenticPlayer>()->FixCameraAnchor();
		return true;
	}
	if (const auto obj = ServerObjectMgr::GetInst()->GetServerObj(target_id))
	{
		if (const auto comp = obj->GetComp<MoveInterpolator>())
		{
			comp->UpdateForcedMoveData(ToOriginVec3(pkt_.target_pos()));
		}
	}
	return true;
}

const bool Handle_s2c_BOSS_ROOM_ENTER(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_BOSS_ROOM_ENTER& pkt_)
{
	// TODO: 보스방 입장 요청시 답변으로 오는 패킷
	// 여기서 네비메시 바꾸고 씬바꾸고 등등 해야함
	// 나가거나 클리어해서 다시 바꿀땐 파티퀘스트 클리어 패킷이오고 거기서 해결
	// 보스방 네비메시는
	// 
	// 
	// ServerObjectMgr::GetInst()->GetMainHero()->SetNavigationMesh(NAVI_MESH_TYPE::BOSS_ROOM);
	// 이렇게 바꾼다.

	ServerObjectMgr::GetInst()->GetTargetScene()->ChangeGameScene(GameScene::GameSceneType::Dungeon);
	ServerObjectMgr::GetInst()->GetMainHero()->GetSceneObject()->GetComponent<AuthenticPlayer>()->FixCameraAnchor();
	
	ServerObjectMgr::GetInst()->GetMainHero()->SetNavigationMesh(NAVI_MESH_TYPE::BOSS_ROOM);

	std::cout << "보스방 입장함\n";
	return true;
}

const bool Handle_s2c_BOSS_FLY(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_BOSS_FLY& pkt_)
{
	// TODO: 목적지와 어떤 비행?? 타입인지 온다.
	// 이 코드가 온순간 서버에서는 이미 해당위치로 순간이동 한 뒤이기 때문에 클라에서는 뭔가 공중으로 간다던지해서
	// 떄려도 공격 안맞을 것 같아야 함.
	const auto boss_fly_type = pkt_.boss_fly_type();
	const auto& boss_ptr = GET_BOSS;
	const auto target_pos = ToOriginVec3(pkt_.target_pos());
	const auto component = boss_ptr->GetComponent<MonsterBoss>();

	switch (boss_fly_type)
	{
	case Nagox::Enum::BOSS_FLY_TYPE_BOSS_FLY_TYPE_1:
		component->OnTakeoffAtPosition(target_pos);
		break;
	case Nagox::Enum::BOSS_FLY_TYPE_BOSS_FLY_TYPE_2:
		component->OnLandingAtPosition(target_pos);
		break;
	}

	// TODO: 이렇게 옮기는게 아니라 뭔가 애니메이션 후 옮기기
	// 서버도 이거 시간 맞춰서 타이머 돌려서 클라에서 이동 다 되었다고 판단 될때까지 다른 패킷은 보류 할 것
	//boss_ptr->GetComponent<ServerObject>()->GetComp<MoveInterpolator>()->UpdateForcedMoveData(target_pos);
	std::cout << std::format("x:{},y:{},z{}\n", target_pos.x, target_pos.y, target_pos.z);
	std::cout << "보스 비행 타입: " << boss_fly_type << std::endl;
	// boss_ptr->GetComponent<ServerObject>()->GetComp<MoveInterpolator>()->UpdateNewMoveDataOnlyPos(target_pos);
	return true;
}

const bool Handle_s2c_BOSS_MOVE(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_BOSS_MOVE& pkt_)
{
	// TODO: 보스 이동후 위치와 속도 그리고 어떤 이동타입인지 온다.
	// 움직이는 방향에 대해서 회전각은 클라에서 처리 필요
	
	const auto boss_move_type = pkt_.boss_move_type();
	const auto& boss_ptr = GET_BOSS;
	const auto target_pos = ToOriginVec3(pkt_.target_pos());
	// 보스의 회전각
	const auto boss_angle = pkt_.boss_angle();
	//std::cout << std::format("x:{},y:{},z{}\n", target_pos.x, target_pos.y, target_pos.z);
	boss_ptr->GetComponent<ServerObject>()->GetComp<MoveInterpolator>()->UpdateNewMoveDataOnlyPos(target_pos);
	
	return true;
}

const bool Handle_s2c_BOSS_PROJ_MARK(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_BOSS_PROJ_MARK& pkt_)
{
	// TODO: 기즈모가 안그려짐 보스 메테오 투사체의 지면위치가 여기에 옴(f12 눌러야하는듯)
	// 그려진다쳐도 구의 y스케일만 압축해서 원으로 그리려했는데 그 기능이 없음
	
	const auto proj_mark_pos = ToOriginVec3(pkt_.mark_pos());
	
	return true;
}

const bool Handle_s2c_HEAL(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_HEAL& pkt_)
{
	// TODO: 힐 받은 사람 주위에 이펙트나 숫자 또는 애니메이션
	const auto healed_user_id = pkt_.target_obj_id();
	const auto heal_val = pkt_.heal_val();
	const auto healed_obj_ptr = Mgr(ServerObjectMgr)->GetServerObj(healed_user_id);
	if (!healed_obj_ptr)
	{
		return true;
	}
	auto damageCount = INSTANCE(GameGUIFacade)->DamageCount;
	damageCount->AddCountObject(healed_obj_ptr->GetTransform()->GetLocalPosition(), -1);
	return true;
}

const bool Handle_s2c_DASH(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_DASH& pkt_)
{
	const auto dash_obj_ptr = Mgr(ServerObjectMgr)->GetServerObj(pkt_.dash_obj_id());
	if (!dash_obj_ptr)
	{
		return true;
	}
	if (const auto fm = dash_obj_ptr->GetComp<ForcedMovement>())
	{
		if (const auto mi = dash_obj_ptr->GetComp<MoveInterpolator>())
		{
			dash_obj_ptr->GetComp<MoveInterpolator>()->UpdateNewMoveDataOnlyPos(ToOriginVec3(pkt_.target_pos()));
			fm->SetForcedMovement(ToOriginVec3(pkt_.target_pos()));
		}
	}
	if (const auto renderer = dash_obj_ptr->GetComponent<PlayerRenderer>())
	{
		// 패킷을 받은 사람이 본인이 아닐 때
		if (dash_obj_ptr->GetComponent<AuthenticPlayer>() == nullptr)
		{
			renderer->Dash();
		}
	}
	return true;
}

const bool Handle_s2c_ARROW_RAIN(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_ARROW_RAIN& pkt_)
{
	constexpr const int arrowCount = 10;

	const Vector3 offsets[arrowCount] =
	{
		Vector3(-2.0f, 22.0f, -1.0f),
		Vector3(-1.0f, 18.5f, 1.0f),
		Vector3(0.0f, 25.0f, 0.0f),
		Vector3(1.2f, 21.0f, -0.8f),
		Vector3(-1.5f, 20.0f, 1.5f),
		Vector3(0.8f, 23.5f, 1.2f),
		Vector3(-2.3f, 17.5f, -1.4f),
		Vector3(0.5f, 24.0f, -1.8f),
		Vector3(-0.7f, 19.0f, 0.9f),
		Vector3(1.7f, 26.0f, 0.3f)
	};

	constexpr const float speeds[arrowCount] =
	{
		55.0f, 48.0f, 25.0f, 42.0f, 30.0f,
		52.0f, 28.0f, 46.0f, 35.0f, 50.0f
	};
	//std::cout << "패킷옴";
	const Vector3 center = ToOriginVec3(pkt_.atk_pos());

	for (int i = 0; i < arrowCount; ++i)
	{
		const Vector3 spawnPos = center + offsets[i] - Vector3{ 0,5, 0 };
		const Vector3 velocity = Vector3(0.0f, -1.0f, 0.0f) * speeds[i];

		
		
		auto s = SceneObject::MakeShared();
		s->GetTransform()->SetLocalPosition(spawnPos);
		s->GetTransform()->SetLocalScale(.5f);

		auto gizmoRenderer = s->AddComponent<MeshRenderer>();
		gizmoRenderer->SetMesh(INSTANCE(Resource)->Load<Mesh>(RESOURCE_PATH(L"sphere.yms")));
		gizmoRenderer->SetMaterial(INSTANCE(Resource)->Load<Shader>(RESOURCE_PATH(L"colornotex.hlsl")));
		s->SetActive(true);
		auto so = s->AddComponent<ServerObject>();
		const auto proj = so->AddComp<Projectile>();
		proj->m_speed = velocity;
		//so->SetObjID((uint32_t)pkt_.proj_id());
		Mgr(ServerObjectMgr)->AddObject(s);
	}

	return true;
}

const bool Handle_s2c_CHAT(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_CHAT& pkt_)
{
	// TODO: 채팅친 유저의 이름과 내용이 패킷으로 온다. 여기서 출력
	const auto chat_user_id = pkt_.chat_user_id();
	const auto chat_user_name = GetOriginWString(pkt_.char_user_name());
	const auto chat_msg = GetOriginWString(pkt_.char_msg());

	// ... 이후 여기서 출력하면 됨
	INSTANCE(GameGUIFacade)->LogFloat->AddText(L"[" + chat_user_name + L"]: " + chat_msg);

	// TODO: 채팅 메세지 보내는법
	// std::string my_chat_msg;
	// Send(Create_c2s_CHAT(my_chat_msg));

	return true;
}
