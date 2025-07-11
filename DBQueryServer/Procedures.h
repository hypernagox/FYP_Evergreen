#pragma once
#include "pch.h"
#include "DBBindRAII.h"

class UpsertInventoryItem : public DBBindRAII<3, 0>
{
public:
	UpsertInventoryItem() noexcept
		: DBBindRAII{L"{CALL dbo.spUpsertInventoryItem(?,?,?)}"}
	{ }


	void In_CharacterUID(int64& v) { BindParam(0, v); }
	void In_CharacterUID(int64&& v)
	{
		m_characterUID = std::move(v);
		BindParam(0, m_characterUID);
	}

	void In_ItemID(int32& v) { BindParam(1, v); }
	void In_ItemID(int32&& v)
	{
		m_itemID = std::move(v);
		BindParam(1, m_itemID);
	}

	void In_AddCount(int32& v) { BindParam(2, v); }
	void In_AddCount(int32&& v)
	{
		m_addCount = std::move(v);
		BindParam(2, m_addCount);
	}
private:
	int64 m_characterUID = {};
	int32 m_itemID = {};
	int32 m_addCount = {};
};




