#include "pch.h"
#include <flatbuffers/flatbuffers.h>
#include "../ServerCore/ServerCorePch.h"
#include "enum_generated.h"
#include "struct_generated.h"
#include "protocol_generated.h"
#include "c2s_PacketHandler.h"

static ServerCore::S_ptr<ServerCore::SendBuffer> CreateSendBuffer(flatbuffers::FlatBufferBuilder& builder, const PKT_ID pktId) noexcept
{
    const uint16_t dataSize = builder.GetSize();
    const uint16_t packetSize = dataSize + static_cast<c_uint16>(sizeof(ServerCore::PacketHeader));
    ServerCore::S_ptr<ServerCore::SendBuffer> sendBuffer = ServerCore::SendBufferMgr::Open(packetSize);
    ServerCore::PacketHeader* const __restrict header =
        reinterpret_cast<ServerCore::PacketHeader* const>(
            ::memcpy(sendBuffer->Buffer() + sizeof(ServerCore::PacketHeader), builder.GetBufferPointer(), dataSize)
            ) - 1;
    header->pkt_size = packetSize;
    header->pkt_id = static_cast<c_uint16>(pktId);
    sendBuffer->Close(packetSize);
    builder.Clear();
    return sendBuffer;
}

ServerCore::S_ptr<ServerCore::SendBuffer> Create_s2c_LOGIN(
    const uint32_t obj_id,
    const uint64_t server_time_stamp,
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    const auto obj_id_value = obj_id;
    const auto server_time_stamp_value = server_time_stamp;
    const auto serializeds2c_LOGIN = Nagox::Protocol::Creates2c_LOGIN(
        builder
,        obj_id_value,
        server_time_stamp_value    );
    builder.Finish(serializeds2c_LOGIN);

    return CreateSendBuffer(builder, PKT_ID::s2c_LOGIN);
}
ServerCore::S_ptr<ServerCore::SendBuffer> Create_s2c_PING_PONG(
    const uint64_t server_time_stamp,
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    const auto server_time_stamp_value = server_time_stamp;
    const auto serializeds2c_PING_PONG = Nagox::Protocol::Creates2c_PING_PONG(
        builder
,        server_time_stamp_value    );
    builder.Finish(serializeds2c_PING_PONG);

    return CreateSendBuffer(builder, PKT_ID::s2c_PING_PONG);
}
ServerCore::S_ptr<ServerCore::SendBuffer> Create_s2c_APPEAR_OBJECT(
    const uint32_t obj_id,
    const Nagox::Enum::GROUP_TYPE& group_type,
    const uint8_t obj_type_info,
    const Nagox::Struct::Vec3& appear_pos,
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    const auto obj_id_value = obj_id;
    const auto group_type_value = group_type;
    const auto obj_type_info_value = obj_type_info;
    const auto appear_pos_offset = &appear_pos;
    const auto serializeds2c_APPEAR_OBJECT = Nagox::Protocol::Creates2c_APPEAR_OBJECT(
        builder
,        obj_id_value,
        group_type_value,
        obj_type_info_value,
        appear_pos_offset    );
    builder.Finish(serializeds2c_APPEAR_OBJECT);

    return CreateSendBuffer(builder, PKT_ID::s2c_APPEAR_OBJECT);
}
ServerCore::S_ptr<ServerCore::SendBuffer> Create_s2c_REMOVE_OBJECT(
    const uint32_t obj_id,
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    const auto obj_id_value = obj_id;
    const auto serializeds2c_REMOVE_OBJECT = Nagox::Protocol::Creates2c_REMOVE_OBJECT(
        builder
,        obj_id_value    );
    builder.Finish(serializeds2c_REMOVE_OBJECT);

    return CreateSendBuffer(builder, PKT_ID::s2c_REMOVE_OBJECT);
}
ServerCore::S_ptr<ServerCore::SendBuffer> Create_s2c_MOVE(
    const uint64_t obj_id,
    const Nagox::Struct::Vec3& pos,
    const Nagox::Struct::Vec3& vel,
    const Nagox::Struct::Vec3& accel,
    const float body_angle,
    const uint64_t time_stamp,
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    const auto obj_id_value = obj_id;
    const auto pos_offset = &pos;
    const auto vel_offset = &vel;
    const auto accel_offset = &accel;
    const auto body_angle_value = body_angle;
    const auto time_stamp_value = time_stamp;
    const auto serializeds2c_MOVE = Nagox::Protocol::Creates2c_MOVE(
        builder
,        obj_id_value,
        pos_offset,
        vel_offset,
        accel_offset,
        body_angle_value,
        time_stamp_value    );
    builder.Finish(serializeds2c_MOVE);

    return CreateSendBuffer(builder, PKT_ID::s2c_MOVE);
}
ServerCore::S_ptr<ServerCore::SendBuffer> Create_s2c_MONSTER_ATTACK(
    const uint64_t obj_id,
    const uint64_t player_id,
    const uint32_t dmg,
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    const auto obj_id_value = obj_id;
    const auto player_id_value = player_id;
    const auto dmg_value = dmg;
    const auto serializeds2c_MONSTER_ATTACK = Nagox::Protocol::Creates2c_MONSTER_ATTACK(
        builder
,        obj_id_value,
        player_id_value,
        dmg_value    );
    builder.Finish(serializeds2c_MONSTER_ATTACK);

    return CreateSendBuffer(builder, PKT_ID::s2c_MONSTER_ATTACK);
}
ServerCore::S_ptr<ServerCore::SendBuffer> Create_s2c_MONSTER_AGGRO_START(
    const Nagox::Enum::GROUP_TYPE& group_type,
    const uint8_t obj_type_info,
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    const auto group_type_value = group_type;
    const auto obj_type_info_value = obj_type_info;
    const auto serializeds2c_MONSTER_AGGRO_START = Nagox::Protocol::Creates2c_MONSTER_AGGRO_START(
        builder
,        group_type_value,
        obj_type_info_value    );
    builder.Finish(serializeds2c_MONSTER_AGGRO_START);

    return CreateSendBuffer(builder, PKT_ID::s2c_MONSTER_AGGRO_START);
}
ServerCore::S_ptr<ServerCore::SendBuffer> Create_s2c_MONSTER_AGGRO_END(
    const Nagox::Enum::GROUP_TYPE& group_type,
    const uint8_t obj_type_info,
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    const auto group_type_value = group_type;
    const auto obj_type_info_value = obj_type_info;
    const auto serializeds2c_MONSTER_AGGRO_END = Nagox::Protocol::Creates2c_MONSTER_AGGRO_END(
        builder
,        group_type_value,
        obj_type_info_value    );
    builder.Finish(serializeds2c_MONSTER_AGGRO_END);

    return CreateSendBuffer(builder, PKT_ID::s2c_MONSTER_AGGRO_END);
}
ServerCore::S_ptr<ServerCore::SendBuffer> Create_s2c_PLAYER_ATTACK(
    const uint64_t atk_player_id,
    const float body_angle,
    const Nagox::Struct::Vec3& atk_pos,
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    const auto atk_player_id_value = atk_player_id;
    const auto body_angle_value = body_angle;
    const auto atk_pos_offset = &atk_pos;
    const auto serializeds2c_PLAYER_ATTACK = Nagox::Protocol::Creates2c_PLAYER_ATTACK(
        builder
,        atk_player_id_value,
        body_angle_value,
        atk_pos_offset    );
    builder.Finish(serializeds2c_PLAYER_ATTACK);

    return CreateSendBuffer(builder, PKT_ID::s2c_PLAYER_ATTACK);
}
ServerCore::S_ptr<ServerCore::SendBuffer> Create_s2c_PLAYER_DEATH(
    const uint64_t player_id,
    const Nagox::Struct::Vec3& rebirth_pos,
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    const auto player_id_value = player_id;
    const auto rebirth_pos_offset = &rebirth_pos;
    const auto serializeds2c_PLAYER_DEATH = Nagox::Protocol::Creates2c_PLAYER_DEATH(
        builder
,        player_id_value,
        rebirth_pos_offset    );
    builder.Finish(serializeds2c_PLAYER_DEATH);

    return CreateSendBuffer(builder, PKT_ID::s2c_PLAYER_DEATH);
}
ServerCore::S_ptr<ServerCore::SendBuffer> Create_s2c_REQUEST_QUEST(
    const uint64_t quest_id,
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    const auto quest_id_value = quest_id;
    const auto serializeds2c_REQUEST_QUEST = Nagox::Protocol::Creates2c_REQUEST_QUEST(
        builder
,        quest_id_value    );
    builder.Finish(serializeds2c_REQUEST_QUEST);

    return CreateSendBuffer(builder, PKT_ID::s2c_REQUEST_QUEST);
}
ServerCore::S_ptr<ServerCore::SendBuffer> Create_s2c_CLEAR_QUEST(
    const uint64_t quest_id,
    const uint8_t is_clear,
    flatbuffers::FlatBufferBuilder* const builder_ptr
)noexcept {
    auto& builder = *builder_ptr;
    const auto quest_id_value = quest_id;
    const auto is_clear_value = is_clear;
    const auto serializeds2c_CLEAR_QUEST = Nagox::Protocol::Creates2c_CLEAR_QUEST(
        builder
,        quest_id_value,
        is_clear_value    );
    builder.Finish(serializeds2c_CLEAR_QUEST);

    return CreateSendBuffer(builder, PKT_ID::s2c_CLEAR_QUEST);
}
