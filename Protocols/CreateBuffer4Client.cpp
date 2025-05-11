#include "pch.h"
#include <flatbuffers/flatbuffers.h>
#include "../ClientNetwork/ClientNetHelper.h"
#include "enum_generated.h"
#include "struct_generated.h"
#include "protocol_generated.h"
#include "s2c_PacketHandler.h"

static NetHelper::S_ptr<NetHelper::SendBuffer> CreateSendBuffer(flatbuffers::FlatBufferBuilder& builder, const CREATE_PKT_ID pktId) noexcept
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
    return sendBuffer;
}

NetHelper::S_ptr<NetHelper::SendBuffer> Create_c2s_LOGIN(
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
NetHelper::S_ptr<NetHelper::SendBuffer> Create_c2s_PING_PONG(
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
NetHelper::S_ptr<NetHelper::SendBuffer> Create_c2s_ENTER(
    const Nagox::Struct::Vec3& pos,
    const Nagox::Enum::PLAYER_TYPE& player_type,
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    builder.Clear();
    const auto pos_offset = &pos;
    const auto player_type_value = player_type;
    const auto serializedc2s_ENTER = Nagox::Protocol::Createc2s_ENTER(
        builder
,        pos_offset,
        player_type_value    );
    builder.Finish(serializedc2s_ENTER);

    return CreateSendBuffer(builder, CREATE_PKT_ID::c2s_ENTER);
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
NetHelper::S_ptr<NetHelper::SendBuffer> Create_c2s_PLAYER_ATTACK(
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
NetHelper::S_ptr<NetHelper::SendBuffer> Create_c2s_PLAYER_DEATH(
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
NetHelper::S_ptr<NetHelper::SendBuffer> Create_c2s_REQUEST_QUEST(
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
NetHelper::S_ptr<NetHelper::SendBuffer> Create_c2s_FIRE_PROJ(
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
NetHelper::S_ptr<NetHelper::SendBuffer> Create_c2s_ACQUIRE_ITEM(
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
NetHelper::S_ptr<NetHelper::SendBuffer> Create_c2s_REQUEST_QUICK_SLOT(
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
NetHelper::S_ptr<NetHelper::SendBuffer> Create_c2s_USE_QUICK_SLOT_ITEM(
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
NetHelper::S_ptr<NetHelper::SendBuffer> Create_c2s_CRAFT_ITEM(
    const uint8_t recipe_id,
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    builder.Clear();
    const auto recipe_id_value = recipe_id;
    const auto serializedc2s_CRAFT_ITEM = Nagox::Protocol::Createc2s_CRAFT_ITEM(
        builder
,        recipe_id_value    );
    builder.Finish(serializedc2s_CRAFT_ITEM);

    return CreateSendBuffer(builder, CREATE_PKT_ID::c2s_CRAFT_ITEM);
}
NetHelper::S_ptr<NetHelper::SendBuffer> Create_c2s_REGISTER_PARTY_QUEST(
    const int32_t quest_id,
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    builder.Clear();
    const auto quest_id_value = quest_id;
    const auto serializedc2s_REGISTER_PARTY_QUEST = Nagox::Protocol::Createc2s_REGISTER_PARTY_QUEST(
        builder
,        quest_id_value    );
    builder.Finish(serializedc2s_REGISTER_PARTY_QUEST);

    return CreateSendBuffer(builder, CREATE_PKT_ID::c2s_REGISTER_PARTY_QUEST);
}
NetHelper::S_ptr<NetHelper::SendBuffer> Create_c2s_ACQUIRE_PARTY_LIST(
    const int32_t target_quest_id,
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    builder.Clear();
    const auto target_quest_id_value = target_quest_id;
    const auto serializedc2s_ACQUIRE_PARTY_LIST = Nagox::Protocol::Createc2s_ACQUIRE_PARTY_LIST(
        builder
,        target_quest_id_value    );
    builder.Finish(serializedc2s_ACQUIRE_PARTY_LIST);

    return CreateSendBuffer(builder, CREATE_PKT_ID::c2s_ACQUIRE_PARTY_LIST);
}
NetHelper::S_ptr<NetHelper::SendBuffer> Create_c2s_INVITE_PARTY_QUEST(
    const uint32_t target_user_id,
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    builder.Clear();
    const auto target_user_id_value = target_user_id;
    const auto serializedc2s_INVITE_PARTY_QUEST = Nagox::Protocol::Createc2s_INVITE_PARTY_QUEST(
        builder
,        target_user_id_value    );
    builder.Finish(serializedc2s_INVITE_PARTY_QUEST);

    return CreateSendBuffer(builder, CREATE_PKT_ID::c2s_INVITE_PARTY_QUEST);
}
NetHelper::S_ptr<NetHelper::SendBuffer> Create_c2s_INVITE_PARTY_RESULT(
    const uint32_t target_party_leader_id,
    const bool is_accept,
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    builder.Clear();
    const auto target_party_leader_id_value = target_party_leader_id;
    const auto is_accept_value = is_accept;
    const auto serializedc2s_INVITE_PARTY_RESULT = Nagox::Protocol::Createc2s_INVITE_PARTY_RESULT(
        builder
,        target_party_leader_id_value,
        is_accept_value    );
    builder.Finish(serializedc2s_INVITE_PARTY_RESULT);

    return CreateSendBuffer(builder, CREATE_PKT_ID::c2s_INVITE_PARTY_RESULT);
}
NetHelper::S_ptr<NetHelper::SendBuffer> Create_c2s_PARTY_JOIN_REQUEST(
    const uint32_t target_party_leader_id,
    const int32_t target_party_quest_id,
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    builder.Clear();
    const auto target_party_leader_id_value = target_party_leader_id;
    const auto target_party_quest_id_value = target_party_quest_id;
    const auto serializedc2s_PARTY_JOIN_REQUEST = Nagox::Protocol::Createc2s_PARTY_JOIN_REQUEST(
        builder
,        target_party_leader_id_value,
        target_party_quest_id_value    );
    builder.Finish(serializedc2s_PARTY_JOIN_REQUEST);

    return CreateSendBuffer(builder, CREATE_PKT_ID::c2s_PARTY_JOIN_REQUEST);
}
NetHelper::S_ptr<NetHelper::SendBuffer> Create_c2s_PARTY_JOIN_REQUEST_RESULT(
    const uint32_t target_party_leader_id,
    const uint32_t target_user_id,
    const bool request_result,
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    builder.Clear();
    const auto target_party_leader_id_value = target_party_leader_id;
    const auto target_user_id_value = target_user_id;
    const auto request_result_value = request_result;
    const auto serializedc2s_PARTY_JOIN_REQUEST_RESULT = Nagox::Protocol::Createc2s_PARTY_JOIN_REQUEST_RESULT(
        builder
,        target_party_leader_id_value,
        target_user_id_value,
        request_result_value    );
    builder.Finish(serializedc2s_PARTY_JOIN_REQUEST_RESULT);

    return CreateSendBuffer(builder, CREATE_PKT_ID::c2s_PARTY_JOIN_REQUEST_RESULT);
}
NetHelper::S_ptr<NetHelper::SendBuffer> Create_c2s_QUEST_START(
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    builder.Clear();
    const auto serializedc2s_QUEST_START = Nagox::Protocol::Createc2s_QUEST_START(
        builder
    );
    builder.Finish(serializedc2s_QUEST_START);

    return CreateSendBuffer(builder, CREATE_PKT_ID::c2s_QUEST_START);
}
NetHelper::S_ptr<NetHelper::SendBuffer> Create_c2s_QUEST_END(
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    builder.Clear();
    const auto serializedc2s_QUEST_END = Nagox::Protocol::Createc2s_QUEST_END(
        builder
    );
    builder.Finish(serializedc2s_QUEST_END);

    return CreateSendBuffer(builder, CREATE_PKT_ID::c2s_QUEST_END);
}
NetHelper::S_ptr<NetHelper::SendBuffer> Create_c2s_PARTY_OUT(
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    builder.Clear();
    const auto serializedc2s_PARTY_OUT = Nagox::Protocol::Createc2s_PARTY_OUT(
        builder
    );
    builder.Finish(serializedc2s_PARTY_OUT);

    return CreateSendBuffer(builder, CREATE_PKT_ID::c2s_PARTY_OUT);
}
NetHelper::S_ptr<NetHelper::SendBuffer> Create_c2s_CHANGE_HARVEST_STATE(
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    builder.Clear();
    const auto serializedc2s_CHANGE_HARVEST_STATE = Nagox::Protocol::Createc2s_CHANGE_HARVEST_STATE(
        builder
    );
    builder.Finish(serializedc2s_CHANGE_HARVEST_STATE);

    return CreateSendBuffer(builder, CREATE_PKT_ID::c2s_CHANGE_HARVEST_STATE);
}
