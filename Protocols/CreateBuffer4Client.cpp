#include "pch.h"
#include <flatbuffers/flatbuffers.h>
#include "../ClientNetwork/ClientNetHelper.h"
#include "enum_generated.h"
#include "struct_generated.h"
#include "protocol_generated.h"
#include "s2c_PacketHandler.h"

static NetHelper::S_ptr<NetHelper::SendBuffer> CreateSendBuffer(flatbuffers::FlatBufferBuilder& builder, const PKT_ID pktId) noexcept
{
    const uint16_t dataSize = builder.GetSize();
    const uint16_t packetSize = dataSize + static_cast<c_uint16>(sizeof(NetHelper::PacketHeader));
    NetHelper::S_ptr<NetHelper::SendBuffer> sendBuffer = NetHelper::SendBufferMgr::Open(packetSize);
    NetHelper::PacketHeader* const __restrict header =
        reinterpret_cast<NetHelper::PacketHeader* const>(
            ::memcpy(sendBuffer->Buffer() + sizeof(NetHelper::PacketHeader), builder.GetBufferPointer(), dataSize)
            ) - 1;
    header->pkt_size = packetSize;
    header->pkt_id = static_cast<c_uint16>(pktId);
    sendBuffer->Close(packetSize);
    builder.Clear();
    return sendBuffer;
}

NetHelper::S_ptr<NetHelper::SendBuffer> Create_c2s_LOGIN(
    const std::string_view& name,
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    const auto name_offset = builder.CreateString(name);
    const auto serializedc2s_LOGIN = Nagox::Protocol::Createc2s_LOGIN(
        builder
,        name_offset    );
    builder.Finish(serializedc2s_LOGIN);

    return CreateSendBuffer(builder, PKT_ID::c2s_LOGIN);
}
NetHelper::S_ptr<NetHelper::SendBuffer> Create_c2s_ENTER(
    const Nagox::Struct::Vec3& pos,
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    const auto pos_offset = &pos;
    const auto serializedc2s_ENTER = Nagox::Protocol::Createc2s_ENTER(
        builder
,        pos_offset    );
    builder.Finish(serializedc2s_ENTER);

    return CreateSendBuffer(builder, PKT_ID::c2s_ENTER);
}
NetHelper::S_ptr<NetHelper::SendBuffer> Create_c2s_MOVE(
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

    return CreateSendBuffer(builder, PKT_ID::c2s_MOVE);
}
NetHelper::S_ptr<NetHelper::SendBuffer> Create_c2s_PLAYER_ATTACK(
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    const auto serializedc2s_PLAYER_ATTACK = Nagox::Protocol::Createc2s_PLAYER_ATTACK(
        builder
    );
    builder.Finish(serializedc2s_PLAYER_ATTACK);

    return CreateSendBuffer(builder, PKT_ID::c2s_PLAYER_ATTACK);
}
