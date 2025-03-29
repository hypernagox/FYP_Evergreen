#pragma once
#include "ContentsComponent.h"
#include "QuickSlot.h"

class Item;

class Inventory
	:public ContentsComponent
{
public:
	CONSTRUCTOR_CONTENTS_COMPONENT(Inventory)
public:
	QuickSlot* GetQuickSlot()noexcept { return &m_quickSlot; }
	const QuickSlot* GetQuickSlot()const noexcept { return &m_quickSlot; }
public:
	const auto GetItemsAll()noexcept { return std::span<Item*>{m_items, m_items + m_curNumOfItems}; }
	const auto GetItemsAll()const noexcept { return const_cast<Inventory*>(this)->GetItemsAll(); }
public:
	const auto GetNumOfItems()const noexcept { return m_curNumOfItems; }
public:
	Item* AddItem2Inventory(Item* const item)noexcept {
		// TODO: ������ ������ ���� �ʴٸ� �ٲپ�� ��
		if (!item)return nullptr;
		if (NUM_OF_MAX_INVENTORY_ITEM >= m_curNumOfItems)return nullptr;
		m_curItemPos = FindItemPos();
		if (-1 == m_curItemPos)return nullptr;
		m_items[m_curItemPos] = item;
		++m_curNumOfItems;
		return item;
	}
private:
	const int32_t FindItemPos()const noexcept {
		for (int i = 0; i < NUM_OF_MAX_INVENTORY_ITEM; ++i) {
			if (!m_items[i])return i;
		}
		return -1;
	}
private:
	int32_t m_curNumOfItems = 0;
	int32_t m_curItemPos = 0;
	Item* m_items[NUM_OF_MAX_INVENTORY_ITEM]{ nullptr }; // �ִ� ������ ������ 30�� ������ �׳� �迭�� ���� �� ����
	QuickSlot m_quickSlot;
};

