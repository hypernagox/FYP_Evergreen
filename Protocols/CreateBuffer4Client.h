#pragma once
#include "../ClientNetwork/ClientNetHelper.h"
#include "enum_generated.h"
#include "struct_generated.h"
#include "protocol_generated.h"

template<typename T>
using Vector = std::vector<T>;

extern flatbuffers::FlatBufferBuilder* const CreateBuilder() noexcept;

static inline flatbuffers::FlatBufferBuilder* const GetBuilder() noexcept { return CreateBuilder(); }

NetHelper::S_ptr<NetHelper::SendBuffer> Create_c2s_LOGIN(
    const std::string_view& name,
    flatbuffers::FlatBufferBuilder* const builder_ptr = GetBuilder()
)noexcept;
NetHelper::S_ptr<NetHelper::SendBuffer> Create_c2s_PING_PONG(
    flatbuffers::FlatBufferBuilder* const builder_ptr = GetBuilder()
)noexcept;
NetHelper::S_ptr<NetHelper::SendBuffer> Create_c2s_ENTER(
    const Nagox::Struct::Vec3& pos,
    const Nagox::Enum::PLAYER_TYPE& player_type,
    flatbuffers::FlatBufferBuilder* const builder_ptr = GetBuilder()
)noexcept;
NetHelper::S_ptr<NetHelper::SendBuffer> Create_c2s_MOVE(
    const Nagox::Struct::Vec3& pos,
    const Nagox::Struct::Vec3& vel,
    const Nagox::Struct::Vec3& accel,
    const float body_angle,
    const uint64_t time_stamp,
    flatbuffers::FlatBufferBuilder* const builder_ptr = GetBuilder()
)noexcept;
NetHelper::S_ptr<NetHelper::SendBuffer> Create_c2s_PLAYER_ATTACK(
    const float body_angle,
    const Nagox::Struct::Vec3& atk_pos,
    flatbuffers::FlatBufferBuilder* const builder_ptr = GetBuilder()
)noexcept;
NetHelper::S_ptr<NetHelper::SendBuffer> Create_c2s_PLAYER_DEATH(
    flatbuffers::FlatBufferBuilder* const builder_ptr = GetBuilder()
)noexcept;
NetHelper::S_ptr<NetHelper::SendBuffer> Create_c2s_REQUEST_QUEST(
    const uint64_t quest_id,
    flatbuffers::FlatBufferBuilder* const builder_ptr = GetBuilder()
)noexcept;
NetHelper::S_ptr<NetHelper::SendBuffer> Create_c2s_FIRE_PROJ(
    const Nagox::Struct::Vec3& pos,
    const float body_angle,
    flatbuffers::FlatBufferBuilder* const builder_ptr = GetBuilder()
)noexcept;
NetHelper::S_ptr<NetHelper::SendBuffer> Create_c2s_ACQUIRE_ITEM(
    const uint64_t item_id,
    flatbuffers::FlatBufferBuilder* const builder_ptr = GetBuilder()
)noexcept;
NetHelper::S_ptr<NetHelper::SendBuffer> Create_c2s_REQUEST_QUICK_SLOT(
    const uint8_t item_id,
    const uint8_t quick_slot_idx,
    flatbuffers::FlatBufferBuilder* const builder_ptr = GetBuilder()
)noexcept;
NetHelper::S_ptr<NetHelper::SendBuffer> Create_c2s_USE_QUICK_SLOT_ITEM(
    const uint8_t quick_slot_idx,
    flatbuffers::FlatBufferBuilder* const builder_ptr = GetBuilder()
)noexcept;
NetHelper::S_ptr<NetHelper::SendBuffer> Create_c2s_CRAFT_ITEM(
    const uint8_t recipe_id,
    flatbuffers::FlatBufferBuilder* const builder_ptr = GetBuilder()
)noexcept;
NetHelper::S_ptr<NetHelper::SendBuffer> Create_c2s_REGISTER_PARTY_QUEST(
    const int32_t quest_id,
    flatbuffers::FlatBufferBuilder* const builder_ptr = GetBuilder()
)noexcept;
NetHelper::S_ptr<NetHelper::SendBuffer> Create_c2s_ACQUIRE_PARTY_LIST(
    const int32_t target_quest_id,
    flatbuffers::FlatBufferBuilder* const builder_ptr = GetBuilder()
)noexcept;
NetHelper::S_ptr<NetHelper::SendBuffer> Create_c2s_INVITE_PARTY_QUEST(
    const uint32_t target_user_id,
    flatbuffers::FlatBufferBuilder* const builder_ptr = GetBuilder()
)noexcept;
NetHelper::S_ptr<NetHelper::SendBuffer> Create_c2s_INVITE_PARTY_RESULT(
    const uint32_t target_party_leader_id,
    const bool is_accept,
    flatbuffers::FlatBufferBuilder* const builder_ptr = GetBuilder()
)noexcept;
NetHelper::S_ptr<NetHelper::SendBuffer> Create_c2s_PARTY_JOIN_REQUEST(
    const uint32_t target_party_leader_id,
    const int32_t target_party_quest_id,
    flatbuffers::FlatBufferBuilder* const builder_ptr = GetBuilder()
)noexcept;
NetHelper::S_ptr<NetHelper::SendBuffer> Create_c2s_PARTY_JOIN_REQUEST_RESULT(
    const uint32_t target_party_leader_id,
    const uint32_t target_user_id,
    const bool request_result,
    flatbuffers::FlatBufferBuilder* const builder_ptr = GetBuilder()
)noexcept;
NetHelper::S_ptr<NetHelper::SendBuffer> Create_c2s_QUEST_START(
    flatbuffers::FlatBufferBuilder* const builder_ptr = GetBuilder()
)noexcept;
NetHelper::S_ptr<NetHelper::SendBuffer> Create_c2s_QUEST_END(
    flatbuffers::FlatBufferBuilder* const builder_ptr = GetBuilder()
)noexcept;
NetHelper::S_ptr<NetHelper::SendBuffer> Create_c2s_PARTY_OUT(
    flatbuffers::FlatBufferBuilder* const builder_ptr = GetBuilder()
)noexcept;
NetHelper::S_ptr<NetHelper::SendBuffer> Create_c2s_CHANGE_HARVEST_STATE(
    flatbuffers::FlatBufferBuilder* const builder_ptr = GetBuilder()
)noexcept;
