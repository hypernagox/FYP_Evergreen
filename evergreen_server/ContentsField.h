#pragma once
#include "Field.h"

class ContentsField
	:public ServerCore::Field
{
public:
	static constexpr inline const uint8_t NUM_OF_GROUPS = static_cast<uint8_t>(Nagox::Enum::GROUP_TYPE_MAX + 1);
public:
	~ContentsField();
public:
	virtual void InitFieldGlobal()noexcept;
	virtual void InitFieldTLS()noexcept;
	virtual void DestroyFieldTLS()noexcept;
private:

};