#pragma once

#ifdef USENETWORK
constexpr const bool g_bUseNetWork = true;
#else
constexpr const bool g_bUseNetWork = false;
#endif

constexpr const bool g_bUseDefaultIP = true;

static constexpr inline const uint8_t NUM_OF_GROUPS = static_cast<uint8_t>(Nagox::Enum::GROUP_TYPE_MAX + 1);

// 소분류
enum PLAYER_TYPE_INFO : uint8_t
{
	WARRIOR = 0,


};

enum MONSTER_TYPE_INFO : uint8_t
{
	FOX = 0,
};

enum ITEM_TYPE_INFO : uint8_t
{
	// TODO: 아이템 아이디 정수값 정하기
	DEFAULT = 0,
};

// 앞으로 월드의 너비-높이 , 등 서버와 통일시킬 constexpr 값이 올 예정
// ex) constexpr int WORLD_WIDTH = 1400
