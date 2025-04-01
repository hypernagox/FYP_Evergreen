#include "pch.h"
#include <flatbuffers/flatbuffers.h>
#include "../NagiocpX/NagiocpXPch.h"
#include "enum_generated.h"
#include "struct_generated.h"
#include "protocol_generated.h"
#include "s2c_DummyPacketHandler.h"

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

NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_c2s_LOGIN(
    const std::string_view& name,
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    builder.Clear();
    const auto name_offset = builder.CreateString(name);
    const auto serializedc2s_LOGIN = Nagox::Protocol::Createc2s_LOGIN(
        builder
,        name_offset    );
    builder.Finish(serializedc2s_LOGIN);

    return CreateSendBuffer(builder, CREATE_PKT_ID::c2s_LOGIN);
}
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_c2s_PING_PONG(
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    builder.Clear();
    const auto serializedc2s_PING_PONG = Nagox::Protocol::Createc2s_PING_PONG(
        builder
    );
    builder.Finish(serializedc2s_PING_PONG);

    return CreateSendBuffer(builder, CREATE_PKT_ID::c2s_PING_PONG);
}
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_c2s_ENTER(
    const Nagox::Struct::Vec3& pos,
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    builder.Clear();
    const auto pos_offset = &pos;
    const auto serializedc2s_ENTER = Nagox::Protocol::Createc2s_ENTER(
        builder
,        pos_offset    );
    builder.Finish(serializedc2s_ENTER);

    return CreateSendBuffer(builder, CREATE_PKT_ID::c2s_ENTER);
}
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_c2s_MOVE(
    const Nagox::Struct::Vec3& pos,
    const Nagox::Struct::Vec3& vel,
    const Nagox::Struct::Vec3& accel,
    const float body_angle,
    const uint64_t time_stamp,
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    builder.Clear();
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

    return CreateSendBuffer(builder, CREATE_PKT_ID::c2s_MOVE);
}
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_c2s_PLAYER_ATTACK(
    const float body_angle,
    const Nagox::Struct::Vec3& atk_pos,
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    builder.Clear();
    const auto body_angle_value = body_angle;
    const auto atk_pos_offset = &atk_pos;
    const auto serializedc2s_PLAYER_ATTACK = Nagox::Protocol::Createc2s_PLAYER_ATTACK(
        builder
,        body_angle_value,
        atk_pos_offset    );
    builder.Finish(serializedc2s_PLAYER_ATTACK);

    return CreateSendBuffer(builder, CREATE_PKT_ID::c2s_PLAYER_ATTACK);
}
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_c2s_PLAYER_DEATH(
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    builder.Clear();
    const auto serializedc2s_PLAYER_DEATH = Nagox::Protocol::Createc2s_PLAYER_DEATH(
        builder
    );
    builder.Finish(serializedc2s_PLAYER_DEATH);

    return CreateSendBuffer(builder, CREATE_PKT_ID::c2s_PLAYER_DEATH);
}
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_c2s_REQUEST_QUEST(
    const uint64_t quest_id,
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    builder.Clear();
    const auto quest_id_value = quest_id;
    const auto serializedc2s_REQUEST_QUEST = Nagox::Protocol::Createc2s_REQUEST_QUEST(
        builder
,        quest_id_value    );
    builder.Finish(serializedc2s_REQUEST_QUEST);

    return CreateSendBuffer(builder, CREATE_PKT_ID::c2s_REQUEST_QUEST);
}
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_c2s_FIRE_PROJ(
    const Nagox::Struct::Vec3& pos,
    const float body_angle,
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    builder.Clear();
    const auto pos_offset = &pos;
    const auto body_angle_value = body_angle;
    const auto serializedc2s_FIRE_PROJ = Nagox::Protocol::Createc2s_FIRE_PROJ(
        builder
,        pos_offset,
        body_angle_value    );
    builder.Finish(serializedc2s_FIRE_PROJ);

    return CreateSendBuffer(builder, CREATE_PKT_ID::c2s_FIRE_PROJ);
}
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_c2s_ACQUIRE_ITEM(
    const uint64_t item_id,
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    builder.Clear();
    const auto item_id_value = item_id;
    const auto serializedc2s_ACQUIRE_ITEM = Nagox::Protocol::Createc2s_ACQUIRE_ITEM(
        builder
,        item_id_value    );
    builder.Finish(serializedc2s_ACQUIRE_ITEM);

    return CreateSendBuffer(builder, CREATE_PKT_ID::c2s_ACQUIRE_ITEM);
}
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_c2s_REQUEST_QUICK_SLOT(
    const uint8_t item_id,
    const uint8_t quick_slot_idx,
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    builder.Clear();
    const auto item_id_value = item_id;
    const auto quick_slot_idx_value = quick_slot_idx;
    const auto serializedc2s_REQUEST_QUICK_SLOT = Nagox::Protocol::Createc2s_REQUEST_QUICK_SLOT(
        builder
,        item_id_value,
        quick_slot_idx_value    );
    builder.Finish(serializedc2s_REQUEST_QUICK_SLOT);

    return CreateSendBuffer(builder, CREATE_PKT_ID::c2s_REQUEST_QUICK_SLOT);
}
NagiocpX::S_ptr<NagiocpX::SendBuffer> Create_c2s_USE_QUICK_SLOT_ITEM(
    const uint8_t quick_slot_idx,
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    builder.Clear();
    const auto quick_slot_idx_value = quick_slot_idx;
    const auto serializedc2s_USE_QUICK_SLOT_ITEM = Nagox::Protocol::Createc2s_USE_QUICK_SLOT_ITEM(
        builder
,        quick_slot_idx_value    );
    builder.Finish(serializedc2s_USE_QUICK_SLOT_ITEM);

    return CreateSendBuffer(builder, CREATE_PKT_ID::c2s_USE_QUICK_SLOT_ITEM);
}
