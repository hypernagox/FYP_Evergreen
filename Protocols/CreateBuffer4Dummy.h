#pragma once
#include "../NagiocpX/NagiocpXPch.h"
#include "enum_generated.h"
#include "struct_generated.h"
#include "protocol_generated.h"

template<typename T>
using Vector = NagiocpX::XVector<T>;

extern flatbuffers::FlatBufferBuilder* const CreateBuilder() noexcept;

static inline flatbuffers::FlatBufferBuilder* const GetBuilder() noexcept { return CreateBuilder(); }

NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_c2s_LOGIN(
    const std::string_view& name,
    flatbuffers::FlatBufferBuilder* const builder_ptr = GetBuilder()
)noexcept;
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_c2s_PING_PONG(
    flatbuffers::FlatBufferBuilder* const builder_ptr = GetBuilder()
)noexcept;
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_c2s_ENTER(
    const Nagox::Struct::Vec3& pos,
    flatbuffers::FlatBufferBuilder* const builder_ptr = GetBuilder()
)noexcept;
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_c2s_MOVE(
    const Nagox::Struct::Vec3& pos,
    const Nagox::Struct::Vec3& vel,
    const Nagox::Struct::Vec3& accel,
    const float body_angle,
    const uint64_t time_stamp,
    flatbuffers::FlatBufferBuilder* const builder_ptr = GetBuilder()
)noexcept;
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_c2s_PLAYER_ATTACK(
    const float body_angle,
    const Nagox::Struct::Vec3& atk_pos,
    flatbuffers::FlatBufferBuilder* const builder_ptr = GetBuilder()
)noexcept;
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_c2s_PLAYER_DEATH(
    flatbuffers::FlatBufferBuilder* const builder_ptr = GetBuilder()
)noexcept;
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_c2s_REQUEST_QUEST(
    const uint64_t quest_id,
    flatbuffers::FlatBufferBuilder* const builder_ptr = GetBuilder()
)noexcept;
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_c2s_FIRE_PROJ(
    const Nagox::Struct::Vec3& pos,
    const float body_angle,
    flatbuffers::FlatBufferBuilder* const builder_ptr = GetBuilder()
)noexcept;
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_c2s_ACQUIRE_ITEM(
    const uint64_t item_id,
    flatbuffers::FlatBufferBuilder* const builder_ptr = GetBuilder()
)noexcept;
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_c2s_REQUEST_QUICK_SLOT(
    const uint8_t item_id,
    const uint8_t quick_slot_idx,
    flatbuffers::FlatBufferBuilder* const builder_ptr = GetBuilder()
)noexcept;
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_c2s_USE_QUICK_SLOT_ITEM(
    const uint8_t quick_slot_idx,
    flatbuffers::FlatBufferBuilder* const builder_ptr = GetBuilder()
)noexcept;
