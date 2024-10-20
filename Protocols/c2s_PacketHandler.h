#pragma once
#include "../ServerCore/ServerCorePch.h"
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
};

class ServerCore::PacketSession;
class ServerCore::Session;
class ServerCore::SendBuffer;

flatbuffers::FlatBufferBuilder* const CreateBuilder()noexcept;
static inline const bool Handle_Invalid(const ServerCore::S_ptr<ServerCore::PacketSession>& pSession_, const BYTE* const pBuff_, const int32_t len_) noexcept { return false; }

const bool Handle_c2s_LOGIN(const ServerCore::S_ptr<ServerCore::PacketSession>& pSession_, const Nagox::Protocol::c2s_LOGIN& pkt_);
const bool Handle_c2s_ENTER(const ServerCore::S_ptr<ServerCore::PacketSession>& pSession_, const Nagox::Protocol::c2s_ENTER& pkt_);
const bool Handle_c2s_MOVE(const ServerCore::S_ptr<ServerCore::PacketSession>& pSession_, const Nagox::Protocol::c2s_MOVE& pkt_);
const bool Handle_c2s_PLAYER_ATTACK(const ServerCore::S_ptr<ServerCore::PacketSession>& pSession_, const Nagox::Protocol::c2s_PLAYER_ATTACK& pkt_);

class c2s_PacketHandler {
    using PacketHandlerFunc = const bool (*)(const ServerCore::S_ptr<ServerCore::PacketSession>&, const BYTE* const, const int32_t);
    constinit static inline PacketHandlerFunc g_fpPacketHandler[UINT16_MAX] = {};
public:
    static void Init() noexcept {
        RegisterHandler<PKT_ID::c2s_LOGIN, Nagox::Protocol::c2s_LOGIN>(Handle_c2s_LOGIN);
        RegisterHandler<PKT_ID::c2s_ENTER, Nagox::Protocol::c2s_ENTER>(Handle_c2s_ENTER);
        RegisterHandler<PKT_ID::c2s_MOVE, Nagox::Protocol::c2s_MOVE>(Handle_c2s_MOVE);
        RegisterHandler<PKT_ID::c2s_PLAYER_ATTACK, Nagox::Protocol::c2s_PLAYER_ATTACK>(Handle_c2s_PLAYER_ATTACK);
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
    template<PKT_ID packetId, typename PacketType>
    static void RegisterHandler(const bool(*handlerFunc)(const ServerCore::S_ptr<ServerCore::PacketSession>&, const PacketType&)) {
        static const auto handler = handlerFunc;
        g_fpPacketHandler[net_etoi(packetId)] = [](const ServerCore::S_ptr<ServerCore::PacketSession>& pSession_, const BYTE* const pBuff_, const int32_t len_) -> const bool {
            const uint8_t* const pkt_ptr = reinterpret_cast<const uint8_t* const>(pBuff_ + sizeof(ServerCore::PacketHeader));
            flatbuffers::Verifier verifier{ pkt_ptr, static_cast<const size_t>(len_ - static_cast<const int32_t>(sizeof(ServerCore::PacketHeader))) };
            return verifier.VerifyBuffer<PacketType>() && handler(pSession_, *flatbuffers::GetRoot<PacketType>(pkt_ptr));
        };
    }
};