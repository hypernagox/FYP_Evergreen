#pragma once
#include "ContentsComponent.h"
#include "QuickSlot.h"
#include "EquipmentSystem.h"

class Item;
class DropItem;

/// <summary>
/// //////////////////////////////////////////////////////////
/// 
/// 
/// 
/// 
/// 주주주주주주주주주주주주주주주주주주주주주주주주주주주주주주주주주주주주주주주주주주주주주주주주주주주주주주주주주주주
/// 의의의의의의의의의의의의의의의의의의의의의의의의의의의의의의의의의의의의의의의의의의의의의의의의의의의의의의의의의의의
/// </summary>
/// 
/// 컨텐츠 살붙으면 무조건 데이터레이스

class Inventory
	:public ContentsComponent
{
public:
	CONSTRUCTOR_CONTENTS_COMPONENT(Inventory)
public:
	
	~Inventory();
	QuickSlot* GetQuickSlot()noexcept { return &m_quickSlot; }
	const QuickSlot* GetQuickSlot()const noexcept { return &m_quickSlot; }
public:
	const auto GetItemsAll()noexcept { return std::span<Item*>{m_items, m_items + m_curNumOfItems}; }
	const auto GetItemsAll()const noexcept { return const_cast<Inventory*>(this)->GetItemsAll(); }
public:
	bool CraftItem(const ItemRecipeData& recipe_info)noexcept;
public:
	const auto GetNumOfItems()const noexcept { return m_curNumOfItems; }
	void DecItemStack(const int8_t item_type, const int cnt)noexcept;
public:
	Item* AddDropItem(const DropItem* const drop_item_info)noexcept;
	Item* AddItem(const int item_id, const int stack_size)noexcept;
	bool SetQuickSlotItem(const uint8_t item_id, const uint8_t quick_idx)noexcept;
	
	// 퀵슬롯 아이템 사용에 성공했다면 해당 아이템의 ID를, 실패했다면 -1를 반환한다.
	int8_t UseQuickSlotItem(const uint8_t quick_idx)noexcept { return m_quickSlot.UseSlotItem(GetOwnerEntityRaw(), quick_idx); }
	const auto GetEquipmentSystem()const noexcept { return &m_equipSystem; }
	const auto GetEquipmentSystem()noexcept { return &m_equipSystem; }
private:
	Item* FindItem(const int8_t item_type)const noexcept;
	int8_t FindItemIndex(const int8_t item_type)const noexcept;
	const int32_t FindItemPos()const noexcept {
		for (int i = 0; i < NUM_OF_MAX_INVENTORY_ITEM; ++i) {
			if (!m_items[i])return i;
		}
		return -1;
	}
private:
	int32_t m_curNumOfItems = 0;
	int32_t m_curItemPos = 0;
	Item* m_items[NUM_OF_MAX_INVENTORY_ITEM]{ nullptr }; // 최대 아이템 제한이 30개 정도면 그냥 배열이 빠를 것 같다
	QuickSlot m_quickSlot;
	EquipmentSystem m_equipSystem;
private:
	Item* AddItem2Inventory(Item* const item)noexcept {
		// TODO: 스레드 세이프 하지 않다면 바꾸어야 함
		if (!item)return nullptr;
		if (NUM_OF_MAX_INVENTORY_ITEM <= m_curNumOfItems)return nullptr;
		m_curItemPos = FindItemPos();
		if (-1 == m_curItemPos)return nullptr;
		m_items[m_curItemPos] = item;
		++m_curNumOfItems;
		return item;
	}
};


