class InsertUser : public DBBindRAII<4, 0>
{
public:
	InsertUser() noexcept
		: DBBindRAII{L"{CALL dbo.spInsertUser(?,?,?,?)}"}
	{ }


	template<int32 N> void In_Id(WCHAR(&v)[N]) { BindParam(0, v); }
	template<int32 N> void In_Id(const WCHAR(&v)[N]) { BindParam(0, v); }
	void In_Id(WCHAR* const v, const int32_t count) { BindParam(0, v, count); }
	void In_Id(const WCHAR* const v, const int32_t count) { BindParam(0, v, count); }


	template<int32 N> void In_Pw(WCHAR(&v)[N]) { BindParam(1, v); }
	template<int32 N> void In_Pw(const WCHAR(&v)[N]) { BindParam(1, v); }
	void In_Pw(WCHAR* const v, const int32_t count) { BindParam(1, v, count); }
	void In_Pw(const WCHAR* const v, const int32_t count) { BindParam(1, v, count); }


	template<int32 N> void In_Type(WCHAR(&v)[N]) { BindParam(2, v); }
	template<int32 N> void In_Type(const WCHAR(&v)[N]) { BindParam(2, v); }
	void In_Type(WCHAR* const v, const int32_t count) { BindParam(2, v, count); }
	void In_Type(const WCHAR* const v, const int32_t count) { BindParam(2, v, count); }


	void In_UID(int32& v) { BindParam(3, v); }
	void In_UID(int32&& v)
	{
		m_uID = std::move(v);
		BindParam(3, m_uID);
	}
private:
	int32 m_uID = {};
};

class GetUserById : public DBBindRAII<1, 4>
{
public:
	GetUserById() noexcept
		: DBBindRAII{L"{CALL dbo.spGetUserById(?)}"}
	{ }


	template<int32 N> void In_Id(WCHAR(&v)[N]) { BindParam(0, v); }
	template<int32 N> void In_Id(const WCHAR(&v)[N]) { BindParam(0, v); }
	void In_Id(WCHAR* const v, const int32_t count) { BindParam(0, v, count); }
	void In_Id(const WCHAR* const v, const int32_t count) { BindParam(0, v, count); }

	template<int32_t N> void Out_Id(OUT WCHAR(&v)[N]) { BindCol(0, v); }
	template<int32_t N> void Out_Pw(OUT WCHAR(&v)[N]) { BindCol(1, v); }
	template<int32_t N> void Out_Type(OUT WCHAR(&v)[N]) { BindCol(2, v); }
	void Out_UID(OUT int32& v) { BindCol(3, v); }
private:
};

class InsertInventoryItem : public DBBindRAII<5, 0>
{
public:
	InsertInventoryItem() noexcept
		: DBBindRAII{L"{CALL dbo.spInsertInventoryItem(?,?,?,?,?)}"}
	{ }


	void In_InventoryItemID(int32& v) { BindParam(0, v); }
	void In_InventoryItemID(int32&& v)
	{
		m_inventoryItemID = std::move(v);
		BindParam(0, m_inventoryItemID);
	}

	template<int32 N> void In_CharacterID(WCHAR(&v)[N]) { BindParam(1, v); }
	template<int32 N> void In_CharacterID(const WCHAR(&v)[N]) { BindParam(1, v); }
	void In_CharacterID(WCHAR* const v, const int32_t count) { BindParam(1, v, count); }
	void In_CharacterID(const WCHAR* const v, const int32_t count) { BindParam(1, v, count); }


	void In_ItemID(int32& v) { BindParam(2, v); }
	void In_ItemID(int32&& v)
	{
		m_itemID = std::move(v);
		BindParam(2, m_itemID);
	}

	void In_SlotIndex(int32& v) { BindParam(3, v); }
	void In_SlotIndex(int32&& v)
	{
		m_slotIndex = std::move(v);
		BindParam(3, m_slotIndex);
	}

	void In_Count(int32& v) { BindParam(4, v); }
	void In_Count(int32&& v)
	{
		m_count = std::move(v);
		BindParam(4, m_count);
	}
private:
	int32 m_inventoryItemID = {};
	int32 m_itemID = {};
	int32 m_slotIndex = {};
	int32 m_count = {};
};




