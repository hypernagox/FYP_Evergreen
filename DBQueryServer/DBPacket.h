#pragma once
#include "pch.h"

#pragma pack (push, 1)
struct DBPacketHeader
{
    uint8_t pkt_size;
    uint8_t pkt_id;
    wchar_t name[21]{};
};
#pragma pack (pop)

void AddProtocol(const uint8 pktID_, const PacketHandlerFunc fpPacketHandler_)noexcept;

enum class DB_PKT :uint8
{
    PLAYER_MOVE,

    END
};

#pragma pack (push, 1)
template <typename T>
struct DBPacket
    :public DBPacketHeader
{
private:
    static bool InitDBPacketPacket()noexcept
    {
        T temp;
        AddProtocol(static_cast<c_uint8>(temp.pkt_id), &T::HandlePacket);
        return true;
    }
    static const inline bool g_bInitPacket = InitDBPacketPacket();

public:
    DBPacket(const DB_PKT pktID_)noexcept
        :DBPacketHeader{ sizeof(T),static_cast<c_uint8>(pktID_) }
    {
    }

    static const bool HandlePacket(BYTE* const pBuff_, c_int32 len_) {
        if (sizeof(T) != len_) [[unlikely]]
            return false;
        return T::Handle(*reinterpret_cast<T* const>(pBuff_));
    }
};
#pragma pack (pop)

#pragma pack (push, 1)

struct s2q_PLAYER_MOVE
    :public DBPacket<s2q_PLAYER_MOVE>
{
    int x, y;
    s2q_PLAYER_MOVE() :DBPacket<s2q_PLAYER_MOVE>{ DB_PKT::PLAYER_MOVE } {}
    static const bool Handle(s2q_PLAYER_MOVE& pkt_);
};

#pragma pack (pop)