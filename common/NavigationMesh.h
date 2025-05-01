#pragma once
#include "pch.h"
#include "NaviCell.h"

class dtNavMesh;
class dtNavMeshQuery;

namespace Common
{
	class NavigationMesh
	{
	public:
		NavigationMesh();
		virtual ~NavigationMesh();
		int Init(const std::wstring_view path);
	public:
		void GetRandomPos(Vector3& out_pos, NaviCell& outCell)const noexcept;
	public:
		const std::vector<DirectX::SimpleMath::Vector3>& GetPathVertices(
			const DirectX::SimpleMath::Vector3& start,
			const DirectX::SimpleMath::Vector3& end,
			const float step = 5.f
		);
		int findRandomPointAroundCircle(float* pos, float radius, float* outPos);
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
			nav->init(m_navMesh, 2048);
			return nav;
		}
		void FreeNavMeshQuery()const noexcept { dtFreeNavMeshQuery(const_cast<dtNavMeshQuery*>(GetNavMeshQuery())); }
		const dtNavMeshQuery* const GetNavMeshQuery()const noexcept
		{
			constinit thread_local const dtNavMeshQuery* nav_q = nullptr;
			if (nullptr == nav_q) [[unlikely]]
			{
				nav_q = InitNavMeshQuery();
			}
			return nav_q;
		}
		dtQueryFilter* const GetNavFilter()const noexcept { return const_cast<NavigationMesh*>(this)->m_filter; }
		NaviCell GetNaviCell(Vector3& pos)const noexcept { return NaviCell{ pos,this }; }
		dtCrowd* GetCrowd()noexcept { return m_crowd; }
	protected:
		static int ParseJson(const std::wstring_view path, rapidjson::Document& doc);
		static int FullPolyDataFromJsonObj(rapidjson::Document& doc, struct rcPolyMesh& mesh);
	private:
		dtNavMesh* m_navMesh;
		dtQueryFilter* m_filter;
		dtCrowd* m_crowd = nullptr;
		static constexpr float m_polyPickExt[3]{ 2,4,2 };
	};
}