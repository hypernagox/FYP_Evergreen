#pragma once
#include "ContentsComponent.h"

namespace ServerCore
{
	class SectorInfoHelper
		: public ContentsComponent
	{
	public:
		CONSTRUCTOR_CONTENTS_COMPONENT(SectorInfoHelper)
	public:
		void SetSectorXY(const uint8_t x, const uint8_t y)noexcept { m_cur_sectorXY = { x,y }; }
		const Point2D GetCurXY()const noexcept { return m_cur_sectorXY; }
	public:
		void ImmigrationSector(const float x, const float y)noexcept;
	public:
		static Vector<uint32_t> FillterSessionEntities(const ContentsEntity* const pEntity_)noexcept;

		static void BroadcastWithID(const Vector<uint32_t>& id, const S_ptr<SendBuffer>& pkt_)noexcept;

		// TODO: ������ �˻��� ���͸� �� ���� �޸���ƽ �����
		// TODO: �Լ������� ����� �������
		static const Vector<Sector*> GetAdjSectors(const ContentsEntity* const pEntity_)noexcept;

		//static void SetSectorFillterHeuristic(const auto fp)noexcept { g_sectorFillterFunc = fp; }
	private:
		Point2D m_cur_sectorXY;

	private:
		//using SectorFillter = Vector<Sector*>(*)(const ContentsEntity* const)noexcept;

		//static inline SectorFillter g_sectorFillterFunc = {};
	};
}

