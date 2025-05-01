#include "pch.h"
#include "NavigationMesh.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <recastnavigation/DetourNavMeshBuilder.h>
#include <rapidjson/document.h>

using namespace rapidjson;

namespace Common
{
	static constexpr int NAVMESHSET_MAGIC = 'M' << 24 | 'S' << 16 | 'E' << 8 | 'T'; //'MSET';
	static constexpr int NAVMESHSET_VERSION = 1;
	struct NavMeshSetHeader
	{
		int magic;
		int version;
		int numTiles;
		dtNavMeshParams params;
	};

	struct NavMeshTileHeader
	{
		dtTileRef tileRef;
		int dataSize;
	};

	NavigationMesh::NavigationMesh()
	{
	}

	NavigationMesh::~NavigationMesh()
	{
		// TODO: crowd 悼矫己力绢
		// crowd 皋葛府包府
		if (m_crowd)
			dtFreeCrowd(m_crowd);
		dtFreeNavMesh(m_navMesh);
		delete m_filter;
	}

	int NavigationMesh::Init(const std::wstring_view path)
	{
		m_navMesh = LoadNavMesh(path);
		if (m_navMesh != NULL)
		{
			m_filter = new dtQueryFilter;
			m_filter->setIncludeFlags(0xFFFF);
			m_filter->setExcludeFlags(0);
			m_crowd = new dtCrowd;
			m_crowd->init(100, 5.f, m_navMesh);
		}
		return m_navMesh != NULL ? 1 : 0;
	}

	static float frand()noexcept
	{
		return (float)rand() / (float)RAND_MAX;
	}

	void NavigationMesh::GetRandomPos(Vector3& out_pos, NaviCell& outCell) const noexcept
	{
		GetNavMeshQuery()->findRandomPoint(m_filter, frand, &outCell.m_cell, &out_pos.x);
		CommonMath::InverseZ(out_pos);
	}

	int NavigationMesh::findRandomPointAroundCircle(float* pos, float radius, float* outPos)
	{
		dtPolyRef ref;
		dtPolyRef m_startRef = 0;
		dtQueryFilter m_filter;
		//m_filter.setIncludeFlags(0xffff);
		//m_filter.setExcludeFlags(0);

		const auto nav_q = GetNavMeshQuery();
		nav_q->findNearestPoly(pos, m_polyPickExt, &m_filter, &m_startRef, 0);
		const dtStatus status = nav_q->findRandomPointAroundCircle(m_startRef, pos, radius, &m_filter, frand, &ref, outPos);
		if (dtStatusSucceed(status))
		{
			return 1;
		}
		return 0;
	}


	int NavigationMesh::ConvertJsonToNavBinFile(const std::string_view jsonContent, const std::wstring_view savePath)
	{
		Document doc;
		doc.Parse(jsonContent.data());

		rcPolyMesh* m_pmesh = rcAllocPolyMesh();
		if (!m_pmesh)
		{
			return 0;
		}
		FullPolyDataFromJsonObj(doc, *m_pmesh);
		int nvp = doc["nvp"].GetInt();
		dtNavMesh* navMesh = NULL;
		if (nvp <= DT_VERTS_PER_POLYGON)
		{
			unsigned char* navData = 0;
			int navDataSize = 0;

			dtNavMeshCreateParams params;
			memset(&params, 0, sizeof(params));
			params.verts = m_pmesh->verts;
			params.vertCount = m_pmesh->nverts;
			params.polys = m_pmesh->polys;
			params.polyAreas = m_pmesh->areas;
			params.polyFlags = m_pmesh->flags;
			params.polyCount = m_pmesh->npolys;
			params.nvp = m_pmesh->nvp;
			params.walkableHeight = doc["agentHeight"].GetFloat();
			params.walkableRadius = doc["agentRadius"].GetFloat();
			params.walkableClimb = doc["agentMaxClimb"].GetFloat();
			//params.walkableHeight = 2;
			//params.walkableRadius = 0.6;
			//params.walkableClimb = 0.9;
			rcVcopy(params.bmin, m_pmesh->bmin);
			rcVcopy(params.bmax, m_pmesh->bmax);
			params.cs = m_pmesh->cs;
			params.ch = m_pmesh->ch;
			params.buildBvTree = true;

			if (!dtCreateNavMeshData(&params, &navData, &navDataSize))
			{
				return 0;
			}

			navMesh = dtAllocNavMesh();
			if (!navMesh)
			{
				dtFree(navData);
				return 0;
			}

			dtStatus status;

			status = navMesh->init(navData, navDataSize, DT_TILE_FREE_DATA);
			if (dtStatusFailed(status))
			{
				dtFree(navData);
				return 0;
			}
			SaveNavMesh(savePath, navMesh);
		}
		return 1;
	}

