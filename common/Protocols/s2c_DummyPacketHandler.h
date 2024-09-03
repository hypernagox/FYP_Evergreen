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
};

class ServerCore::PacketSession;
class ServerCore::Session;
class ServerCore::SendBuffer;

flatbuffers::FlatBufferBuilder* const CreateBuilder()noexcept;
static inline const bool Handle_Invalid(const ServerCore::S_ptr<ServerCore::PacketSession>& pSession_, const BYTE* const pBuff_, const int32_t len_) noexcept { return false; }

const bool Handle_s2c_LOGIN(const ServerCore::S_ptr<ServerCore::PacketSession>& pSession_, const Nagox::Protocol::s2c_LOGIN& pkt_);
const bool Handle_s2c_APPEAR_OBJECT(const ServerCore::S_ptr<ServerCore::PacketSession>& pSession_, const Nagox::Protocol::s2c_APPEAR_OBJECT& pkt_);
const bool Handle_s2c_REMOVE_OBJECT(const ServerCore::S_ptr<ServerCore::PacketSession>& pSession_, const Nagox::Protocol::s2c_REMOVE_OBJECT& pkt_);
const bool Handle_s2c_MOVE(const ServerCore::S_ptr<ServerCore::PacketSession>& pSession_, const Nagox::Protocol::s2c_MOVE& pkt_);

class s2c_DummyPacketHandler {
    using PacketHandlerFunc = const bool (*)(const ServerCore::S_ptr<ServerCore::PacketSession>&, const BYTE* const, const int32_t);
    static inline PacketHandlerFunc g_fpPacketHandler[UINT16_MAX] = {};
public:
    static void Init() noexcept {
        RegisterHandler<PKT_ID::s2c_LOGIN, Nagox::Protocol::s2c_LOGIN>(Handle_s2c_LOGIN);
        RegisterHandler<PKT_ID::s2c_APPEAR_OBJECT, Nagox::Protocol::s2c_APPEAR_OBJECT>(Handle_s2c_APPEAR_OBJECT);
        RegisterHandler<PKT_ID::s2c_REMOVE_OBJECT, Nagox::Protocol::s2c_REMOVE_OBJECT>(Handle_s2c_REMOVE_OBJECT);
        RegisterHandler<PKT_ID::s2c_MOVE, Nagox::Protocol::s2c_MOVE>(Handle_s2c_MOVE);
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
    s2c_DummyPacketHandler() = delete;
    s2c_DummyPacketHandler(const s2c_DummyPacketHandler&) = delete;
    s2c_DummyPacketHandler(s2c_DummyPacketHandler&&)noexcept = delete;
    s2c_DummyPacketHandler& operator=(const s2c_DummyPacketHandler&) = delete;
    s2c_DummyPacketHandler& operator=(s2c_DummyPacketHandler&&)noexcept = delete;
    ~s2c_DummyPacketHandler() = delete;

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