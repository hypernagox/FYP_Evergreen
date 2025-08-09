#include "pch.h"
#include <flatbuffers/flatbuffers.h>
#include "../NagiocpX/NagiocpXPch.h"
#include "enum_generated.h"
#include "struct_generated.h"
#include "protocol_generated.h"
#include "c2s_PacketHandler.h"

static NagiocpX::S_ptr<NagiocpX::SendBuffer> CreateSendBuffer(flatbuffers::FlatBufferBuilder& builder, const CREATE_PKT_ID pktId) noexcept
{
    const uint16_t dataSize = builder.GetSize();
    const uint16_t packetSize = dataSize + static_cast<c_uint16>(sizeof(NagiocpX::PacketHeader));
    NagiocpX::S_ptr<NagiocpX::SendBuffer> sendBuffer = NagiocpX::SendBufferMgr::Open(packetSize);
    NagiocpX::PacketHeader* const __restrict header =
        reinterpret_cast<NagiocpX::PacketHeader* const>(
            ::memcpy(sendBuffer->Buffer() + sizeof(NagiocpX::PacketHeader), builder.GetBufferPointer(), dataSize)
            ) - 1;
    header->pkt_size = packetSize;
    header->pkt_id = static_cast<c_uint16>(pktId);
    sendBuffer->Close(packetSize);
    return sendBuffer;
}

NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_LOGIN(
    const uint32_t obj_id,
    const uint64_t server_time_stamp,
    const Nagox::Enum::LOGIN_RESULT& login_result,
    const Vector<int> item_ids,
    const Vector<int> item_counts,
    const int32_t equip_weapon_id,
    const int32_t equip_armor_id,
    const std::string_view& class_type,
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    builder.Clear();
    const auto obj_id_value = obj_id;
    const auto server_time_stamp_value = server_time_stamp;
    const auto login_result_value = login_result;
    const auto item_ids_offset = builder.CreateVector(item_ids);
    const auto item_counts_offset = builder.CreateVector(item_counts);
    const auto equip_weapon_id_value = equip_weapon_id;
    const auto equip_armor_id_value = equip_armor_id;
    const auto class_type_offset = builder.CreateString(class_type);
   const auto serializeds2c_LOGIN = Nagox::Protocol::Creates2c_LOGIN(
    builder,    obj_id_value
,     server_time_stamp_value
,     login_result_value
,     item_ids_offset
,     item_counts_offset
,     equip_weapon_id_value
,     equip_armor_id_value
,     class_type_offset
    );
    builder.Finish(serializeds2c_LOGIN);

    return CreateSendBuffer(builder, CREATE_PKT_ID::s2c_LOGIN);
}
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_PING_PONG(
    const uint64_t server_time_stamp,
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    builder.Clear();
    const auto server_time_stamp_value = server_time_stamp;
   const auto serializeds2c_PING_PONG = Nagox::Protocol::Creates2c_PING_PONG(
    builder,    server_time_stamp_value
    );
    builder.Finish(serializeds2c_PING_PONG);

    return CreateSendBuffer(builder, CREATE_PKT_ID::s2c_PING_PONG);
}
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_APPEAR_OBJECT(
    const uint32_t obj_id,
    const Nagox::Enum::GROUP_TYPE& group_type,
    const uint8_t obj_type_info,
    const Nagox::Struct::Vec3& appear_pos,
    const int32_t obj_max_hp,
    const int32_t obj_cur_hp,
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    builder.Clear();
    const auto obj_id_value = obj_id;
    const auto group_type_value = group_type;
    const auto obj_type_info_value = obj_type_info;
    const auto appear_pos_offset = &appear_pos;
    const auto obj_max_hp_value = obj_max_hp;
    const auto obj_cur_hp_value = obj_cur_hp;
   const auto serializeds2c_APPEAR_OBJECT = Nagox::Protocol::Creates2c_APPEAR_OBJECT(
    builder,    obj_id_value
,     group_type_value
,     obj_type_info_value
,     appear_pos_offset
,     obj_max_hp_value
,     obj_cur_hp_value
    );
    builder.Finish(serializeds2c_APPEAR_OBJECT);

    return CreateSendBuffer(builder, CREATE_PKT_ID::s2c_APPEAR_OBJECT);
}
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_REMOVE_OBJECT(
    const uint32_t obj_id,
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    builder.Clear();
    const auto obj_id_value = obj_id;
   const auto serializeds2c_REMOVE_OBJECT = Nagox::Protocol::Creates2c_REMOVE_OBJECT(
    builder,    obj_id_value
    );
    builder.Finish(serializeds2c_REMOVE_OBJECT);

    return CreateSendBuffer(builder, CREATE_PKT_ID::s2c_REMOVE_OBJECT);
}
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_MOVE(
    const uint64_t obj_id,
    const Nagox::Struct::Vec3& pos,
    const Nagox::Struct::Vec3& vel,
    const Nagox::Struct::Vec3& accel,
    const float body_angle,
    const uint64_t time_stamp,
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    builder.Clear();
    const auto obj_id_value = obj_id;
    const auto pos_offset = &pos;
    const auto vel_offset = &vel;
    const auto accel_offset = &accel;
    const auto body_angle_value = body_angle;
    const auto time_stamp_value = time_stamp;
   const auto serializeds2c_MOVE = Nagox::Protocol::Creates2c_MOVE(
    builder,    obj_id_value
,     pos_offset
,     vel_offset
,     accel_offset
,     body_angle_value
,     time_stamp_value
    );
    builder.Finish(serializeds2c_MOVE);

    return CreateSendBuffer(builder, CREATE_PKT_ID::s2c_MOVE);
}
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_MONSTER_ATTACK(
    const uint64_t obj_id,
    const uint64_t player_id,
    const uint32_t dmg,
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    builder.Clear();
    const auto obj_id_value = obj_id;
    const auto player_id_value = player_id;
    const auto dmg_value = dmg;
   const auto serializeds2c_MONSTER_ATTACK = Nagox::Protocol::Creates2c_MONSTER_ATTACK(
    builder,    obj_id_value
,     player_id_value
,     dmg_value
    );
    builder.Finish(serializeds2c_MONSTER_ATTACK);

    return CreateSendBuffer(builder, CREATE_PKT_ID::s2c_MONSTER_ATTACK);
}
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_NOTIFY_HIT_DMG(
    const uint64_t hit_obj_id,
    const int32_t hit_after_hp,
    const int32_t hit_count,
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    builder.Clear();
    const auto hit_obj_id_value = hit_obj_id;
    const auto hit_after_hp_value = hit_after_hp;
    const auto hit_count_value = hit_count;
   const auto serializeds2c_NOTIFY_HIT_DMG = Nagox::Protocol::Creates2c_NOTIFY_HIT_DMG(
    builder,    hit_obj_id_value
,     hit_after_hp_value
,     hit_count_value
    );
    builder.Finish(serializeds2c_NOTIFY_HIT_DMG);

    return CreateSendBuffer(builder, CREATE_PKT_ID::s2c_NOTIFY_HIT_DMG);
}
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_MONSTER_AGGRO_START(
    const Nagox::Enum::GROUP_TYPE& group_type,
    const uint8_t obj_type_info,
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    builder.Clear();
    const auto group_type_value = group_type;
    const auto obj_type_info_value = obj_type_info;
   const auto serializeds2c_MONSTER_AGGRO_START = Nagox::Protocol::Creates2c_MONSTER_AGGRO_START(
    builder,    group_type_value
,     obj_type_info_value
    );
    builder.Finish(serializeds2c_MONSTER_AGGRO_START);

    return CreateSendBuffer(builder, CREATE_PKT_ID::s2c_MONSTER_AGGRO_START);
}
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_MONSTER_AGGRO_END(
    const Nagox::Enum::GROUP_TYPE& group_type,
    const uint8_t obj_type_info,
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    builder.Clear();
    const auto group_type_value = group_type;
    const auto obj_type_info_value = obj_type_info;
   const auto serializeds2c_MONSTER_AGGRO_END = Nagox::Protocol::Creates2c_MONSTER_AGGRO_END(
    builder,    group_type_value
,     obj_type_info_value
    );
    builder.Finish(serializeds2c_MONSTER_AGGRO_END);

    return CreateSendBuffer(builder, CREATE_PKT_ID::s2c_MONSTER_AGGRO_END);
}
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_PLAYER_ATTACK(
    const uint64_t atk_player_id,
    const float body_angle,
    const Nagox::Struct::Vec3& atk_pos,
    const Nagox::Enum::SKILL_TYPE& atk_type,
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    builder.Clear();
    const auto atk_player_id_value = atk_player_id;
    const auto body_angle_value = body_angle;
    const auto atk_pos_offset = &atk_pos;
    const auto atk_type_value = atk_type;
   const auto serializeds2c_PLAYER_ATTACK = Nagox::Protocol::Creates2c_PLAYER_ATTACK(
    builder,    atk_player_id_value
,     body_angle_value
,     atk_pos_offset
,     atk_type_value
    );
    builder.Finish(serializeds2c_PLAYER_ATTACK);

    return CreateSendBuffer(builder, CREATE_PKT_ID::s2c_PLAYER_ATTACK);
}
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_PLAYER_DEATH(
    const uint64_t player_id,
    const Nagox::Struct::Vec3& rebirth_pos,
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    builder.Clear();
    const auto player_id_value = player_id;
    const auto rebirth_pos_offset = &rebirth_pos;
   const auto serializeds2c_PLAYER_DEATH = Nagox::Protocol::Creates2c_PLAYER_DEATH(
    builder,    player_id_value
,     rebirth_pos_offset
    );
    builder.Finish(serializeds2c_PLAYER_DEATH);

    return CreateSendBuffer(builder, CREATE_PKT_ID::s2c_PLAYER_DEATH);
}
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_REQUEST_QUEST(
    const uint64_t quest_id,
    const bool is_accept,
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    builder.Clear();
    const auto quest_id_value = quest_id;
    const auto is_accept_value = is_accept;
   const auto serializeds2c_REQUEST_QUEST = Nagox::Protocol::Creates2c_REQUEST_QUEST(
    builder,    quest_id_value
,     is_accept_value
    );
    builder.Finish(serializeds2c_REQUEST_QUEST);

    return CreateSendBuffer(builder, CREATE_PKT_ID::s2c_REQUEST_QUEST);
}
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_PROCESS_COMMON_QUEST(
    const uint64_t quest_id,
    const Nagox::Enum::MONSTER_TYPE& mon_type,
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    builder.Clear();
    const auto quest_id_value = quest_id;
    const auto mon_type_value = mon_type;
   const auto serializeds2c_PROCESS_COMMON_QUEST = Nagox::Protocol::Creates2c_PROCESS_COMMON_QUEST(
    builder,    quest_id_value
,     mon_type_value
    );
    builder.Finish(serializeds2c_PROCESS_COMMON_QUEST);

    return CreateSendBuffer(builder, CREATE_PKT_ID::s2c_PROCESS_COMMON_QUEST);
}
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_CLEAR_QUEST(
    const uint64_t quest_id,
    const uint8_t is_clear,
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    builder.Clear();
    const auto quest_id_value = quest_id;
    const auto is_clear_value = is_clear;
   const auto serializeds2c_CLEAR_QUEST = Nagox::Protocol::Creates2c_CLEAR_QUEST(
    builder,    quest_id_value
,     is_clear_value
    );
    builder.Finish(serializeds2c_CLEAR_QUEST);

    return CreateSendBuffer(builder, CREATE_PKT_ID::s2c_CLEAR_QUEST);
}
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_FIRE_PROJ(
    const uint64_t shoot_obj_id,
    const uint64_t proj_id,
    const uint8_t proj_type,
    const Nagox::Struct::Vec3& pos,
    const Nagox::Struct::Vec3& vel,
    const float radius,
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    builder.Clear();
    const auto shoot_obj_id_value = shoot_obj_id;
    const auto proj_id_value = proj_id;
    const auto proj_type_value = proj_type;
    const auto pos_offset = &pos;
    const auto vel_offset = &vel;
    const auto radius_value = radius;
   const auto serializeds2c_FIRE_PROJ = Nagox::Protocol::Creates2c_FIRE_PROJ(
    builder,    shoot_obj_id_value
,     proj_id_value
,     proj_type_value
,     pos_offset
,     vel_offset
,     radius_value
    );
    builder.Finish(serializeds2c_FIRE_PROJ);

    return CreateSendBuffer(builder, CREATE_PKT_ID::s2c_FIRE_PROJ);
}
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_ACQUIRE_ITEM(
    const uint64_t get_user_id,
    const uint64_t item_obj_id,
    const uint8_t item_detail_id,
    const uint8_t item_stack_size,
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    builder.Clear();
    const auto get_user_id_value = get_user_id;
    const auto item_obj_id_value = item_obj_id;
    const auto item_detail_id_value = item_detail_id;
    const auto item_stack_size_value = item_stack_size;
   const auto serializeds2c_ACQUIRE_ITEM = Nagox::Protocol::Creates2c_ACQUIRE_ITEM(
    builder,    get_user_id_value
,     item_obj_id_value
,     item_detail_id_value
,     item_stack_size_value
    );
    builder.Finish(serializeds2c_ACQUIRE_ITEM);

    return CreateSendBuffer(builder, CREATE_PKT_ID::s2c_ACQUIRE_ITEM);
}
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_USE_QUICK_SLOT_ITEM(
    const uint64_t use_user_id,
    const uint8_t item_id,
    const uint8_t quick_slot_idx,
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    builder.Clear();
    const auto use_user_id_value = use_user_id;
    const auto item_id_value = item_id;
    const auto quick_slot_idx_value = quick_slot_idx;
   const auto serializeds2c_USE_QUICK_SLOT_ITEM = Nagox::Protocol::Creates2c_USE_QUICK_SLOT_ITEM(
    builder,    use_user_id_value
,     item_id_value
,     quick_slot_idx_value
    );
    builder.Finish(serializeds2c_USE_QUICK_SLOT_ITEM);

    return CreateSendBuffer(builder, CREATE_PKT_ID::s2c_USE_QUICK_SLOT_ITEM);
}
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_CRAFT_ITEM(
    const uint8_t recipe_id,
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    builder.Clear();
    const auto recipe_id_value = recipe_id;
   const auto serializeds2c_CRAFT_ITEM = Nagox::Protocol::Creates2c_CRAFT_ITEM(
    builder,    recipe_id_value
    );
    builder.Finish(serializeds2c_CRAFT_ITEM);

    return CreateSendBuffer(builder, CREATE_PKT_ID::s2c_CRAFT_ITEM);
}
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_REGISTER_PARTY_QUEST(
    const int32_t quest_id,
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    builder.Clear();
    const auto quest_id_value = quest_id;
   const auto serializeds2c_REGISTER_PARTY_QUEST = Nagox::Protocol::Creates2c_REGISTER_PARTY_QUEST(
    builder,    quest_id_value
    );
    builder.Finish(serializeds2c_REGISTER_PARTY_QUEST);

    return CreateSendBuffer(builder, CREATE_PKT_ID::s2c_REGISTER_PARTY_QUEST);
}
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_ACQUIRE_PARTY_LIST(
    const Vector<uint32_t> party_leader_ids,
    const int32_t target_quest_id,
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    builder.Clear();
    const auto party_leader_ids_offset = builder.CreateVector(party_leader_ids);
    const auto target_quest_id_value = target_quest_id;
   const auto serializeds2c_ACQUIRE_PARTY_LIST = Nagox::Protocol::Creates2c_ACQUIRE_PARTY_LIST(
    builder,    party_leader_ids_offset
,     target_quest_id_value
    );
    builder.Finish(serializeds2c_ACQUIRE_PARTY_LIST);

    return CreateSendBuffer(builder, CREATE_PKT_ID::s2c_ACQUIRE_PARTY_LIST);
}
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_INVITE_PARTY_QUEST(
    const uint32_t target_party_leader_id,
    const int32_t target_party_quest_id,
    const std::string_view& target_party_leader_name,
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    builder.Clear();
    const auto target_party_leader_id_value = target_party_leader_id;
    const auto target_party_quest_id_value = target_party_quest_id;
    const auto target_party_leader_name_offset = builder.CreateString(target_party_leader_name);
   const auto serializeds2c_INVITE_PARTY_QUEST = Nagox::Protocol::Creates2c_INVITE_PARTY_QUEST(
    builder,    target_party_leader_id_value
,     target_party_quest_id_value
,     target_party_leader_name_offset
    );
    builder.Finish(serializeds2c_INVITE_PARTY_QUEST);

    return CreateSendBuffer(builder, CREATE_PKT_ID::s2c_INVITE_PARTY_QUEST);
}
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_INVITE_PARTY_RESULT(
    const uint32_t target_party_leader_id,
    const uint32_t target_user_id,
    const bool invite_result,
    const std::string_view& target_user_name,
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    builder.Clear();
    const auto target_party_leader_id_value = target_party_leader_id;
    const auto target_user_id_value = target_user_id;
    const auto invite_result_value = invite_result;
    const auto target_user_name_offset = builder.CreateString(target_user_name);
   const auto serializeds2c_INVITE_PARTY_RESULT = Nagox::Protocol::Creates2c_INVITE_PARTY_RESULT(
    builder,    target_party_leader_id_value
,     target_user_id_value
,     invite_result_value
,     target_user_name_offset
    );
    builder.Finish(serializeds2c_INVITE_PARTY_RESULT);

    return CreateSendBuffer(builder, CREATE_PKT_ID::s2c_INVITE_PARTY_RESULT);
}
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_PARTY_JOIN_REQUEST(
    const uint32_t target_user_id,
    const std::string_view& target_user_name,
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    builder.Clear();
    const auto target_user_id_value = target_user_id;
    const auto target_user_name_offset = builder.CreateString(target_user_name);
   const auto serializeds2c_PARTY_JOIN_REQUEST = Nagox::Protocol::Creates2c_PARTY_JOIN_REQUEST(
    builder,    target_user_id_value
,     target_user_name_offset
    );
    builder.Finish(serializeds2c_PARTY_JOIN_REQUEST);

    return CreateSendBuffer(builder, CREATE_PKT_ID::s2c_PARTY_JOIN_REQUEST);
}
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_PARTY_JOIN_REQUEST_RESULT(
    const uint32_t target_user_id,
    const bool request_result,
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    builder.Clear();
    const auto target_user_id_value = target_user_id;
    const auto request_result_value = request_result;
   const auto serializeds2c_PARTY_JOIN_REQUEST_RESULT = Nagox::Protocol::Creates2c_PARTY_JOIN_REQUEST_RESULT(
    builder,    target_user_id_value
,     request_result_value
    );
    builder.Finish(serializeds2c_PARTY_JOIN_REQUEST_RESULT);

    return CreateSendBuffer(builder, CREATE_PKT_ID::s2c_PARTY_JOIN_REQUEST_RESULT);
}
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_PARTY_JOIN_NEW_PLAYER(
    const uint32_t target_user_id,
    const std::string_view& target_user_name,
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    builder.Clear();
    const auto target_user_id_value = target_user_id;
    const auto target_user_name_offset = builder.CreateString(target_user_name);
   const auto serializeds2c_PARTY_JOIN_NEW_PLAYER = Nagox::Protocol::Creates2c_PARTY_JOIN_NEW_PLAYER(
    builder,    target_user_id_value
,     target_user_name_offset
    );
    builder.Finish(serializeds2c_PARTY_JOIN_NEW_PLAYER);

    return CreateSendBuffer(builder, CREATE_PKT_ID::s2c_PARTY_JOIN_NEW_PLAYER);
}
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_QUEST_END(
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    builder.Clear();
   const auto serializeds2c_QUEST_END = Nagox::Protocol::Creates2c_QUEST_END(
    builder    );
    builder.Finish(serializeds2c_QUEST_END);

    return CreateSendBuffer(builder, CREATE_PKT_ID::s2c_QUEST_END);
}
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_PARTY_QUEST_START(
    const uint32_t quest_id,
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    builder.Clear();
    const auto quest_id_value = quest_id;
   const auto serializeds2c_PARTY_QUEST_START = Nagox::Protocol::Creates2c_PARTY_QUEST_START(
    builder,    quest_id_value
    );
    builder.Finish(serializeds2c_PARTY_QUEST_START);

    return CreateSendBuffer(builder, CREATE_PKT_ID::s2c_PARTY_QUEST_START);
}
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_PARTY_OUT(
    const uint32_t out_user_id,
    const uint32_t cur_leader_id,
    const std::string_view& out_user_name,
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    builder.Clear();
    const auto out_user_id_value = out_user_id;
    const auto cur_leader_id_value = cur_leader_id;
    const auto out_user_name_offset = builder.CreateString(out_user_name);
   const auto serializeds2c_PARTY_OUT = Nagox::Protocol::Creates2c_PARTY_OUT(
    builder,    out_user_id_value
,     cur_leader_id_value
,     out_user_name_offset
    );
    builder.Finish(serializeds2c_PARTY_OUT);

    return CreateSendBuffer(builder, CREATE_PKT_ID::s2c_PARTY_OUT);
}
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_PARTY_QUEST_CLEAR(
    const int32_t party_quest_id,
    const Nagox::Struct::Vec3& clear_tree_pos,
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    builder.Clear();
    const auto party_quest_id_value = party_quest_id;
    const auto clear_tree_pos_offset = &clear_tree_pos;
   const auto serializeds2c_PARTY_QUEST_CLEAR = Nagox::Protocol::Creates2c_PARTY_QUEST_CLEAR(
    builder,    party_quest_id_value
,     clear_tree_pos_offset
    );
    builder.Finish(serializeds2c_PARTY_QUEST_CLEAR);

    return CreateSendBuffer(builder, CREATE_PKT_ID::s2c_PARTY_QUEST_CLEAR);
}
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_PARTY_MEMBERS_INFORMATION(
    const Vector<uint32_t> party_member_ids,
    const Vector<String> party_member_names,
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    builder.Clear();
    const auto party_member_ids_offset = builder.CreateVector(party_member_ids);
    const auto party_member_names_offset = builder.CreateVectorOfStrings(party_member_names);
   const auto serializeds2c_PARTY_MEMBERS_INFORMATION = Nagox::Protocol::Creates2c_PARTY_MEMBERS_INFORMATION(
    builder,    party_member_ids_offset
,     party_member_names_offset
    );
    builder.Finish(serializeds2c_PARTY_MEMBERS_INFORMATION);

    return CreateSendBuffer(builder, CREATE_PKT_ID::s2c_PARTY_MEMBERS_INFORMATION);
}
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_CHANGE_HARVEST_STATE(
    const uint32_t harvest_id,
    const bool is_active,
    const uint16_t harvest_mesh_type,
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    builder.Clear();
    const auto harvest_id_value = harvest_id;
    const auto is_active_value = is_active;
    const auto harvest_mesh_type_value = harvest_mesh_type;
   const auto serializeds2c_CHANGE_HARVEST_STATE = Nagox::Protocol::Creates2c_CHANGE_HARVEST_STATE(
    builder,    harvest_id_value
,     is_active_value
,     harvest_mesh_type_value
    );
    builder.Finish(serializeds2c_CHANGE_HARVEST_STATE);

    return CreateSendBuffer(builder, CREATE_PKT_ID::s2c_CHANGE_HARVEST_STATE);
}
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_NOTIFY_USER_DETAIL_INFO(
    const uint32_t obj_id,
    const std::string_view& user_name,
    const uint32_t weapon_id,
    const uint32_t armor_id,
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    builder.Clear();
    const auto obj_id_value = obj_id;
    const auto user_name_offset = builder.CreateString(user_name);
    const auto weapon_id_value = weapon_id;
    const auto armor_id_value = armor_id;
   const auto serializeds2c_NOTIFY_USER_DETAIL_INFO = Nagox::Protocol::Creates2c_NOTIFY_USER_DETAIL_INFO(
    builder,    obj_id_value
,     user_name_offset
,     weapon_id_value
,     armor_id_value
    );
    builder.Finish(serializeds2c_NOTIFY_USER_DETAIL_INFO);

    return CreateSendBuffer(builder, CREATE_PKT_ID::s2c_NOTIFY_USER_DETAIL_INFO);
}
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_FORCED_MOVE(
    const uint32_t target_user_id,
    const Nagox::Struct::Vec3& target_pos,
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    builder.Clear();
    const auto target_user_id_value = target_user_id;
    const auto target_pos_offset = &target_pos;
   const auto serializeds2c_FORCED_MOVE = Nagox::Protocol::Creates2c_FORCED_MOVE(
    builder,    target_user_id_value
,     target_pos_offset
    );
    builder.Finish(serializeds2c_FORCED_MOVE);

    return CreateSendBuffer(builder, CREATE_PKT_ID::s2c_FORCED_MOVE);
}
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_BOSS_ROOM_ENTER(
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    builder.Clear();
   const auto serializeds2c_BOSS_ROOM_ENTER = Nagox::Protocol::Creates2c_BOSS_ROOM_ENTER(
    builder    );
    builder.Finish(serializeds2c_BOSS_ROOM_ENTER);

    return CreateSendBuffer(builder, CREATE_PKT_ID::s2c_BOSS_ROOM_ENTER);
}
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_BOSS_FLY(
    const Nagox::Struct::Vec3& target_pos,
    const Nagox::Enum::BOSS_FLY_TYPE& boss_fly_type,
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    builder.Clear();
    const auto target_pos_offset = &target_pos;
    const auto boss_fly_type_value = boss_fly_type;
   const auto serializeds2c_BOSS_FLY = Nagox::Protocol::Creates2c_BOSS_FLY(
    builder,    target_pos_offset
,     boss_fly_type_value
    );
    builder.Finish(serializeds2c_BOSS_FLY);

    return CreateSendBuffer(builder, CREATE_PKT_ID::s2c_BOSS_FLY);
}
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_BOSS_MOVE(
    const Nagox::Struct::Vec3& target_pos,
    const float boss_speed,
    const Nagox::Enum::BOSS_MOVE_TYPE& boss_move_type,
    const float boss_angle,
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    builder.Clear();
    const auto target_pos_offset = &target_pos;
    const auto boss_speed_value = boss_speed;
    const auto boss_move_type_value = boss_move_type;
    const auto boss_angle_value = boss_angle;
   const auto serializeds2c_BOSS_MOVE = Nagox::Protocol::Creates2c_BOSS_MOVE(
    builder,    target_pos_offset
,     boss_speed_value
,     boss_move_type_value
,     boss_angle_value
    );
    builder.Finish(serializeds2c_BOSS_MOVE);

    return CreateSendBuffer(builder, CREATE_PKT_ID::s2c_BOSS_MOVE);
}
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_BOSS_PROJ_MARK(
    const Nagox::Struct::Vec3& mark_pos,
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    builder.Clear();
    const auto mark_pos_offset = &mark_pos;
   const auto serializeds2c_BOSS_PROJ_MARK = Nagox::Protocol::Creates2c_BOSS_PROJ_MARK(
    builder,    mark_pos_offset
    );
    builder.Finish(serializeds2c_BOSS_PROJ_MARK);

    return CreateSendBuffer(builder, CREATE_PKT_ID::s2c_BOSS_PROJ_MARK);
}
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_HEAL(
    const uint32_t target_obj_id,
    const int32_t heal_val,
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    builder.Clear();
    const auto target_obj_id_value = target_obj_id;
    const auto heal_val_value = heal_val;
   const auto serializeds2c_HEAL = Nagox::Protocol::Creates2c_HEAL(
    builder,    target_obj_id_value
,     heal_val_value
    );
    builder.Finish(serializeds2c_HEAL);

    return CreateSendBuffer(builder, CREATE_PKT_ID::s2c_HEAL);
}
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_DASH(
    const uint32_t dash_obj_id,
    const Nagox::Struct::Vec3& target_pos,
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    builder.Clear();
    const auto dash_obj_id_value = dash_obj_id;
    const auto target_pos_offset = &target_pos;
   const auto serializeds2c_DASH = Nagox::Protocol::Creates2c_DASH(
    builder,    dash_obj_id_value
,     target_pos_offset
    );
    builder.Finish(serializeds2c_DASH);

    return CreateSendBuffer(builder, CREATE_PKT_ID::s2c_DASH);
}
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_ARROW_RAIN(
    const uint64_t atk_player_id,
    const float body_angle,
    const Nagox::Struct::Vec3& atk_pos,
    const Nagox::Enum::SKILL_TYPE& atk_type,
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    builder.Clear();
    const auto atk_player_id_value = atk_player_id;
    const auto body_angle_value = body_angle;
    const auto atk_pos_offset = &atk_pos;
    const auto atk_type_value = atk_type;
   const auto serializeds2c_ARROW_RAIN = Nagox::Protocol::Creates2c_ARROW_RAIN(
    builder,    atk_player_id_value
,     body_angle_value
,     atk_pos_offset
,     atk_type_value
    );
    builder.Finish(serializeds2c_ARROW_RAIN);

    return CreateSendBuffer(builder, CREATE_PKT_ID::s2c_ARROW_RAIN);
}
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_CHAT(
    const uint32_t chat_user_id,
    const std::string_view& char_user_name,
    const std::string_view& char_msg,
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    builder.Clear();
    const auto chat_user_id_value = chat_user_id;
    const auto char_user_name_offset = builder.CreateString(char_user_name);
    const auto char_msg_offset = builder.CreateString(char_msg);
   const auto serializeds2c_CHAT = Nagox::Protocol::Creates2c_CHAT(
    builder,    chat_user_id_value
,     char_user_name_offset
,     char_msg_offset
    );
    builder.Finish(serializeds2c_CHAT);

    return CreateSendBuffer(builder, CREATE_PKT_ID::s2c_CHAT);
}
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_SHOOT_CATAPULT(
    const uint64_t catapult_id,
    const Nagox::Struct::Vec3& catapult_pos,
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    builder.Clear();
    const auto catapult_id_value = catapult_id;
    const auto catapult_pos_offset = &catapult_pos;
   const auto serializeds2c_SHOOT_CATAPULT = Nagox::Protocol::Creates2c_SHOOT_CATAPULT(
    builder,    catapult_id_value
,     catapult_pos_offset
    );
    builder.Finish(serializeds2c_SHOOT_CATAPULT);

    return CreateSendBuffer(builder, CREATE_PKT_ID::s2c_SHOOT_CATAPULT);
}
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_NOTIFY_CATAPULT(
    const Nagox::Struct::Vec3& catapult_pos,
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    builder.Clear();
    const auto catapult_pos_offset = &catapult_pos;
   const auto serializeds2c_NOTIFY_CATAPULT = Nagox::Protocol::Creates2c_NOTIFY_CATAPULT(
    builder,    catapult_pos_offset
    );
    builder.Finish(serializeds2c_NOTIFY_CATAPULT);

    return CreateSendBuffer(builder, CREATE_PKT_ID::s2c_NOTIFY_CATAPULT);
}
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_BOSS_CHANGE_PHASE(
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    builder.Clear();
   const auto serializeds2c_BOSS_CHANGE_PHASE = Nagox::Protocol::Creates2c_BOSS_CHANGE_PHASE(
    builder    );
    builder.Finish(serializeds2c_BOSS_CHANGE_PHASE);

    return CreateSendBuffer(builder, CREATE_PKT_ID::s2c_BOSS_CHANGE_PHASE);
}
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_BOSS_READY_TO_BREATH(
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    builder.Clear();
   const auto serializeds2c_BOSS_READY_TO_BREATH = Nagox::Protocol::Creates2c_BOSS_READY_TO_BREATH(
    builder    );
    builder.Finish(serializeds2c_BOSS_READY_TO_BREATH);

    return CreateSendBuffer(builder, CREATE_PKT_ID::s2c_BOSS_READY_TO_BREATH);
}
