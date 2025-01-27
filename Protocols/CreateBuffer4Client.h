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
