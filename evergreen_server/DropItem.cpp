#include "pch.h"
#include "DropItem.h"

bool DropItem::TryDropItem() const noexcept
{
	return g_urd(g_RandEngine) < m_probability;
}
