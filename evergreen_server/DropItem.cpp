#include "pch.h"
#include "DropItem.h"

bool DropItem::TryDropItem() const noexcept
{
	return NagiocpX::ProbabilityCheck(m_probability);
}
