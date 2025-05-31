#pragma once

namespace Common
{
    class NaviAgent;

    class PathFinder
    {
    public:
        static DirectX::SimpleMath::Vector3 CatmullRom(const DirectX::SimpleMath::Vector3& P0, const DirectX::SimpleMath::Vector3& P1, const DirectX::SimpleMath::Vector3& P2, const DirectX::SimpleMath::Vector3& P3, const float t) noexcept
        {
            return 0.5f * (
                (2.0f * P1) +
                (-P0 + P2) * t +
                (2.0f * P0 - 5.0f * P1 + 4.0f * P2 - P3) * t * t +
                (-P0 + 3.0f * P1 - 3.0f * P2 + P3) * t * t * t
                );
        }

        static std::vector<DirectX::SimpleMath::Vector3> SmoothPath(DirectX::SimpleMath::Vector3* const path, const int s, const int numSegments)noexcept
        {
            std::vector<DirectX::SimpleMath::Vector3> smoothedPath;


            if (s < 4)
            {
                for (int i = 0; i < s; ++i)path[i].z = -path[i].z;
                return std::vector<DirectX::SimpleMath::Vector3>{ path, path + s };
            }

            smoothedPath.reserve(s);
            const int num = s - 2;

            for (int i = 1; i < num; ++i)
            {
                const DirectX::SimpleMath::Vector3& P0 = path[i - 1];
                const DirectX::SimpleMath::Vector3& P1 = path[i];
                const DirectX::SimpleMath::Vector3& P2 = path[i + 1];
                const DirectX::SimpleMath::Vector3& P3 = path[i + 2];


                for (int j = 0; j <= numSegments; ++j)
                {
                    const float t = static_cast<float>(j) / static_cast<float>(numSegments);
                    smoothedPath.emplace_back(CatmullRom(P0, P1, P2, P3, t));
                    smoothedPath.back().z = -smoothedPath.back().z;
                }
            }


            smoothedPath.emplace_back(path[s - 1]);

            return smoothedPath;
        }
    public:
        const auto GetAgent()const noexcept { return m_agent; }
        void SetNaviAgent(NaviAgent* const agent)noexcept { m_agent = agent; }
    public:
        std::span<DirectX::SimpleMath::Vector3> GetPath(const DirectX::SimpleMath::Vector3& start, const DirectX::SimpleMath::Vector3& dest)const noexcept;
    private:
        NaviAgent* m_agent;
    };
}