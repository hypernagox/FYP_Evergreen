#pragma once
#include "../NagiocpX/NagiocpXPch.h"
#include "enum_generated.h"
#include "struct_generated.h"
#include "protocol_generated.h"

template<typename T> requires std::is_enum_v<T>
static inline consteval const uint16_t net_etoi(const T eType_) noexcept { return static_cast<const uint16_t>(eType_); }

enum class HANDLE_PKT_ID : uint16_t {
    c2s_LOGIN = 1000,
    c2s_REGISTER_ACCOUNT = 1001,
    c2s_PING_PONG = 1002,
    c2s_ENTER = 1003,
    c2s_MOVE = 1004,
    c2s_PLAYER_ATTACK = 1005,
    c2s_PLAYER_DEATH = 1006,
    c2s_REQUEST_QUEST = 1007,
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
    c2s_CHANGE_HARVEST_STATE = 1021,
    c2s_CHANGE_EQUIPMENT = 1022,
    c2s_CHANGE_CHANNEL = 1023,
    c2s_DASH = 1024,
    c2s_CHAT = 1025,
    c2s_SHOOT_CATAPULT = 1026,
};

enum class CREATE_PKT_ID : uint16_t {
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
    s2c_PROCESS_COMMON_QUEST = 1012,
    s2c_CLEAR_QUEST = 1013,
    s2c_FIRE_PROJ = 1014,
    s2c_ACQUIRE_ITEM = 1015,
    s2c_USE_QUICK_SLOT_ITEM = 1016,
    s2c_CRAFT_ITEM = 1017,
    s2c_REGISTER_PARTY_QUEST = 1018,
    s2c_ACQUIRE_PARTY_LIST = 1019,
    s2c_INVITE_PARTY_QUEST = 1020,
    s2c_INVITE_PARTY_RESULT = 1021,
    s2c_PARTY_JOIN_REQUEST = 1022,
    s2c_PARTY_JOIN_REQUEST_RESULT = 1023,
    s2c_PARTY_JOIN_NEW_PLAYER = 1024,
    s2c_QUEST_END = 1025,
    s2c_PARTY_QUEST_START = 1026,
    s2c_PARTY_OUT = 1027,
    s2c_PARTY_QUEST_CLEAR = 1028,
    s2c_PARTY_MEMBERS_INFORMATION = 1029,
    s2c_CHANGE_HARVEST_STATE = 1030,
    s2c_NOTIFY_USER_DETAIL_INFO = 1031,
    s2c_FORCED_MOVE = 1032,
    s2c_BOSS_ROOM_ENTER = 1033,
    s2c_BOSS_FLY = 1034,
    s2c_BOSS_MOVE = 1035,
    s2c_BOSS_PROJ_MARK = 1036,
    s2c_HEAL = 1037,
    s2c_DASH = 1038,
    s2c_ARROW_RAIN = 1039,
    s2c_CHAT = 1040,
    s2c_SHOOT_CATAPULT = 1041,
    s2c_NOTIFY_CATAPULT = 1042,
    s2c_BOSS_CHANGE_PHASE = 1043,
    s2c_BOSS_READY_TO_BREATH = 1044,
};

class NagiocpX::PacketSession;
class NagiocpX::Session;
class NagiocpX::SendBuffer;

flatbuffers::FlatBufferBuilder* const CreateBuilder()noexcept;
static inline const bool Handle_Invalid(const NagiocpX::S_ptr<NagiocpX::PacketSession>& pSession_, const BYTE* const pBuff_, const int32_t len_) noexcept { return false; }

