#pragma once
#include "pch.h"

class NaviMesh;
class NaviCell;

class NaviAgent
{
	static constexpr const int NAVI_DEPTH = 5; // 네비셀과 그 이웃을 어느 깊이까지 탐색 할 까?
public:
	NaviAgent();
public:
	// 이동 후 내 셀이 어디일지 계산해본다
	const NaviCell* GetPostCell(const DirectX::SimpleMath::Vector3& pos)const noexcept;
	
	// 지금 위치한 내 셀의 경사각을 얻는다.
	float GetSlope()const noexcept; 

	// 다른 셀과의 경사각을 구한다.
	float GetSlope(const NaviCell* const cell)const noexcept; 

	// 다른 셀과의 거리를 구해본다
	float GetCellDist(const NaviCell* const cell)const noexcept;

	// 이번 프레임 결정된 위치를 기반으로 적절한 위치를 구한다 (수정 및 경사각 반영 필요)
	DirectX::SimpleMath::Vector3 GetNaviPos(const DirectX::SimpleMath::Vector3& pos)noexcept; 
	
private:
	// 주의: 네비셀은 네비메시랑 같이 움직여야한다
	// 네비셀 자체엔 내가 어디 네비메시(청크)소속인지에 대한 정보가 없기 때문임
	// 넣을순 있는데 그러면 네비셀이 캐시라인 하나에 다 안들어옴
	const NaviMesh* m_pNaviMesh = nullptr; // 이 에이전트가 속한 지금 네비메시(청크)
	const NaviCell* m_pNaviCell = nullptr; // 이번 프레임에서 이동하기 전 위치해있던 cell
											// prev cell이라고도 할 수 있음.
};

