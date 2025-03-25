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