const bool Handle_c2s_LOGIN(const NagiocpX::S_ptr<NagiocpX::PacketSession>& pSession_, const Nagox::Protocol::c2s_LOGIN& pkt_);
const bool Handle_c2s_REGISTER_ACCOUNT(const NagiocpX::S_ptr<NagiocpX::PacketSession>& pSession_, const Nagox::Protocol::c2s_REGISTER_ACCOUNT& pkt_);
const bool Handle_c2s_PING_PONG(const NagiocpX::S_ptr<NagiocpX::PacketSession>& pSession_, const Nagox::Protocol::c2s_PING_PONG& pkt_);
const bool Handle_c2s_ENTER(const NagiocpX::S_ptr<NagiocpX::PacketSession>& pSession_, const Nagox::Protocol::c2s_ENTER& pkt_);
const bool Handle_c2s_MOVE(const NagiocpX::S_ptr<NagiocpX::PacketSession>& pSession_, const Nagox::Protocol::c2s_MOVE& pkt_);
const bool Handle_c2s_PLAYER_ATTACK(const NagiocpX::S_ptr<NagiocpX::PacketSession>& pSession_, const Nagox::Protocol::c2s_PLAYER_ATTACK& pkt_);
const bool Handle_c2s_PLAYER_DEATH(const NagiocpX::S_ptr<NagiocpX::PacketSession>& pSession_, const Nagox::Protocol::c2s_PLAYER_DEATH& pkt_);
const bool Handle_c2s_REQUEST_QUEST(const NagiocpX::S_ptr<NagiocpX::PacketSession>& pSession_, const Nagox::Protocol::c2s_REQUEST_QUEST& pkt_);
const bool Handle_c2s_ACQUIRE_ITEM(const NagiocpX::S_ptr<NagiocpX::PacketSession>& pSession_, const Nagox::Protocol::c2s_ACQUIRE_ITEM& pkt_);
const bool Handle_c2s_REQUEST_QUICK_SLOT(const NagiocpX::S_ptr<NagiocpX::PacketSession>& pSession_, const Nagox::Protocol::c2s_REQUEST_QUICK_SLOT& pkt_);
const bool Handle_c2s_USE_QUICK_SLOT_ITEM(const NagiocpX::S_ptr<NagiocpX::PacketSession>& pSession_, const Nagox::Protocol::c2s_USE_QUICK_SLOT_ITEM& pkt_);
const bool Handle_c2s_CRAFT_ITEM(const NagiocpX::S_ptr<NagiocpX::PacketSession>& pSession_, const Nagox::Protocol::c2s_CRAFT_ITEM& pkt_);
const bool Handle_c2s_REGISTER_PARTY_QUEST(const NagiocpX::S_ptr<NagiocpX::PacketSession>& pSession_, const Nagox::Protocol::c2s_REGISTER_PARTY_QUEST& pkt_);
const bool Handle_c2s_ACQUIRE_PARTY_LIST(const NagiocpX::S_ptr<NagiocpX::PacketSession>& pSession_, const Nagox::Protocol::c2s_ACQUIRE_PARTY_LIST& pkt_);
const bool Handle_c2s_INVITE_PARTY_QUEST(const NagiocpX::S_ptr<NagiocpX::PacketSession>& pSession_, const Nagox::Protocol::c2s_INVITE_PARTY_QUEST& pkt_);
const bool Handle_c2s_INVITE_PARTY_RESULT(const NagiocpX::S_ptr<NagiocpX::PacketSession>& pSession_, const Nagox::Protocol::c2s_INVITE_PARTY_RESULT& pkt_);
const bool Handle_c2s_PARTY_JOIN_REQUEST(const NagiocpX::S_ptr<NagiocpX::PacketSession>& pSession_, const Nagox::Protocol::c2s_PARTY_JOIN_REQUEST& pkt_);
const bool Handle_c2s_PARTY_JOIN_REQUEST_RESULT(const NagiocpX::S_ptr<NagiocpX::PacketSession>& pSession_, const Nagox::Protocol::c2s_PARTY_JOIN_REQUEST_RESULT& pkt_);
const bool Handle_c2s_QUEST_START(const NagiocpX::S_ptr<NagiocpX::PacketSession>& pSession_, const Nagox::Protocol::c2s_QUEST_START& pkt_);
const bool Handle_c2s_QUEST_END(const NagiocpX::S_ptr<NagiocpX::PacketSession>& pSession_, const Nagox::Protocol::c2s_QUEST_END& pkt_);
const bool Handle_c2s_PARTY_OUT(const NagiocpX::S_ptr<NagiocpX::PacketSession>& pSession_, const Nagox::Protocol::c2s_PARTY_OUT& pkt_);
const bool Handle_c2s_CHANGE_HARVEST_STATE(const NagiocpX::S_ptr<NagiocpX::PacketSession>& pSession_, const Nagox::Protocol::c2s_CHANGE_HARVEST_STATE& pkt_);
const bool Handle_c2s_CHANGE_EQUIPMENT(const NagiocpX::S_ptr<NagiocpX::PacketSession>& pSession_, const Nagox::Protocol::c2s_CHANGE_EQUIPMENT& pkt_);
const bool Handle_c2s_CHANGE_CHANNEL(const NagiocpX::S_ptr<NagiocpX::PacketSession>& pSession_, const Nagox::Protocol::c2s_CHANGE_CHANNEL& pkt_);
const bool Handle_c2s_DASH(const NagiocpX::S_ptr<NagiocpX::PacketSession>& pSession_, const Nagox::Protocol::c2s_DASH& pkt_);
const bool Handle_c2s_CHAT(const NagiocpX::S_ptr<NagiocpX::PacketSession>& pSession_, const Nagox::Protocol::c2s_CHAT& pkt_);
const bool Handle_c2s_SHOOT_CATAPULT(const NagiocpX::S_ptr<NagiocpX::PacketSession>& pSession_, const Nagox::Protocol::c2s_SHOOT_CATAPULT& pkt_);

