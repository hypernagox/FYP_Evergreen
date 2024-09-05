#include "flatbuffers/pch/pch.h"
#include "flatbuffers/pch/flatc_pch.h"
#include "flatbuffers/flatbuffers.h"
#include "../ServerCore/ServerCorePch.h"
#include "enum_generated.h"
#include "struct_generated.h"
#include "protocol_generated.h"
#include "s2c_DummyPacketHandler.h"

static ServerCore::S_ptr<ServerCore::SendBuffer> CreateSendBuffer(const uint8_t* const flatBufferPtr, const PKT_ID pktId, const uint16_t dataSize) noexcept
{
    const uint16_t packetSize = dataSize + static_cast<c_uint16>(sizeof(ServerCore::PacketHeader));
    ServerCore::S_ptr<ServerCore::SendBuffer> sendBuffer = ServerCore::SendBufferMgr::Open(packetSize);
    ServerCore::PacketHeader* const __restrict header =
        reinterpret_cast<ServerCore::PacketHeader* const>(
            ::memcpy(sendBuffer->Buffer() + sizeof(ServerCore::PacketHeader), flatBufferPtr, dataSize)
            ) - 1;
    header->pkt_size = packetSize;
    header->pkt_id = static_cast<c_uint16>(pktId);
    sendBuffer->Close(packetSize);
    return sendBuffer;
}

ServerCore::S_ptr<ServerCore::SendBuffer> Create_c2s_LOGIN(
    const std::string_view& name,
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    const auto name_offset = builder.CreateString(name);
    const auto serializedc2s_LOGIN = Nagox::Protocol::Createc2s_LOGIN(
        builder
,        name_offset    );
    builder.Finish(serializedc2s_LOGIN);

    return CreateSendBuffer(builder.GetBufferPointer(), PKT_ID::c2s_LOGIN, builder.GetSize());
}
ServerCore::S_ptr<ServerCore::SendBuffer> Create_c2s_ENTER(
    const Nagox::Struct::Vec3& pos,
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    const auto pos_offset = &pos;
    const auto serializedc2s_ENTER = Nagox::Protocol::Createc2s_ENTER(
        builder
,        pos_offset    );
    builder.Finish(serializedc2s_ENTER);

    return CreateSendBuffer(builder.GetBufferPointer(), PKT_ID::c2s_ENTER, builder.GetSize());
}
ServerCore::S_ptr<ServerCore::SendBuffer> Create_c2s_MOVE(
    const Nagox::Struct::Vec3& pos,
    const Nagox::Struct::Vec3& vel,
    const Nagox::Struct::Vec3& accel,
    const float body_angle,
    const uint64_t time_stamp,
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    const auto pos_offset = &pos;
    const auto vel_offset = &vel;
    const auto accel_offset = &accel;
    const auto body_angle_value = body_angle;
    const auto time_stamp_value = time_stamp;
    const auto serializedc2s_MOVE = Nagox::Protocol::Createc2s_MOVE(
        builder
,        pos_offset,
        vel_offset,
        accel_offset,
        body_angle_value,
        time_stamp_value    );
    builder.Finish(serializedc2s_MOVE);

    return CreateSendBuffer(builder.GetBufferPointer(), PKT_ID::c2s_MOVE, builder.GetSize());
}
