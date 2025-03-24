using System.Collections;
using System.Collections.Generic;
using UnityEditor;
using System;
using UnityEngine.AI;
using System.IO;
using System.Runtime.Serialization;
using System.Linq;
using System.Text;
using UnityEngine;

namespace Extend
{
    public static class NavMeshExporter
    {
        // [System.Runtime.InteropServices.DllImport("NavigatorForUnity", CallingConvention = System.Runtime.InteropServices.CallingConvention.Winapi)]    
        [System.Runtime.InteropServices.DllImport("NavigatorForUnity")]
        private static extern int ConvertJsonToNavBinFile(string jsonContent, string savePath);

        private static int MaxVertexPerPoly = 6;
        private const ushort NullIndex = 0xffff;

        // these two gotten from recast demo
        private const float xzCellSize = 0.30f;
        private const float yCellSize = 0.20f;

        private static string ProgressBarName = "작업 중...";

        [UnityEditor.MenuItem("NavMeshExporter/ExportToNavBin")]
        static void ExportToNavBin()
        {
            ProgressBarName = "ExportToNavBin";
            string outstring = GenNavMesh("json");
            string select_path = EditorUtility.SaveFilePanel("NavMesh를 NavBin 파일로 내보내기", "", "navmesh", "bin");
            EditorUtility.DisplayProgressBar(ProgressBarName, "타일 캐시로 변환 중", 0.85f);
            int resultCode = ConvertJsonToNavBinFile(outstring, select_path);
            EditorUtility.ClearProgressBar();
            if (resultCode == 1)
                EditorUtility.DisplayDialog("알림", "NavMesh를 NavBin 파일로 내보내기 성공!", "확인");
            else
                EditorUtility.DisplayDialog("오류", "NavMesh를 NavBin 파일로 내보내기 실패!", "닫기");
        }

        [UnityEditor.MenuItem("NavMeshExporter/ExportToJson")]
        static void ExportToJson()
        {
            ProgressBarName = "ExportToJson";
            string outstring = GenNavMesh("json");
            string select_path = EditorUtility.SaveFilePanel("NavMesh를 JSON 파일로 내보내기", "", "navmesh", "json");
            EditorUtility.DisplayProgressBar(ProgressBarName, "JSON 파일 쓰기 시작", 0.95f);
            System.IO.File.WriteAllText(select_path, outstring);
            EditorUtility.ClearProgressBar();
            EditorUtility.DisplayDialog("알림", "NavMesh를 JSON 파일로 내보내기 성공!", "확인");
        }

        [UnityEditor.MenuItem("NavMeshExporter/ExportToObj")]
        static void ExportToObj()
        {
            ProgressBarName = "ExportToObj";
            string outstring = GenNavMesh("obj");
            string select_path = EditorUtility.SaveFilePanel("NavMesh를 OBJ 파일로 내보내기", "", "navmesh", "obj");
            EditorUtility.DisplayProgressBar(ProgressBarName, "OBJ 파일 쓰기 시작", 0.95f);
            System.IO.File.WriteAllText(select_path, outstring);
            EditorUtility.ClearProgressBar();
            EditorUtility.DisplayDialog("알림", "NavMesh를 OBJ 파일로 내보내기 성공!", "확인");
        }
        static void GetBounds(List<Vector3> vertices, ref float[] boundsMin, ref float[] boundsMax)
        {
            for (int axis = 0; axis <= 2; axis++)
            {
                float min_value = Int32.MaxValue;
                float max_value = Int32.MinValue;
                for (int i = 0; i < vertices.Count; i++)
                {
                    float test_value = vertices[i][axis];
                    if (test_value < min_value)
                    {
                        min_value = test_value;
                    }
                    if (test_value > max_value)
                    {
                        max_value = test_value;
                    }
                }
                boundsMin[axis] = min_value;
                boundsMax[axis] = max_value;
            }
        }

