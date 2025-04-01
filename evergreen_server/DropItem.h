#pragma once
#include "ContentsComponent.h"

// TODO: 드랍 아이템이 해야 할 일과
// 가져야 할 정보 , 예) 어떤 아이템인가?
class DropItem
	:public ContentsComponent
{
public:
	CONSTRUCTOR_CONTENTS_COMPONENT(DropItem)
public:
	bool TryDropItem()const noexcept;
	uint8_t GetNumOfItemStack()const noexcept { return m_numOfItemsStack; }
	void SetItemStack(const uint8_t item_stack)noexcept {
		m_numOfItemsStack = item_stack;
	}
	int8_t GetDropItemDetailInfo()const noexcept { return m_dropItemDetailType; }
	void SetDropItemDetailInfo(const int8_t info) { m_dropItemDetailType = info; }
private:
	int8_t m_dropItemDetailType = -1;
	int8_t m_numOfItemsStack = -1;
	float m_probability = .7f;
	// TODO: 그 외의 아이템 식별자..

	static inline std::uniform_real_distribution<float> g_urd{ 0.0f, 1.0f };
};

