#pragma once
#include "Field.h"

class ContentsField
	:public NagiocpX::Field
{
public:
	static constexpr inline const uint8_t NUM_OF_GROUPS = static_cast<uint8_t>(Nagox::Enum::GROUP_TYPE_MAX + 1);
public:
	ContentsField();
	~ContentsField();
public:
	virtual void InitFieldGlobal()noexcept override;
	virtual void InitFieldTLS()noexcept override;
	virtual void MigrationAfterBehavior(Field* const prev_field)noexcept override;
private:

};