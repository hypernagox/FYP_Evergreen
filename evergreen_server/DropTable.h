#pragma once
#include "ContentsComponent.h"

class DropTable
	:public ContentsComponent
{
public:
	CONSTRUCTOR_CONTENTS_COMPONENT(DropTable)
public:
	void SetItemType(const std::string_view mon_name);
	void SetItemTypeByID(const int id_) { m_itemType = (int8_t)id_; }
	int8_t GetItemType()const noexcept { return m_itemType; }
	void TryCreateItem()const noexcept;
public:
	Vector3 m_drop_offset{ 0,0,0 };
private:
	int8_t m_itemType = -1;
	// TODO: È®·ü / °¹¼ö
};

