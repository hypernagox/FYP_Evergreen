#pragma once
#include "ContentsComponent.h"

// TODO: ��� �������� �ؾ� �� �ϰ�
// ������ �� ���� , ��) � �������ΰ�?
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
private:
	uint8_t m_numOfItemsStack = 1;
	float m_probability = .7f;
	// TODO: �� ���� ������ �ĺ���..

	static inline std::uniform_real_distribution<float> g_urd{ 0.0f, 1.0f };
};

