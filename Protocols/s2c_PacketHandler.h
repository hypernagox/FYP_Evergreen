#pragma once
#include "../ClientNetwork/ClientNetHelper.h"
#include "enum_generated.h"
#include "struct_generated.h"
#include "protocol_generated.h"

template<typename T> requires std::is_enum_v<T>
static inline constexpr const uint16_t net_etoi(const T eType_) noexcept { return static_cast<const uint16_t>(eType_); }

enum class PKT_ID : uint16_t {
    c2s_LOGIN = 1000,
    s2c_LOGIN = 1001,
    c2s_ENTER = 1002,
    s2c_APPEAR_OBJECT = 1003,
    s2c_REMOVE_OBJECT = 1004,
    c2s_MOVE = 1005,
    s2c_MOVE = 1006,
    s2c_MONSTER_ATTACK = 1007,
    s2c_MONSTER_AGGRO_START = 1008,
    s2c_MONSTER_AGGRO_END = 1009,
    c2s_PLAYER_ATTACK = 1010,
    s2c_PLAYER_DEATH = 1011,
    c2s_PLAYER_DEATH = 1012,
};

class NetHelper::PacketSession;
class NetHelper::Session;
class NetHelper::SendBuffer;

flatbuffers::FlatBufferBuilder* const CreateBuilder()noexcept;
static inline const bool Handle_Invalid(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const BYTE* const pBuff_, const int32_t len_) noexcept { return false; }

const bool Handle_s2c_LOGIN(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_LOGIN& pkt_);
const bool Handle_s2c_APPEAR_OBJECT(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_APPEAR_OBJECT& pkt_);
const bool Handle_s2c_REMOVE_OBJECT(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_REMOVE_OBJECT& pkt_);
const bool Handle_s2c_MOVE(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_MOVE& pkt_);
const bool Handle_s2c_MONSTER_ATTACK(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_MONSTER_ATTACK& pkt_);
const bool Handle_s2c_MONSTER_AGGRO_START(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_MONSTER_AGGRO_START& pkt_);
const bool Handle_s2c_MONSTER_AGGRO_END(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_MONSTER_AGGRO_END& pkt_);
const bool Handle_s2c_PLAYER_DEATH(const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const Nagox::Protocol::s2c_PLAYER_DEATH& pkt_);

class s2c_PacketHandler {
    using PacketHandlerFunc = const bool (*)(const NetHelper::S_ptr<NetHelper::PacketSession>&, const BYTE* const, const int32_t);
    constinit static inline PacketHandlerFunc g_fpPacketHandler[UINT16_MAX] = {};
public:
    static void Init() noexcept {
        RegisterHandler<PKT_ID::s2c_LOGIN, Nagox::Protocol::s2c_LOGIN>(Handle_s2c_LOGIN);
        RegisterHandler<PKT_ID::s2c_APPEAR_OBJECT, Nagox::Protocol::s2c_APPEAR_OBJECT>(Handle_s2c_APPEAR_OBJECT);
        RegisterHandler<PKT_ID::s2c_REMOVE_OBJECT, Nagox::Protocol::s2c_REMOVE_OBJECT>(Handle_s2c_REMOVE_OBJECT);
        RegisterHandler<PKT_ID::s2c_MOVE, Nagox::Protocol::s2c_MOVE>(Handle_s2c_MOVE);
        RegisterHandler<PKT_ID::s2c_MONSTER_ATTACK, Nagox::Protocol::s2c_MONSTER_ATTACK>(Handle_s2c_MONSTER_ATTACK);
        RegisterHandler<PKT_ID::s2c_MONSTER_AGGRO_START, Nagox::Protocol::s2c_MONSTER_AGGRO_START>(Handle_s2c_MONSTER_AGGRO_START);
        RegisterHandler<PKT_ID::s2c_MONSTER_AGGRO_END, Nagox::Protocol::s2c_MONSTER_AGGRO_END>(Handle_s2c_MONSTER_AGGRO_END);
        RegisterHandler<PKT_ID::s2c_PLAYER_DEATH, Nagox::Protocol::s2c_PLAYER_DEATH>(Handle_s2c_PLAYER_DEATH);
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
    template<PKT_ID packetId, typename PacketType>
    static void RegisterHandler(const bool(*handlerFunc)(const NetHelper::S_ptr<NetHelper::PacketSession>&, const PacketType&)) {
        static const auto handler = handlerFunc;
        g_fpPacketHandler[net_etoi(packetId)] = [](const NetHelper::S_ptr<NetHelper::PacketSession>& pSession_, const BYTE* const pBuff_, const int32_t len_) -> const bool {
            const uint8_t* const pkt_ptr = reinterpret_cast<const uint8_t* const>(pBuff_ + sizeof(NetHelper::PacketHeader));
            flatbuffers::Verifier verifier{ pkt_ptr, static_cast<const size_t>(len_ - static_cast<const int32_t>(sizeof(NetHelper::PacketHeader))) };
            return verifier.VerifyBuffer<PacketType>() && handler(pSession_, *flatbuffers::GetRoot<PacketType>(pkt_ptr));
        };
    }
};