        // NavMesh 원본 JSON 생성(테스트용)
        static string GenNavMeshOriginJson()
        {
            UnityEngine.AI.NavMeshTriangulation navtri = UnityEngine.AI.NavMesh.CalculateTriangulation();
            string outnav = "";
            outnav = "{\"v\":[\n";
            for (int i = 0; i < navtri.vertices.Length; i++)
            {
                if (i > 0)
                    outnav += ",\n";
                outnav += "[" + navtri.vertices[i].x + "," + navtri.vertices[i].y + "," + navtri.vertices[i].z + "]";
            }
            outnav += "\n],\"p\":[\n";

            for (int i = 0; i < navtri.indices.Length; i++)
            {
                if (i > 0)
                    outnav += ",\n";

                int index = navtri.indices[i];
                outnav += index.ToString();
                var sphere = GameObject.CreatePrimitive(PrimitiveType.Sphere);
                sphere.name = "s" + index;
                sphere.transform.position = navtri.vertices[index];
            }
            outnav += "\n]}";
            return outnav;
        }

        static void GetShardVertex(List<int> polysA, List<int> polysB, ref List<int> shardVertex)
        {
            for (int i = 0; i < polysA.Count; i++)
            {
                for (int j = 0; j < polysB.Count; j++)
                {
                    if (polysA[i] == polysB[j] && polysA[i] != NullIndex)
                    {
                        shardVertex.Add(i);
                    }
                }
            }
        }

        static void MergePolyAndNeighbor(ref List<List<int>> polys, List<List<int>> neighbor)
        {
            for (int i = 0; i < polys.Count; i++)
            {
                var poly = polys[i];
                List<int> neighborVal = neighbor[i];
                for (int j = 0; j < MaxVertexPerPoly; j++)
                {
                    poly.Add(neighborVal[j]);
                }
            }
        }

        static int GetVaildVertexNum(List<int> poly)
        {
            int num = 0;
            for (int i = 0; i < poly.Count; i++)
            {
                if (poly[i] == NullIndex)
                    break;
                num++;
            }
            return num;
        }

        static void GenNeighbor(List<List<int>> polys, ref List<List<int>> neighbor, float progress)
        {
            if (neighbor == null)
                neighbor = new List<List<int>>();

            for (int i = 0; i < polys.Count; i++)
                neighbor.Add(new List<int>(Enumerable.Repeat((int)NullIndex, MaxVertexPerPoly)));

            var edgeToPolys = new Dictionary<(int, int), List<(int polyIndex, int edgeIndex)>>();

            for (int polyIndex = 0; polyIndex < polys.Count; polyIndex++)
            {
                var poly = polys[polyIndex];
                int validVerts = GetVaildVertexNum(poly);

                for (int i = 0; i < validVerts; i++)
                {
                    int v1 = poly[i];
                    int v2 = poly[(i + 1) % validVerts];
                    var edgeKey = (Math.Min(v1, v2), Math.Max(v1, v2));

                    if (!edgeToPolys.TryGetValue(edgeKey, out var list))
                    {
                        list = new List<(int, int)>();
                        edgeToPolys[edgeKey] = list;
                    }
                    list.Add((polyIndex, i));
                }
            }

            int count = 0;
            foreach (var kvp in edgeToPolys)
            {
                var polyList = kvp.Value;

                if (polyList.Count == 2)
                {
                    var (a, aIdx) = polyList[0];
                    var (b, bIdx) = polyList[1];
                    neighbor[a][aIdx] = b;
                    neighbor[b][bIdx] = a;
                }
                else if (polyList.Count > 2)
                {
                    Debug.LogError($"❌ 여러 폴리곤이 같은 edge를 공유함: {kvp.Key.Item1}, {kvp.Key.Item2}");
                }


                if (++count % 500 == 0)
                {
                    float detailPro = progress + 0.1f * count / edgeToPolys.Count;
                    EditorUtility.DisplayProgressBar(ProgressBarName, $"인접 다각형 계산 중 ({count}/{edgeToPolys.Count})", detailPro);
                }
            }
        }



        static private void AddPolyByVertex(List<int> vertexList, ref List<List<int>> polys, bool isReverse)
        {
            if (MaxVertexPerPoly < vertexList.Count)
                MaxVertexPerPoly = vertexList.Count;

            var polyVert = new List<int>(vertexList);
            if (isReverse)
                polyVert.Reverse();

            polys.Add(polyVert);
        }

