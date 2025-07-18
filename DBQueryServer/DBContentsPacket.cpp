#include "pch.h"
#include "../DBContentsPacket.hpp"
#include "s2q_PacketHandler.h"
#include "Procedures.h"

void AddProtocol(const uint16 pktID_, void (*fpPacketHandler_)(const char* const)noexcept)noexcept
{
	s2q_PacketHandler::AddProtocol(pktID_, fpPacketHandler_);
}

#define DECLARE_PACKET_FUNC(pkt_name) void pkt_name::Handle(pkt_name& pkt_) noexcept

DECLARE_PACKET_FUNC(s2q_ADD_OR_UPDATE_ITEM)
{
	std::cout << "s2q_ADD_OR_UPDATE_ITEM\n";
	{
		UpsertInventoryItem up;
		std::cout <<"UID: " <<pkt_.pkt_db_uid << std::endl;
		std::cout <<"Item ID: " <<pkt_.item_id << std::endl;
		std::cout <<"Count: " <<pkt_.item_count << std::endl;

		up.In_CharacterUID(pkt_.pkt_db_uid);
		up.In_ItemID(pkt_.item_id);
		up.In_AddCount(pkt_.item_count);
		up.Execute();
	}
}

DECLARE_PACKET_FUNC(s2q_SWAP_EQUIPMENT)
{
	std::cout << "s2q_SWAP_EQUIPMENT\n";
	{
		// 장비 종류
		// 0이면 무기 1이면 방어구로
		if (0 == pkt_.equip_type)
		{
			EquipWeapon e;
			e.In_CharacterUID(pkt_.pkt_db_uid);
			e.In_ItemID(pkt_.equip_id);
			e.Execute();
		}
		else if (1 == pkt_.equip_type)
		{
			EquipArmor e;
			e.In_CharacterUID(pkt_.pkt_db_uid);
			e.In_ItemID(pkt_.equip_id);
			e.Execute();
		}
		std::cout << "UID: " << pkt_.pkt_db_uid << std::endl;
		std::cout << "Equip ID: " << pkt_.equip_id << std::endl;
	}
}