	void NavigationMesh::SaveNavMesh(const std::wstring_view savePath, const dtNavMesh* mesh)
	{
		if (!mesh) return;

		FILE* fp = nullptr;
		if (_wfopen_s(&fp, savePath.data(), L"wb") != 0 || !fp)
			return;

		// Store header.
		NavMeshSetHeader header;
		header.magic = NAVMESHSET_MAGIC;
		header.version = NAVMESHSET_VERSION;
		header.numTiles = 0;
		for (int i = 0; i < mesh->getMaxTiles(); ++i)
		{
			const dtMeshTile* tile = mesh->getTile(i);
			if (!tile || !tile->header || !tile->dataSize) continue;
			header.numTiles++;
		}
		memcpy(&header.params, mesh->getParams(), sizeof(dtNavMeshParams));
		fwrite(&header, sizeof(NavMeshSetHeader), 1, fp);

		// Store tiles.
		for (int i = 0; i < mesh->getMaxTiles(); ++i)
		{
			const dtMeshTile* tile = mesh->getTile(i);
			if (!tile || !tile->header || !tile->dataSize) continue;

			NavMeshTileHeader tileHeader;
			tileHeader.tileRef = mesh->getTileRef(tile);
			tileHeader.dataSize = tile->dataSize;
			fwrite(&tileHeader, sizeof(tileHeader), 1, fp);

			fwrite(tile->data, tile->dataSize, 1, fp);
		}

		fclose(fp);
	}

	dtNavMesh* NavigationMesh::LoadNavMesh(const std::wstring_view path)
	{
		FILE* fp = nullptr;
		if (_wfopen_s(&fp, path.data(), L"rb") != 0 || !fp) return 0;

		// Read header.
		NavMeshSetHeader header;
		size_t readLen = fread(&header, sizeof(NavMeshSetHeader), 1, fp);
		if (readLen != 1)
		{
			fclose(fp);
			return 0;
		}
		if (header.magic != NAVMESHSET_MAGIC)
		{
			fclose(fp);
			return 0;
		}
		if (header.version != NAVMESHSET_VERSION)
		{
			fclose(fp);
			return 0;
		}

		dtNavMesh* mesh = dtAllocNavMesh();
		if (!mesh)
		{
			fclose(fp);
			return 0;
		}
		dtStatus status = mesh->init(&header.params);
		if (dtStatusFailed(status))
		{
			fclose(fp);
			return 0;
		}

		// Read tiles.
		for (int i = 0; i < header.numTiles; ++i)
		{
			NavMeshTileHeader tileHeader;
			readLen = fread(&tileHeader, sizeof(tileHeader), 1, fp);
			if (readLen != 1)
			{
				fclose(fp);
				return 0;
			}

			if (!tileHeader.tileRef || !tileHeader.dataSize)
				break;

			unsigned char* data = (unsigned char*)dtAlloc(tileHeader.dataSize, DT_ALLOC_PERM);
			if (!data) break;
			memset(data, 0, tileHeader.dataSize);
			readLen = fread(data, tileHeader.dataSize, 1, fp);
			if (readLen != 1)
			{
				dtFree(data);
				fclose(fp);
				return 0;
			}

			mesh->addTile(data, tileHeader.dataSize, DT_TILE_FREE_DATA, tileHeader.tileRef, 0);
		}
		fclose(fp);
		return mesh;
	}

