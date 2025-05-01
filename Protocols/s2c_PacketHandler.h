#pragma once
#include "../ClientNetwork/ClientNetHelper.h"
#include "enum_generated.h"
#include "struct_generated.h"
#include "protocol_generated.h"

template<typename T> requires std::is_enum_v<T>
static inline consteval const uint16_t net_etoi(const T eType_) noexcept { return static_cast<const uint16_t>(eType_); }

enum class HANDLE_PKT_ID : uint16_t {
    s2c_LOGIN = 1000,
    s2c_PING_PONG = 1001,
    s2c_APPEAR_OBJECT = 1002,
    s2c_REMOVE_OBJECT = 1003,
    s2c_MOVE = 1004,
    s2c_MONSTER_ATTACK = 1005,
    s2c_NOTIFY_HIT_DMG = 1006,
    s2c_MONSTER_AGGRO_START = 1007,
    s2c_MONSTER_AGGRO_END = 1008,
    s2c_PLAYER_ATTACK = 1009,
    s2c_PLAYER_DEATH = 1010,
    s2c_REQUEST_QUEST = 1011,
    s2c_CLEAR_QUEST = 1012,
    s2c_FIRE_PROJ = 1013,
    s2c_ACQUIRE_ITEM = 1014,
    s2c_USE_QUICK_SLOT_ITEM = 1015,
    s2c_CRAFT_ITEM = 1016,
    s2c_REGISTER_PARTY_QUEST = 1017,
    s2c_ACQUIRE_PARTY_LIST = 1018,
    s2c_INVITE_PARTY_QUEST = 1019,
    s2c_INVITE_PARTY_RESULT = 1020,
    s2c_PARTY_JOIN_REQUEST = 1021,
    s2c_PARTY_JOIN_REQUEST_RESULT = 1022,
    s2c_PARTY_JOIN_NEW_PLAYER = 1023,
    s2c_PARTY_OUT = 1024,
    s2c_PARTY_QUEST_CLEAR = 1025,
    s2c_PARTY_MEMBERS_INFORMATION = 1026,
    s2c_GET_HARVEST = 1027,
};

enum class CREATE_PKT_ID : uint16_t {
    c2s_LOGIN = 1000,
    c2s_PING_PONG = 1001,
    c2s_ENTER = 1002,
    c2s_MOVE = 1003,
    c2s_PLAYER_ATTACK = 1004,
    c2s_PLAYER_DEATH = 1005,
    c2s_REQUEST_QUEST = 1006,
    c2s_FIRE_PROJ = 1007,
    c2s_ACQUIRE_ITEM = 1008,
    c2s_REQUEST_QUICK_SLOT = 1009,
    c2s_USE_QUICK_SLOT_ITEM = 1010,
    c2s_CRAFT_ITEM = 1011,
    c2s_REGISTER_PARTY_QUEST = 1012,
    c2s_ACQUIRE_PARTY_LIST = 1013,
    c2s_INVITE_PARTY_QUEST = 1014,
    c2s_INVITE_PARTY_RESULT = 1015,
    c2s_PARTY_JOIN_REQUEST = 1016,
    c2s_PARTY_JOIN_REQUEST_RESULT = 1017,
    c2s_QUEST_START = 1018,
    c2s_QUEST_END = 1019,
    c2s_PARTY_OUT = 1020,
};

class NetHelper::PacketSession;
class NetHelper::Session;
class NetHelper::SendBuffer;

flatbuffers::FlatBufferBuilder* const CreateBuilder()noexcept;
static inline const bool Handle_Invalid(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const BYTE* const pBuff_, const int32_t len_) noexcept { return false; }

