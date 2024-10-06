#pragma once
#include "pch.h"

class NaviMesh;
class NaviCell;

class NaviAgent
{
	static constexpr const int NAVI_DEPTH = 5; // �׺񼿰� �� �̿��� ��� ���̱��� Ž�� �� ��?
public:
	NaviAgent();
public:
	// �̵� �� �� ���� ������� ����غ���
	const NaviCell* GetPostCell(const DirectX::SimpleMath::Vector3& pos)const noexcept;
	
	// ���� ��ġ�� �� ���� ��簢�� ��´�.
	float GetSlope()const noexcept; 

	// �ٸ� ������ ��簢�� ���Ѵ�.
	float GetSlope(const NaviCell* const cell)const noexcept; 

	// �ٸ� ������ �Ÿ��� ���غ���
	float GetCellDist(const NaviCell* const cell)const noexcept;

	// �̹� ������ ������ ��ġ�� ������� ������ ��ġ�� ���Ѵ� (���� �� ��簢 �ݿ� �ʿ�)
	DirectX::SimpleMath::Vector3 GetNaviPos(const DirectX::SimpleMath::Vector3& pos)noexcept; 
	
private:
	// ����: �׺��� �׺�޽ö� ���� ���������Ѵ�
	// �׺� ��ü�� ���� ��� �׺�޽�(ûũ)�Ҽ������� ���� ������ ���� ������
	// ������ �ִµ� �׷��� �׺��� ĳ�ö��� �ϳ��� �� �ȵ���
	const NaviMesh* m_pNaviMesh = nullptr; // �� ������Ʈ�� ���� ���� �׺�޽�(ûũ)
	const NaviCell* m_pNaviCell = nullptr; // �̹� �����ӿ��� �̵��ϱ� �� ��ġ���ִ� cell
											// prev cell�̶�� �� �� ����.
};

