#pragma once

// TODO: ��ȹ���� �����
// TODO: ������ Ŭ�� ������ ������ �ĺ��� �ʿ�
// ���̵��: enum �ΰ� ���� / ��) 8����Ʈ�� �� 4����Ʈ�� ������Ÿ��, ��4����Ʈ�� �� �������� ����ũ�� �ĺ���
// ������ �ĺ��� �ʿ�
class Item
{

public:
	virtual bool UseItem(ContentsEntity* const owner)noexcept { return true; }
public:
	int8_t m_itemDetailType = -1;
	int8_t m_numOfItemStack = -1;
};

