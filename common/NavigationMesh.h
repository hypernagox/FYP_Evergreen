#pragma once
#include "pch.h"
#include "NaviCell.h"

class dtNavMesh;
class dtNavMeshQuery;
class Navigator;

namespace Common
{
	extern thread_local std::default_random_engine LDefaultRandEngine;

	class NavigationMesh
	{
		friend class Navigator;
	public:
		NavigationMesh();
		virtual ~NavigationMesh();
		int Init(const dtNavMesh* const p_nav_mesh);
	public:
		void GetRandomPos(Vector3& out_pos, NaviCell& outCell)const noexcept;
	public:
		const std::vector<DirectX::SimpleMath::Vector3>& GetPathVertices(
			const DirectX::SimpleMath::Vector3& start,
			const DirectX::SimpleMath::Vector3& end,
			const float step = 5.f
		);
		int findRandomPointAroundCircle(const float* pos, float radius, float* outPos);
		static dtNavMesh* LoadNavMesh(const std::wstring_view path);
		static void SaveNavMesh(const std::wstring_view savePath, const dtNavMesh* mesh);
		static int ConvertJsonToNavBinFile(const std::string_view  jsonContent, const std::wstring_view savePath);
		static int FullPolyDataFromJson(const std::wstring_view path, struct rcPolyMesh& mesh);
	public:
		const dtNavMesh* const GetNavMesh()const noexcept { return m_navMesh; }
		const auto InitNavMeshQuery()const noexcept
		{
			const auto nav = dtAllocNavMeshQuery();
			// TODO: 매직넘버
			nav->init(m_navMesh, 256);
			return nav;
		}
		const dtNavMeshQuery* const GetNavMeshQuery()const noexcept { return m_nav_q; }
		const dtQueryFilter* const GetNavFilter()const noexcept { return const_cast<NavigationMesh*>(this)->m_filter; }
		NaviCell GetNaviCell(Vector3& pos)const noexcept { return NaviCell{ pos,this }; }
	protected:
		static int ParseJson(const std::wstring_view path, rapidjson::Document& doc);
		static int FullPolyDataFromJsonObj(rapidjson::Document& doc, struct rcPolyMesh& mesh);
	private:
		dtNavMeshQuery* m_nav_q = nullptr;
		const dtNavMesh* m_navMesh = nullptr;
		dtQueryFilter* m_filter = nullptr;
		static constexpr const float m_polyPickExt[3]{ 2,4,2 };
	};
}