const bool Handle_s2c_LOGIN(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_LOGIN& pkt_);
const bool Handle_s2c_PING_PONG(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_PING_PONG& pkt_);
const bool Handle_s2c_APPEAR_OBJECT(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_APPEAR_OBJECT& pkt_);
const bool Handle_s2c_REMOVE_OBJECT(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_REMOVE_OBJECT& pkt_);
const bool Handle_s2c_MOVE(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_MOVE& pkt_);
const bool Handle_s2c_MONSTER_ATTACK(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_MONSTER_ATTACK& pkt_);
const bool Handle_s2c_NOTIFY_HIT_DMG(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_NOTIFY_HIT_DMG& pkt_);
const bool Handle_s2c_MONSTER_AGGRO_START(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_MONSTER_AGGRO_START& pkt_);
const bool Handle_s2c_MONSTER_AGGRO_END(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_MONSTER_AGGRO_END& pkt_);
const bool Handle_s2c_PLAYER_ATTACK(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_PLAYER_ATTACK& pkt_);
const bool Handle_s2c_PLAYER_DEATH(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_PLAYER_DEATH& pkt_);
const bool Handle_s2c_REQUEST_QUEST(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_REQUEST_QUEST& pkt_);
const bool Handle_s2c_CLEAR_QUEST(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_CLEAR_QUEST& pkt_);
const bool Handle_s2c_FIRE_PROJ(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_FIRE_PROJ& pkt_);
const bool Handle_s2c_ACQUIRE_ITEM(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_ACQUIRE_ITEM& pkt_);
const bool Handle_s2c_USE_QUICK_SLOT_ITEM(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_USE_QUICK_SLOT_ITEM& pkt_);
const bool Handle_s2c_CRAFT_ITEM(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_CRAFT_ITEM& pkt_);
const bool Handle_s2c_REGISTER_PARTY_QUEST(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_REGISTER_PARTY_QUEST& pkt_);
const bool Handle_s2c_ACQUIRE_PARTY_LIST(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_ACQUIRE_PARTY_LIST& pkt_);
const bool Handle_s2c_INVITE_PARTY_QUEST(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_INVITE_PARTY_QUEST& pkt_);
const bool Handle_s2c_INVITE_PARTY_RESULT(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_INVITE_PARTY_RESULT& pkt_);
const bool Handle_s2c_PARTY_JOIN_REQUEST(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_PARTY_JOIN_REQUEST& pkt_);
const bool Handle_s2c_PARTY_JOIN_REQUEST_RESULT(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_PARTY_JOIN_REQUEST_RESULT& pkt_);
const bool Handle_s2c_PARTY_JOIN_NEW_PLAYER(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_PARTY_JOIN_NEW_PLAYER& pkt_);
const bool Handle_s2c_PARTY_OUT(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_PARTY_OUT& pkt_);
const bool Handle_s2c_PARTY_QUEST_CLEAR(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_PARTY_QUEST_CLEAR& pkt_);
const bool Handle_s2c_PARTY_MEMBERS_INFORMATION(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_PARTY_MEMBERS_INFORMATION& pkt_);
const bool Handle_s2c_GET_HARVEST(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_GET_HARVEST& pkt_);

