#pragma once

#ifdef USENETWORK
constexpr const bool g_bUseNetWork = true;
#else
constexpr const bool g_bUseNetWork = false;
#endif

constexpr const bool g_bUseDefaultIP = true;

static constexpr inline const uint8_t NUM_OF_GROUPS = static_cast<uint8_t>(Nagox::Enum::GROUP_TYPE_MAX + 1);

// �Һз�
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
	// TODO: ������ ���̵� ������ ���ϱ�
	DEFAULT = 0,
};

// ������ ������ �ʺ�-���� , �� ������ ���Ͻ�ų constexpr ���� �� ����
// ex) constexpr int WORLD_WIDTH = 1400