	int NavigationMesh::FullPolyDataFromJson(const std::wstring_view path, rcPolyMesh& mesh)
	{
		Document doc;
		ParseJson(path, doc);
		return FullPolyDataFromJsonObj(doc, mesh);
	}

	int NavigationMesh::ParseJson(const std::wstring_view path, rapidjson::Document& doc)
	{
		FILE* fp = nullptr;
		if (_wfopen_s(&fp, path.data(), L"r") != 0 || !fp)
			return 0;
		fseek(fp, 0L, SEEK_END);
		int fileSize = ftell(fp);
		char* data = (char*)dtAlloc(fileSize, DT_ALLOC_PERM);
		if (!data)
			return 0;
		memset(data, 0, fileSize);
		fseek(fp, 0L, SEEK_SET);
		const auto is_read_ok = fread(data, fileSize, 1, fp);
		if (is_read_ok != 1)
		{
			dtFree(data);
			fclose(fp);
			return 0;
		}
		doc.Parse(data);
		dtFree(data);
		fclose(fp);
		if (doc.HasParseError())
		{
			return 0;
		}
		return 1;
	}

	int NavigationMesh::FullPolyDataFromJsonObj(rapidjson::Document& doc, rcPolyMesh& mesh)
	{
		mesh.maxEdgeError = 1.3f;
		mesh.borderSize = 0;
		int nvp = doc["nvp"].GetInt();
		float cs = doc["cs"].GetFloat();
		float ch = doc["ch"].GetFloat();
		mesh.nvp = nvp;
		mesh.cs = cs;
		mesh.ch = ch;
		rapidjson::Value& bmin_array = doc["bmin"];
		if (bmin_array.IsArray())
		{
			mesh.bmin[0] = bmin_array[0].GetFloat();
			mesh.bmin[1] = bmin_array[1].GetFloat();
			mesh.bmin[2] = bmin_array[2].GetFloat();
		}
		rapidjson::Value& bmax_array = doc["bmax"];
		if (bmax_array.IsArray())
		{
			mesh.bmax[0] = bmax_array[0].GetFloat();
			mesh.bmax[1] = bmax_array[1].GetFloat();
			mesh.bmax[2] = bmax_array[2].GetFloat();
		}
		rapidjson::Value& vertex_array = doc["v"];
		if (vertex_array.IsArray())
		{
			int maxVertices = vertex_array.Size();
			mesh.nverts = maxVertices;
			mesh.verts = (unsigned short*)malloc(sizeof(unsigned short) * maxVertices * 3);
			memset(mesh.verts, 0, sizeof(unsigned short) * maxVertices * 3);
			for (SizeType i = 0; i < vertex_array.Size(); i++)
			{
				rapidjson::Value& vertex = vertex_array[i];
				unsigned short* v = &mesh.verts[i * 3];
				v[0] = vertex[0].GetInt();
				v[1] = vertex[1].GetInt();
				v[2] = vertex[2].GetInt();
				//printf("a[%d] = %ud %ud %ud\n", i, v[0], v[1], v[2]);
			}
		}
		rapidjson::Value& polys_array = doc["p"];
		if (polys_array.IsArray())
		{
			int polyNum = polys_array.Size();
			mesh.npolys = polyNum;
			mesh.maxpolys = polyNum;
			int mallocSize = sizeof(unsigned short) * polyNum * 2 * nvp;
			mesh.polys = (unsigned short*)malloc(mallocSize);
			memset(mesh.polys, 0xffff, mallocSize);
			mesh.regs = (unsigned short*)malloc(sizeof(unsigned short) * polyNum);
			memset(mesh.regs, 1, sizeof(unsigned short) * polyNum);
			mesh.areas = (unsigned char*)malloc(sizeof(unsigned char) * polyNum);
			memset(mesh.areas, 1, sizeof(unsigned char) * polyNum);
			mesh.flags = (unsigned short*)malloc(sizeof(unsigned short) * polyNum);
			memset(mesh.flags, 1, sizeof(unsigned short) * polyNum);
			for (SizeType i = 0; i < polys_array.Size(); i++)
			{
				rapidjson::Value& poly = polys_array[i];
				assert(poly.IsArray() && poly.Size() == nvp * 2);
				unsigned short* p = &mesh.polys[i * nvp * 2];
				for (int j = 0; j < nvp * 2; j++)
				{
					p[j] = poly[j].GetInt();
				}
				//printf("a[%d] = %ud %ud %ud\n", i, v[0], v[1], v[2]);
			}
		}
		return 1;
	}

