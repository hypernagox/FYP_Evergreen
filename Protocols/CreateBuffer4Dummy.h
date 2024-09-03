#pragma once
#include "../ServerCore/ServerCorePch.h"
#include "enum_generated.h"
#include "struct_generated.h"
#include "protocol_generated.h"

extern flatbuffers::FlatBufferBuilder* const CreateBuilder() noexcept;

static inline flatbuffers::FlatBufferBuilder* const GetBuilder() noexcept {
    thread_local flatbuffers::FlatBufferBuilder* const builder = CreateBuilder();
    builder->Clear();
    return builder;
}

ServerCore::S_ptr<ServerCore::SendBuffer> Create_c2s_LOGIN(
    const std::string_view& name,
    flatbuffers::FlatBufferBuilder* const builder_ptr = GetBuilder()
)noexcept;
ServerCore::S_ptr<ServerCore::SendBuffer> Create_c2s_ENTER(
    const Nagox::Struct::Vec3& pos,
    flatbuffers::FlatBufferBuilder* const builder_ptr = GetBuilder()
)noexcept;
ServerCore::S_ptr<ServerCore::SendBuffer> Create_c2s_MOVE(
    const Nagox::Struct::Vec3& pos,
    const Nagox::Struct::Vec3& vel,
    const Nagox::Struct::Vec3& accel,
    const float body_angle,
    const uint64_t time_stamp,
    flatbuffers::FlatBufferBuilder* const builder_ptr = GetBuilder()
)noexcept;