class c2s_PacketHandler {
    using PacketHandlerFunc = const bool (*)(const NagiocpX::S_ptr<NagiocpX::PacketSession>&, const BYTE* const, const int32_t);
    constinit static inline PacketHandlerFunc g_fpPacketHandler[UINT16_MAX] = {};
public:
    static void Init() noexcept {
        RegisterHandler<HANDLE_PKT_ID::c2s_LOGIN, Nagox::Protocol::c2s_LOGIN, Handle_c2s_LOGIN>();
        RegisterHandler<HANDLE_PKT_ID::c2s_REGISTER_ACCOUNT, Nagox::Protocol::c2s_REGISTER_ACCOUNT, Handle_c2s_REGISTER_ACCOUNT>();
        RegisterHandler<HANDLE_PKT_ID::c2s_PING_PONG, Nagox::Protocol::c2s_PING_PONG, Handle_c2s_PING_PONG>();
        RegisterHandler<HANDLE_PKT_ID::c2s_ENTER, Nagox::Protocol::c2s_ENTER, Handle_c2s_ENTER>();
        RegisterHandler<HANDLE_PKT_ID::c2s_MOVE, Nagox::Protocol::c2s_MOVE, Handle_c2s_MOVE>();
        RegisterHandler<HANDLE_PKT_ID::c2s_PLAYER_ATTACK, Nagox::Protocol::c2s_PLAYER_ATTACK, Handle_c2s_PLAYER_ATTACK>();
        RegisterHandler<HANDLE_PKT_ID::c2s_PLAYER_DEATH, Nagox::Protocol::c2s_PLAYER_DEATH, Handle_c2s_PLAYER_DEATH>();
        RegisterHandler<HANDLE_PKT_ID::c2s_REQUEST_QUEST, Nagox::Protocol::c2s_REQUEST_QUEST, Handle_c2s_REQUEST_QUEST>();
        RegisterHandler<HANDLE_PKT_ID::c2s_ACQUIRE_ITEM, Nagox::Protocol::c2s_ACQUIRE_ITEM, Handle_c2s_ACQUIRE_ITEM>();
        RegisterHandler<HANDLE_PKT_ID::c2s_REQUEST_QUICK_SLOT, Nagox::Protocol::c2s_REQUEST_QUICK_SLOT, Handle_c2s_REQUEST_QUICK_SLOT>();
        RegisterHandler<HANDLE_PKT_ID::c2s_USE_QUICK_SLOT_ITEM, Nagox::Protocol::c2s_USE_QUICK_SLOT_ITEM, Handle_c2s_USE_QUICK_SLOT_ITEM>();
        RegisterHandler<HANDLE_PKT_ID::c2s_CRAFT_ITEM, Nagox::Protocol::c2s_CRAFT_ITEM, Handle_c2s_CRAFT_ITEM>();
        RegisterHandler<HANDLE_PKT_ID::c2s_REGISTER_PARTY_QUEST, Nagox::Protocol::c2s_REGISTER_PARTY_QUEST, Handle_c2s_REGISTER_PARTY_QUEST>();
        RegisterHandler<HANDLE_PKT_ID::c2s_ACQUIRE_PARTY_LIST, Nagox::Protocol::c2s_ACQUIRE_PARTY_LIST, Handle_c2s_ACQUIRE_PARTY_LIST>();
        RegisterHandler<HANDLE_PKT_ID::c2s_INVITE_PARTY_QUEST, Nagox::Protocol::c2s_INVITE_PARTY_QUEST, Handle_c2s_INVITE_PARTY_QUEST>();
        RegisterHandler<HANDLE_PKT_ID::c2s_INVITE_PARTY_RESULT, Nagox::Protocol::c2s_INVITE_PARTY_RESULT, Handle_c2s_INVITE_PARTY_RESULT>();
        RegisterHandler<HANDLE_PKT_ID::c2s_PARTY_JOIN_REQUEST, Nagox::Protocol::c2s_PARTY_JOIN_REQUEST, Handle_c2s_PARTY_JOIN_REQUEST>();
        RegisterHandler<HANDLE_PKT_ID::c2s_PARTY_JOIN_REQUEST_RESULT, Nagox::Protocol::c2s_PARTY_JOIN_REQUEST_RESULT, Handle_c2s_PARTY_JOIN_REQUEST_RESULT>();
        RegisterHandler<HANDLE_PKT_ID::c2s_QUEST_START, Nagox::Protocol::c2s_QUEST_START, Handle_c2s_QUEST_START>();
        RegisterHandler<HANDLE_PKT_ID::c2s_QUEST_END, Nagox::Protocol::c2s_QUEST_END, Handle_c2s_QUEST_END>();
        RegisterHandler<HANDLE_PKT_ID::c2s_PARTY_OUT, Nagox::Protocol::c2s_PARTY_OUT, Handle_c2s_PARTY_OUT>();
        RegisterHandler<HANDLE_PKT_ID::c2s_CHANGE_HARVEST_STATE, Nagox::Protocol::c2s_CHANGE_HARVEST_STATE, Handle_c2s_CHANGE_HARVEST_STATE>();
        RegisterHandler<HANDLE_PKT_ID::c2s_CHANGE_EQUIPMENT, Nagox::Protocol::c2s_CHANGE_EQUIPMENT, Handle_c2s_CHANGE_EQUIPMENT>();
        RegisterHandler<HANDLE_PKT_ID::c2s_CHANGE_CHANNEL, Nagox::Protocol::c2s_CHANGE_CHANNEL, Handle_c2s_CHANGE_CHANNEL>();
        RegisterHandler<HANDLE_PKT_ID::c2s_DASH, Nagox::Protocol::c2s_DASH, Handle_c2s_DASH>();
        RegisterHandler<HANDLE_PKT_ID::c2s_CHAT, Nagox::Protocol::c2s_CHAT, Handle_c2s_CHAT>();
        RegisterHandler<HANDLE_PKT_ID::c2s_SHOOT_CATAPULT, Nagox::Protocol::c2s_SHOOT_CATAPULT, Handle_c2s_SHOOT_CATAPULT>();
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
    c2s_PacketHandler() = delete;
    c2s_PacketHandler(const c2s_PacketHandler&) = delete;
    c2s_PacketHandler(c2s_PacketHandler&&)noexcept = delete;
    c2s_PacketHandler& operator=(const c2s_PacketHandler&) = delete;
    c2s_PacketHandler& operator=(c2s_PacketHandler&&)noexcept = delete;
    ~c2s_PacketHandler() = delete;

private:
    template<HANDLE_PKT_ID packetId, typename PacketType, const bool(*const handler)(const NagiocpX::S_ptr<NagiocpX::PacketSession>&, const PacketType&)>
    constexpr static void RegisterHandler()noexcept {
        g_fpPacketHandler[net_etoi(packetId)] = [](const NagiocpX::S_ptr<NagiocpX::PacketSession>& pSession_, const BYTE* const pBuff_, const int32_t len_) -> const bool {
            const uint8_t* const pkt_ptr = reinterpret_cast<const uint8_t* const>(pBuff_ + sizeof(NagiocpX::PacketHeader));
            flatbuffers::Verifier verifier{ pkt_ptr, static_cast<const size_t>(len_ - static_cast<const int32_t>(sizeof(NagiocpX::PacketHeader))) };
            return verifier.VerifyBuffer<PacketType>() && handler(pSession_, *flatbuffers::GetRoot<PacketType>(pkt_ptr));
        };
    }
};