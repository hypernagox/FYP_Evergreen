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

private:
	Item* m_items[NUM_OF_MAX_INVENTORY_ITEM]{ nullptr }; // 최대 아이템 제한이 30개 정도면 그냥 배열이 빠를 것 같다
	QuickSlot m_quickSlot;
};

