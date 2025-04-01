#pragma once
#include "../NagiocpX/NagiocpXPch.h"
#include "enum_generated.h"
#include "struct_generated.h"
#include "protocol_generated.h"

template<typename T>
using Vector = NagiocpX::XVector<T>;

extern flatbuffers::FlatBufferBuilder* const CreateBuilder() noexcept;

static inline flatbuffers::FlatBufferBuilder* const GetBuilder() noexcept { return CreateBuilder(); }

NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_LOGIN(
    const uint32_t obj_id,
    const uint64_t server_time_stamp,
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
    flatbuffers::FlatBufferBuilder* const builder_ptr = GetBuilder()
)noexcept;
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_PLAYER_DEATH(
    const uint64_t player_id,
    const Nagox::Struct::Vec3& rebirth_pos,
    flatbuffers::FlatBufferBuilder* const builder_ptr = GetBuilder()
)noexcept;
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_REQUEST_QUEST(
    const uint64_t quest_id,
    flatbuffers::FlatBufferBuilder* const builder_ptr = GetBuilder()
)noexcept;
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_CLEAR_QUEST(
    const uint64_t quest_id,
    const uint8_t is_clear,
    flatbuffers::FlatBufferBuilder* const builder_ptr = GetBuilder()
)noexcept;
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_s2c_FIRE_PROJ(
    const uint64_t proj_id,
    const Nagox::Struct::Vec3& pos,
    const Nagox::Struct::Vec3& vel,
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
