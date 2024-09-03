#pragma once
#include "ServerCorePch.h"

#pragma pack (push, 1)

struct DBPacketHeader
{
    uint8_t pkt_size;
    uint8_t pkt_id;
    uint64_t user_id;
};

#pragma pack (pop)

enum class DB_PKT :uint8
{
    LOGIN,

    PLAYER_MOVE,
    UPDATE_PLAYER_INFO,
    ADD_OR_UPDATE_ITEM,
    CONSUME_ITEM,




    END
};

#pragma pack (push, 1)
template <typename T>
struct DBPacket
    :public DBPacketHeader
{
public:
    DBPacket(const DB_PKT pktID_)noexcept
        :DBPacketHeader{ sizeof(T),static_cast<c_uint8>(pktID_) }
    {
    }

    bool SerializeToArray(void* const __restrict buffer, const size_t dataSize)const noexcept {
        if (sizeof(T) != dataSize) [[unlikely]]
            return false;
        return ::memcpy(buffer, this, dataSize);
    }

    ServerCore::S_ptr<ServerCore::SendBuffer> MakeSendBuffer()const noexcept
    {
        const uint8_t packetSize = sizeof(T);

        ServerCore::S_ptr<ServerCore::SendBuffer> sendBuffer = Mgr(SendBufferMgr)->Open(packetSize);
        NAGOX_ASSERT(SerializeToArray(sendBuffer->Buffer(), packetSize));
        sendBuffer->Close(packetSize);

        return sendBuffer;
    }
};
#pragma pack (pop)

#pragma pack (push, 1)

struct s2q_LOGIN : public DBPacket<s2q_LOGIN>
{
    char userName[32]{};

    s2q_LOGIN() : DBPacket<s2q_LOGIN>{ DB_PKT::LOGIN } {}

  //  static const bool Handle(s2q_LOGIN& pkt_);
};

struct s2q_PLAYER_MOVE : public DBPacket<s2q_PLAYER_MOVE> {
    int x, y;

    s2q_PLAYER_MOVE() : DBPacket<s2q_PLAYER_MOVE>{ DB_PKT::PLAYER_MOVE } {}

    //static const bool Handle(s2q_PLAYER_MOVE& pkt_);
};

struct s2q_UPDATE_PLAYER_INFO : public DBPacket<s2q_UPDATE_PLAYER_INFO>
{
    int level{};
    int  experience{};
    int  gold{};
    int  hp{};

    s2q_UPDATE_PLAYER_INFO() : DBPacket<s2q_UPDATE_PLAYER_INFO>{ DB_PKT::UPDATE_PLAYER_INFO } {}

   // static const bool Handle(s2q_UPDATE_PLAYER_INFO& pkt_);
};

struct s2q_ADD_OR_UPDATE_ITEM : public DBPacket<s2q_ADD_OR_UPDATE_ITEM> {
    char itemName[32]{};
    int quantity = 1;

    s2q_ADD_OR_UPDATE_ITEM() : DBPacket<s2q_ADD_OR_UPDATE_ITEM>{ DB_PKT::ADD_OR_UPDATE_ITEM } {}

    //static const bool Handle(s2q_ADD_OR_UPDATE_ITEM& pkt_);
};

struct s2q_CONSUME_ITEM : public DBPacket<s2q_CONSUME_ITEM> {
    char itemName[32]{};
    int quantity;

    s2q_CONSUME_ITEM() : DBPacket<s2q_CONSUME_ITEM>{ DB_PKT::CONSUME_ITEM } {}

    //static const bool Handle(s2q_CONSUME_ITEM& pkt_);

};
#pragma pack (pop)