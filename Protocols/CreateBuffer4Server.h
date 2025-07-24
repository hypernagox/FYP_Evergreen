#pragma once
#include "../NagiocpX/NagiocpXPch.h"
#include "enum_generated.h"
#include "struct_generated.h"
#include "protocol_generated.h"

template<typename T>
using Vector = NagiocpX::XVector<T>;
using String = NagiocpX::XString;

extern flatbuffers::FlatBufferBuilder* const CreateBuilder() noexcept;

static inline flatbuffers::FlatBufferBuilder* const GetBuilder() noexcept { return CreateBuilder(); }

NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_LOGIN(
    const uint32_t obj_id,
    const uint64_t server_time_stamp,
    const Nagox::Enum::LOGIN_RESULT& login_result,
    const Vector<int> item_ids,
    const Vector<int> item_counts,
    const int32_t equip_weapon_id,
    const int32_t equip_armor_id,
    const std::string_view& class_type,
    flatbuffers::FlatBufferBuilder* const builder_ptr = GetBuilder()
)noexcept;
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_PING_PONG(
    const uint64_t server_time_stamp,
    flatbuffers::FlatBufferBuilder* const builder_ptr = GetBuilder()
)noexcept;
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_APPEAR_OBJECT(
    const uint32_t obj_id,
    const Nagox::Enum::GROUP_TYPE& group_type,
    const uint8_t obj_type_info,
    const Nagox::Struct::Vec3& appear_pos,
    const int32_t obj_max_hp,
    const int32_t obj_cur_hp,
    flatbuffers::FlatBufferBuilder* const builder_ptr = GetBuilder()
)noexcept;
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_REMOVE_OBJECT(
    const uint32_t obj_id,
    flatbuffers::FlatBufferBuilder* const builder_ptr = GetBuilder()
)noexcept;
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_MOVE(
    const uint64_t obj_id,
    const Nagox::Struct::Vec3& pos,
    const Nagox::Struct::Vec3& vel,
    const Nagox::Struct::Vec3& accel,
    const float body_angle,
    const uint64_t time_stamp,
    flatbuffers::FlatBufferBuilder* const builder_ptr = GetBuilder()
)noexcept;
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_MONSTER_ATTACK(
    const uint64_t obj_id,
    const uint64_t player_id,
    const uint32_t dmg,
    flatbuffers::FlatBufferBuilder* const builder_ptr = GetBuilder()
)noexcept;
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_NOTIFY_HIT_DMG(
    const uint64_t hit_obj_id,
    const int32_t hit_after_hp,
    const int32_t hit_count,
    flatbuffers::FlatBufferBuilder* const builder_ptr = GetBuilder()
)noexcept;
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_MONSTER_AGGRO_START(
    const Nagox::Enum::GROUP_TYPE& group_type,
    const uint8_t obj_type_info,
    flatbuffers::FlatBufferBuilder* const builder_ptr = GetBuilder()
)noexcept;
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_MONSTER_AGGRO_END(
    const Nagox::Enum::GROUP_TYPE& group_type,
    const uint8_t obj_type_info,
    flatbuffers::FlatBufferBuilder* const builder_ptr = GetBuilder()
)noexcept;
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_PLAYER_ATTACK(
    const uint64_t atk_player_id,
    const float body_angle,
    const Nagox::Struct::Vec3& atk_pos,
    const Nagox::Enum::SKILL_TYPE& atk_type,
    flatbuffers::FlatBufferBuilder* const builder_ptr = GetBuilder()
)noexcept;
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_PLAYER_DEATH(
    const uint64_t player_id,
    const Nagox::Struct::Vec3& rebirth_pos,
    flatbuffers::FlatBufferBuilder* const builder_ptr = GetBuilder()
)noexcept;
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_REQUEST_QUEST(
    const uint64_t quest_id,
    const bool is_accept,
    flatbuffers::FlatBufferBuilder* const builder_ptr = GetBuilder()
)noexcept;
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_PROCESS_COMMON_QUEST(
    const uint64_t quest_id,
    const Nagox::Enum::MONSTER_TYPE& mon_type,
    flatbuffers::FlatBufferBuilder* const builder_ptr = GetBuilder()
)noexcept;
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_CLEAR_QUEST(
    const uint64_t quest_id,
    const uint8_t is_clear,
    flatbuffers::FlatBufferBuilder* const builder_ptr = GetBuilder()
)noexcept;
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_FIRE_PROJ(
    const uint64_t shoot_obj_id,
    const uint64_t proj_id,
    const uint8_t proj_type,
    const Nagox::Struct::Vec3& pos,
    const Nagox::Struct::Vec3& vel,
    const float radius,
    flatbuffers::FlatBufferBuilder* const builder_ptr = GetBuilder()
)noexcept;
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_ACQUIRE_ITEM(
    const uint64_t get_user_id,
    const uint64_t item_obj_id,
    const uint8_t item_detail_id,
    const uint8_t item_stack_size,
    flatbuffers::FlatBufferBuilder* const builder_ptr = GetBuilder()
)noexcept;
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_USE_QUICK_SLOT_ITEM(
    const uint64_t use_user_id,
    const uint8_t item_id,
    const uint8_t quick_slot_idx,
    flatbuffers::FlatBufferBuilder* const builder_ptr = GetBuilder()
)noexcept;
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_CRAFT_ITEM(
    const uint8_t recipe_id,
    flatbuffers::FlatBufferBuilder* const builder_ptr = GetBuilder()
)noexcept;
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_REGISTER_PARTY_QUEST(
    const int32_t quest_id,
    flatbuffers::FlatBufferBuilder* const builder_ptr = GetBuilder()
)noexcept;
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_ACQUIRE_PARTY_LIST(
    const Vector<uint32_t> party_leader_ids,
    const int32_t target_quest_id,
    flatbuffers::FlatBufferBuilder* const builder_ptr = GetBuilder()
)noexcept;
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_INVITE_PARTY_QUEST(
    const uint32_t target_party_leader_id,
    const int32_t target_party_quest_id,
    const std::string_view& target_party_leader_name,
    flatbuffers::FlatBufferBuilder* const builder_ptr = GetBuilder()
)noexcept;
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_INVITE_PARTY_RESULT(
    const uint32_t target_party_leader_id,
    const uint32_t target_user_id,
    const bool invite_result,
    const std::string_view& target_user_name,
    flatbuffers::FlatBufferBuilder* const builder_ptr = GetBuilder()
)noexcept;
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_PARTY_JOIN_REQUEST(
    const uint32_t target_user_id,
    const std::string_view& target_user_name,
    flatbuffers::FlatBufferBuilder* const builder_ptr = GetBuilder()
)noexcept;
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_PARTY_JOIN_REQUEST_RESULT(
    const uint32_t target_user_id,
    const bool request_result,
    flatbuffers::FlatBufferBuilder* const builder_ptr = GetBuilder()
)noexcept;
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_PARTY_JOIN_NEW_PLAYER(
    const uint32_t target_user_id,
    const std::string_view& target_user_name,
    flatbuffers::FlatBufferBuilder* const builder_ptr = GetBuilder()
)noexcept;
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_QUEST_END(
    flatbuffers::FlatBufferBuilder* const builder_ptr = GetBuilder()
)noexcept;
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_PARTY_QUEST_START(
    flatbuffers::FlatBufferBuilder* const builder_ptr = GetBuilder()
)noexcept;
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_PARTY_OUT(
    const uint32_t out_user_id,
    const uint32_t cur_leader_id,
    const std::string_view& out_user_name,
    flatbuffers::FlatBufferBuilder* const builder_ptr = GetBuilder()
)noexcept;
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_PARTY_QUEST_CLEAR(
    const int32_t party_quest_id,
    const Nagox::Struct::Vec3& clear_tree_pos,
    flatbuffers::FlatBufferBuilder* const builder_ptr = GetBuilder()
)noexcept;
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_PARTY_MEMBERS_INFORMATION(
    const Vector<uint32_t> party_member_ids,
    const Vector<String> party_member_names,
    flatbuffers::FlatBufferBuilder* const builder_ptr = GetBuilder()
)noexcept;
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_CHANGE_HARVEST_STATE(
    const uint32_t harvest_id,
    const bool is_active,
    const uint16_t harvest_mesh_type,
    flatbuffers::FlatBufferBuilder* const builder_ptr = GetBuilder()
)noexcept;
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_NOTIFY_USER_DETAIL_INFO(
    const uint32_t obj_id,
    const std::string_view& user_name,
    const uint32_t weapon_id,
    const uint32_t armor_id,
    flatbuffers::FlatBufferBuilder* const builder_ptr = GetBuilder()
)noexcept;
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_FORCED_MOVE(
    const uint32_t target_user_id,
    const Nagox::Struct::Vec3& target_pos,
    flatbuffers::FlatBufferBuilder* const builder_ptr = GetBuilder()
)noexcept;
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_BOSS_ROOM_ENTER(
    flatbuffers::FlatBufferBuilder* const builder_ptr = GetBuilder()
)noexcept;
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_BOSS_FLY(
    const Nagox::Struct::Vec3& target_pos,
    const Nagox::Enum::BOSS_FLY_TYPE& boss_fly_type,
    flatbuffers::FlatBufferBuilder* const builder_ptr = GetBuilder()
)noexcept;
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_BOSS_MOVE(
    const Nagox::Struct::Vec3& target_pos,
    const float boss_speed,
    const Nagox::Enum::BOSS_MOVE_TYPE& boss_move_type,
    const float boss_angle,
    flatbuffers::FlatBufferBuilder* const builder_ptr = GetBuilder()
)noexcept;
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_BOSS_PROJ_MARK(
    const Nagox::Struct::Vec3& mark_pos,
    flatbuffers::FlatBufferBuilder* const builder_ptr = GetBuilder()
)noexcept;
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_HEAL(
    const uint32_t target_obj_id,
    const int32_t heal_val,
    flatbuffers::FlatBufferBuilder* const builder_ptr = GetBuilder()
)noexcept;
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_DASH(
    const uint32_t dash_obj_id,
    const Nagox::Struct::Vec3& target_pos,
    flatbuffers::FlatBufferBuilder* const builder_ptr = GetBuilder()
)noexcept;
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_ARROW_RAIN(
    const uint64_t atk_player_id,
    const float body_angle,
    const Nagox::Struct::Vec3& atk_pos,
    const Nagox::Enum::SKILL_TYPE& atk_type,
    flatbuffers::FlatBufferBuilder* const builder_ptr = GetBuilder()
)noexcept;
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_CHAT(
    const uint32_t chat_user_id,
    const std::string_view& char_user_name,
    const std::string_view& char_msg,
    flatbuffers::FlatBufferBuilder* const builder_ptr = GetBuilder()
)noexcept;
