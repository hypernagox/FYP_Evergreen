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

		// TODO: 적당히 검색할 섹터만 잘 고르는 휴리스틱 만들기
		// TODO: 함수포인터 등록을 고려하자
		static const Vector<Sector*> GetAdjSectors(const ContentsEntity* const pEntity_)noexcept;

		//static void SetSectorFillterHeuristic(const auto fp)noexcept { g_sectorFillterFunc = fp; }
	private:
		Point2D m_cur_sectorXY;

	private:
		//using SectorFillter = Vector<Sector*>(*)(const ContentsEntity* const)noexcept;

		//static inline SectorFillter g_sectorFillterFunc = {};
	};
}

