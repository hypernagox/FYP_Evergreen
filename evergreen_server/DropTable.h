#pragma once
#include "ContentsComponent.h"

class DropTable
	:public ContentsComponent
{
public:
	CONSTRUCTOR_CONTENTS_COMPONENT(DropTable)
public:
	void SetItemType(const std::string_view mon_name);
	int8_t GetItemType()const noexcept { return m_itemType; }
	void TryCreateItem()const noexcept;
private:
	int8_t m_itemType = -1;
	// TODO: È®·ü
};