        // TODO: area 필드 내보내기
        static string GenNavMesh(string style)
        {
            float progress = 0;
            float detailPro = 0;

            EditorUtility.DisplayProgressBar(ProgressBarName, "NavMesh 삼각형 데이터 가져오는 중", progress);
            UnityEngine.AI.NavMeshTriangulation navtri = UnityEngine.AI.NavMesh.CalculateTriangulation();

            Dictionary<Vector3, int> vertexMap = new Dictionary<Vector3, int>(navtri.vertices.Length);
            Dictionary<int, int> indexmap = new Dictionary<int, int>();
            List<Vector3> repos = new List<Vector3>();

            progress += 0.1f;
            float mergeSameVertexSumPro = 0.1f;

            for (int i = 0; i < navtri.vertices.Length; i++)
            {
                if (i % 10 == 0)
                    EditorUtility.DisplayProgressBar(ProgressBarName, $"동일 정점 병합 중({i + 1}/{navtri.vertices.Length})", progress + mergeSameVertexSumPro * (i + 1) / navtri.vertices.Length);

                if (!vertexMap.TryGetValue(navtri.vertices[i], out int existingIndex))
                {
                    existingIndex = repos.Count;
                    repos.Add(navtri.vertices[i]);
                    vertexMap[navtri.vertices[i]] = existingIndex;
                }

                indexmap[i] = existingIndex;
            }

            progress = detailPro;
            detailPro = 0.0f;

            HashSet<int> polylast = new HashSet<int>();
            List<List<int>> polys = new List<List<int>>();
            MaxVertexPerPoly = 6;
            float makePolySumPro = 0.1f;

            for (int i = 0; i < navtri.indices.Length / 3; i++)
            {
                if (i % 10 == 0)
                {
                    detailPro = progress + makePolySumPro * (i / (float)(navtri.indices.Length / 3));
                    EditorUtility.DisplayProgressBar(ProgressBarName, $"인접 삼각형 병합 중({i}/{navtri.indices.Length / 3})", detailPro);
                }

                int i0 = navtri.indices[i * 3 + 0];
                int i1 = navtri.indices[i * 3 + 1];
                int i2 = navtri.indices[i * 3 + 2];

                if (polylast.Contains(i0) || polylast.Contains(i1) || polylast.Contains(i2))
                {
                    polylast.Add(i0);
                    polylast.Add(i1);
                    polylast.Add(i2);
                }
                else
                {
                    if (polylast.Count > 0)
                    {
                        AddPolyByVertex(polylast.ToList(), ref polys, style == "json");
                    }
                    polylast.Clear();
                    polylast.Add(i0);
                    polylast.Add(i1);
                    polylast.Add(i2);
                }
            }

            if (polylast.Count > 0)
            {
                AddPolyByVertex(polylast.ToList(), ref polys, style == "json");
            }

            progress = detailPro;
            detailPro = 0.0f;
            StringBuilder outnavBuilder = new StringBuilder();

            if (style == "json")
            {
                for (int i = 0; i < polys.Count; i++)
                {
                    for (int j = 0; j < polys[i].Count; j++)
                    {
                        if (polys[i][j] != NullIndex)
                            polys[i][j] = indexmap[polys[i][j]];
                    }
                    for (int ii = polys[i].Count; ii < MaxVertexPerPoly; ii++)
                        polys[i].Add(NullIndex);
                }

                List<List<int>> neighbor = new List<List<int>>();
                GenNeighbor(polys, ref neighbor, progress);
                progress += 0.1f;

                MergePolyAndNeighbor(ref polys, neighbor);

                float[] boundsMin = new float[3];
                float[] boundsMax = new float[3];

                EditorUtility.DisplayProgressBar(ProgressBarName, "모든 정점의 X축 반전", progress += 0.1f);
              
                EditorUtility.DisplayProgressBar(ProgressBarName, "바운딩 박스 계산", progress += 0.1f);
                GetBounds(repos, ref boundsMin, ref boundsMax);

                for (int i = 0; i < repos.Count; i++)
                {
                    float rx = repos[i].x - boundsMin[0];
                    float ry = repos[i].y - boundsMin[1];
                    float rz = repos[i].z - boundsMin[2];
                    repos[i] = new Vector3(
                        (ushort)Math.Round(rx / xzCellSize),
                        (ushort)Math.Round(ry / yCellSize),
                        (ushort)Math.Round(rz / xzCellSize)
                    );
                }

                outnavBuilder.Append("{");
                outnavBuilder.AppendFormat("\"nvp\":{0},\n", MaxVertexPerPoly);
                outnavBuilder.AppendFormat("\"cs\":{0},\n", xzCellSize);
                outnavBuilder.AppendFormat("\"ch\":{0},\n", yCellSize);

                var settings = NavMesh.GetSettingsByIndex(0);
                outnavBuilder.AppendFormat("\"agentHeight\":{0},\n", settings.agentHeight);
                outnavBuilder.AppendFormat("\"agentRadius\":{0},\n", settings.agentRadius);
                outnavBuilder.AppendFormat("\"agentMaxClimb\":{0},\n", settings.agentClimb);

                outnavBuilder.AppendFormat("\"bmin\":[{0},{1},{2}],\n", boundsMin[0], boundsMin[1], boundsMin[2]);
                outnavBuilder.AppendFormat("\"bmax\":[{0},{1},{2}],\n", boundsMax[0], boundsMax[1], boundsMax[2]);

                // 정점
                outnavBuilder.Append("\"v\":[\n");
                for (int i = 0; i < repos.Count; i++)
                {
                    detailPro = progress + 0.1f * (i + 1) / repos.Count;
                    if (i % 10 == 0)
                        EditorUtility.DisplayProgressBar(ProgressBarName, $"JSON 정점 데이터 생성 중({i + 1}/{repos.Count})", detailPro);

                    if (i > 0)
                        outnavBuilder.Append(",\n");

                    outnavBuilder.AppendFormat("[{0},{1},{2}]", repos[i].x, repos[i].y, -repos[i].z);
                }
                outnavBuilder.Append("\n],\n\"p\":[\n");

                progress = detailPro;

                // 폴리곤
                for (int i = 0; i < polys.Count; i++)
                {
                    detailPro = progress + 0.1f * (i + 1) / polys.Count;
                    if (i % 10 == 0)
                        EditorUtility.DisplayProgressBar(ProgressBarName, $"JSON 다각형 데이터 생성 중({i + 1}/{polys.Count})", detailPro);

                    if (i > 0)
                        outnavBuilder.Append(",\n");

                    List<int> polyLine = new List<int>();
                    for (int j = 0; j < MaxVertexPerPoly * 2; j++)
                        polyLine.Add(polys[i][j]);

                    outnavBuilder.Append("[" + string.Join(",", polyLine) + "]");
                }

                outnavBuilder.Append("\n]}");
            }
            else if (style == "obj")
            {
                detailPro = 0.0f;

                for (int i = 0; i < repos.Count; i++)
                {
                    detailPro = progress + 0.4f * (i + 1) / polys.Count;
                    if (i % 10 == 0)
                        EditorUtility.DisplayProgressBar(ProgressBarName, $"OBJ 정점 데이터 생성 중({i + 1}/{repos.Count})", detailPro);

                    outnavBuilder.AppendFormat("v {0} {1} {2}\r\n", repos[i].x, repos[i].y, -repos[i].z);
                }
                outnavBuilder.Append("\r\n");

                progress = detailPro;

                for (int i = 0; i < polys.Count; i++)
                {
                    detailPro = progress + 0.5f * (i + 1) / polys.Count;
                    if (i % 10 == 0)
                        EditorUtility.DisplayProgressBar(ProgressBarName, $"OBJ 다각형 데이터 생성 중({i + 1}/{polys.Count})", detailPro);

                    outnavBuilder.Append("f");
                    for (int j = polys[i].Count - 1; j >= 0; j--)
                    {
                        outnavBuilder.Append(" " + (indexmap[polys[i][j]] + 1));
                    }
                    outnavBuilder.Append("\r\n");
                }
            }

            return outnavBuilder.ToString();
        }
    }
}