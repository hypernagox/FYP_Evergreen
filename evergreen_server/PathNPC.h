#pragma once
#include "ContentsComponent.h"

class PathNPC
	:public ContentsComponent
{
	CONSTRUCTOR_CONTENTS_COMPONENT(PathNPC)
public:
	void UpdateMove();
	void InitPathNPC();
public:
	class NaviAgent* m_navAgent = nullptr;
	XVector<std::pair<Vector3, float>> m_vecDirDists;
	int m_cur_idx = 0;
	float m_speed = 1.f;
	float m_curDistAcc = 0.f;
	class PartyQuestSystem* m_owner_system = nullptr;
	uint64_t m_last_update_timestamp = ::GetTickCount64();
};