class s2c_PacketHandler {
    using PacketHandlerFunc = const bool (*)(const NetHelper::S_ptr<NetHelper::PacketSession>&, const BYTE* const, const int32_t);
    constinit static inline PacketHandlerFunc g_fpPacketHandler[UINT16_MAX] = {};
public:
    static void Init() noexcept {
        RegisterHandler<HANDLE_PKT_ID::s2c_LOGIN, Nagox::Protocol::s2c_LOGIN, Handle_s2c_LOGIN>();
        RegisterHandler<HANDLE_PKT_ID::s2c_PING_PONG, Nagox::Protocol::s2c_PING_PONG, Handle_s2c_PING_PONG>();
        RegisterHandler<HANDLE_PKT_ID::s2c_APPEAR_OBJECT, Nagox::Protocol::s2c_APPEAR_OBJECT, Handle_s2c_APPEAR_OBJECT>();
        RegisterHandler<HANDLE_PKT_ID::s2c_REMOVE_OBJECT, Nagox::Protocol::s2c_REMOVE_OBJECT, Handle_s2c_REMOVE_OBJECT>();
        RegisterHandler<HANDLE_PKT_ID::s2c_MOVE, Nagox::Protocol::s2c_MOVE, Handle_s2c_MOVE>();
        RegisterHandler<HANDLE_PKT_ID::s2c_MONSTER_ATTACK, Nagox::Protocol::s2c_MONSTER_ATTACK, Handle_s2c_MONSTER_ATTACK>();
        RegisterHandler<HANDLE_PKT_ID::s2c_NOTIFY_HIT_DMG, Nagox::Protocol::s2c_NOTIFY_HIT_DMG, Handle_s2c_NOTIFY_HIT_DMG>();
        RegisterHandler<HANDLE_PKT_ID::s2c_MONSTER_AGGRO_START, Nagox::Protocol::s2c_MONSTER_AGGRO_START, Handle_s2c_MONSTER_AGGRO_START>();
        RegisterHandler<HANDLE_PKT_ID::s2c_MONSTER_AGGRO_END, Nagox::Protocol::s2c_MONSTER_AGGRO_END, Handle_s2c_MONSTER_AGGRO_END>();
        RegisterHandler<HANDLE_PKT_ID::s2c_PLAYER_ATTACK, Nagox::Protocol::s2c_PLAYER_ATTACK, Handle_s2c_PLAYER_ATTACK>();
        RegisterHandler<HANDLE_PKT_ID::s2c_PLAYER_DEATH, Nagox::Protocol::s2c_PLAYER_DEATH, Handle_s2c_PLAYER_DEATH>();
        RegisterHandler<HANDLE_PKT_ID::s2c_REQUEST_QUEST, Nagox::Protocol::s2c_REQUEST_QUEST, Handle_s2c_REQUEST_QUEST>();
        RegisterHandler<HANDLE_PKT_ID::s2c_CLEAR_QUEST, Nagox::Protocol::s2c_CLEAR_QUEST, Handle_s2c_CLEAR_QUEST>();
        RegisterHandler<HANDLE_PKT_ID::s2c_FIRE_PROJ, Nagox::Protocol::s2c_FIRE_PROJ, Handle_s2c_FIRE_PROJ>();
        RegisterHandler<HANDLE_PKT_ID::s2c_ACQUIRE_ITEM, Nagox::Protocol::s2c_ACQUIRE_ITEM, Handle_s2c_ACQUIRE_ITEM>();
        RegisterHandler<HANDLE_PKT_ID::s2c_USE_QUICK_SLOT_ITEM, Nagox::Protocol::s2c_USE_QUICK_SLOT_ITEM, Handle_s2c_USE_QUICK_SLOT_ITEM>();
        RegisterHandler<HANDLE_PKT_ID::s2c_CRAFT_ITEM, Nagox::Protocol::s2c_CRAFT_ITEM, Handle_s2c_CRAFT_ITEM>();
        RegisterHandler<HANDLE_PKT_ID::s2c_REGISTER_PARTY_QUEST, Nagox::Protocol::s2c_REGISTER_PARTY_QUEST, Handle_s2c_REGISTER_PARTY_QUEST>();
        RegisterHandler<HANDLE_PKT_ID::s2c_ACQUIRE_PARTY_LIST, Nagox::Protocol::s2c_ACQUIRE_PARTY_LIST, Handle_s2c_ACQUIRE_PARTY_LIST>();
        RegisterHandler<HANDLE_PKT_ID::s2c_INVITE_PARTY_QUEST, Nagox::Protocol::s2c_INVITE_PARTY_QUEST, Handle_s2c_INVITE_PARTY_QUEST>();
        RegisterHandler<HANDLE_PKT_ID::s2c_INVITE_PARTY_RESULT, Nagox::Protocol::s2c_INVITE_PARTY_RESULT, Handle_s2c_INVITE_PARTY_RESULT>();
        RegisterHandler<HANDLE_PKT_ID::s2c_PARTY_JOIN_REQUEST, Nagox::Protocol::s2c_PARTY_JOIN_REQUEST, Handle_s2c_PARTY_JOIN_REQUEST>();
        RegisterHandler<HANDLE_PKT_ID::s2c_PARTY_JOIN_REQUEST_RESULT, Nagox::Protocol::s2c_PARTY_JOIN_REQUEST_RESULT, Handle_s2c_PARTY_JOIN_REQUEST_RESULT>();
        RegisterHandler<HANDLE_PKT_ID::s2c_PARTY_JOIN_NEW_PLAYER, Nagox::Protocol::s2c_PARTY_JOIN_NEW_PLAYER, Handle_s2c_PARTY_JOIN_NEW_PLAYER>();
        RegisterHandler<HANDLE_PKT_ID::s2c_PARTY_OUT, Nagox::Protocol::s2c_PARTY_OUT, Handle_s2c_PARTY_OUT>();
        RegisterHandler<HANDLE_PKT_ID::s2c_PARTY_QUEST_CLEAR, Nagox::Protocol::s2c_PARTY_QUEST_CLEAR, Handle_s2c_PARTY_QUEST_CLEAR>();
        RegisterHandler<HANDLE_PKT_ID::s2c_PARTY_MEMBERS_INFORMATION, Nagox::Protocol::s2c_PARTY_MEMBERS_INFORMATION, Handle_s2c_PARTY_MEMBERS_INFORMATION>();
        RegisterHandler<HANDLE_PKT_ID::s2c_GET_HARVEST, Nagox::Protocol::s2c_GET_HARVEST, Handle_s2c_GET_HARVEST>();
        for (auto& fpHandlerFunc : g_fpPacketHandler) {
            if (nullptr == fpHandlerFunc)
                fpHandlerFunc = Handle_Invalid;
        }
    }

    static inline const PacketHandlerFunc* const GetPacketHandlerList() noexcept { return g_fpPacketHandler; }

    static void AddProtocol(uint16_t pktID_, PacketHandlerFunc fpPacketHandler_) noexcept {
        g_fpPacketHandler[pktID_] = fpPacketHandler_;
    }

public:
    s2c_PacketHandler() = delete;
    s2c_PacketHandler(const s2c_PacketHandler&) = delete;
    s2c_PacketHandler(s2c_PacketHandler&&)noexcept = delete;
    s2c_PacketHandler& operator=(const s2c_PacketHandler&) = delete;
    s2c_PacketHandler& operator=(s2c_PacketHandler&&)noexcept = delete;
    ~s2c_PacketHandler() = delete;

private:
    template<HANDLE_PKT_ID packetId, typename PacketType, const bool(*const handler)(const NetHelper::S_ptr<NetHelper::PacketSession>&, const PacketType&)>
    constexpr static void RegisterHandler()noexcept {
        g_fpPacketHandler[net_etoi(packetId)] = [](const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const BYTE* const pBuff_, const int32_t len_) -> const bool {
            const uint8_t* const pkt_ptr = reinterpret_cast<const uint8_t* const>(pBuff_ + sizeof(NetHelper::PacketHeader));
            flatbuffers::Verifier verifier{ pkt_ptr, static_cast<const size_t>(len_ - static_cast<const int32_t>(sizeof(NetHelper::PacketHeader))) };
            return verifier.VerifyBuffer<PacketType>() && handler(pSession_, *flatbuffers::GetRoot<PacketType>(pkt_ptr));
        };
    }
};