	thread_local std::vector<DirectX::SimpleMath::Vector3> path_result = {};

	const std::vector<DirectX::SimpleMath::Vector3>& NavigationMesh::GetPathVertices(
		const DirectX::SimpleMath::Vector3& start,
		const DirectX::SimpleMath::Vector3& end,
		const float step
	)
	{
		constexpr const int MAX_PATH_COUNT = 128;
		extern thread_local std::vector<DirectX::SimpleMath::Vector3> path_result;
		path_result.clear();

		const dtNavMeshQuery* const navQuery = GetNavMeshQuery();
		const dtQueryFilter& filter = *m_filter;
		constexpr const float EXTENTS[3] = { 2.0f, 4.0f, 2.0f };

		const float startPos[3] = { start.x, start.y, -start.z };
		const float endPos[3] = { end.x, end.y, -end.z };

		dtPolyRef startRef, endRef;
		float closestStart[3], closestEnd[3];

		if (dtStatusFailed(navQuery->findNearestPoly(startPos, EXTENTS, &filter, &startRef, closestStart)))
			return path_result;

		if (dtStatusFailed(navQuery->findNearestPoly(endPos, EXTENTS, &filter, &endRef, closestEnd)))
			return path_result;

		dtPolyRef path[MAX_PATH_COUNT];
		int pathCount = 0;

		if (dtStatusFailed(navQuery->findPath(startRef, endRef, closestStart, closestEnd, &filter, path, &pathCount, MAX_PATH_COUNT)))
			return path_result;

		float straightPath[MAX_PATH_COUNT * 3];
		unsigned char flags[MAX_PATH_COUNT];
		dtPolyRef polys[MAX_PATH_COUNT];
		int straightPathCount = 0;

		if (dtStatusFailed(navQuery->findStraightPath(closestStart, closestEnd, path, pathCount,
			straightPath, flags, polys, &straightPathCount, MAX_PATH_COUNT)))
			return path_result;

		for (int i = 0; i < straightPathCount - 1; ++i)
		{
			const Vector3& p1 = reinterpret_cast<const Vector3&>(straightPath[i * 3]);
			const Vector3& p2 = reinterpret_cast<const Vector3&>(straightPath[(i + 1) * 3]);
			const Vector3 delta = p2 - p1;
			const float distance = delta.Length();
			if (distance < 1e-5f) 
				continue;
			const int segmentCount = static_cast<int>(distance / step);
			const Vector3 direction = delta / distance;
			for (int j = 0; j < segmentCount; ++j)
			{
				const auto v = p1 + direction * (step * j);
				path_result.emplace_back(v.x, v.y, -v.z);
			}
		}
		const Vector3& p3 = reinterpret_cast<const Vector3&>(straightPath[(straightPathCount - 1) * 3]);
		path_result.emplace_back(p3.x, p3.y, -p3.z);
		return path_result;
	}
}