#pragma once
#include "pch.h"
#include "DBPacketDefine.h"

using uint64 = unsigned long long;
using uint8 = unsigned char;
using uint16 = unsigned short;
using uint32 = unsigned int;

extern void AddProtocol(const uint16 pktID_, void (*fpPacketHandler_)(const char* const)noexcept)noexcept;


#pragma pack (push, 1)

struct DBContentsPacketHeader
{
    uint16_t pkt_size;
    uint16_t pkt_id;
    uint64_t pkt_db_uid;
};

// 패킷 구조체 이름과 동일하게 만들어야 편하다
enum class PKT_ID : uint16
{
    s2q_ADD_OR_UPDATE_ITEM,

    END,
};

template <typename T>
static constexpr inline unsigned char etoi(const T eType)noexcept { return static_cast<uint8>(eType); }

#pragma pack (pop)

#define DECLARE_PACKET(pkt_name)                                                                      \
    private:                                                                                          \
       static void HandlePacket(const char* const pBuff_)noexcept {                                   \
        return pkt_name::Handle(*reinterpret_cast<pkt_name* const>((pkt_name*)pBuff_));               \
    }                                                                                                 \
    static bool RegisterHandleFunc() noexcept                                                         \
    {                                                                                                 \
        if constexpr ((#pkt_name[0] == 's' && G_PROJECT == PROJECT_TYPE::QUERY_SERVER))               \
        {                                                                                             \
            pkt_name temp = {};                                                                       \
            AddProtocol(static_cast<unsigned short>(temp.pkt_id), &pkt_name::HandlePacket);           \
        }                                                                                             \
        return true;                                                                                  \
    }                                                                                                 \
   static const inline bool g_bInitPacket = RegisterHandleFunc();                                     \
   static void Handle(pkt_name& pkt_) noexcept;                                                       \
   public:                                                                                            \
   pkt_name()noexcept:DBContentsPacketHeader{(uint16_t)sizeof(pkt_name), (uint16_t)PKT_ID::pkt_name, 0}{}


#pragma pack (push, 1)

struct s2q_ADD_OR_UPDATE_ITEM
    :public DBContentsPacketHeader
{
    int item_id;
    int item_count;
    DECLARE_PACKET(s2q_ADD_OR_UPDATE_ITEM);

};

#pragma pack (pop)