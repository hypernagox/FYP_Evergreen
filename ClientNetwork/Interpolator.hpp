#pragma once
#include "ClientNetworkPch.h"

namespace NetHelper
{
    template <typename T>
    class Interpolator
    {
    public:
        constexpr static const uint64 GetTimeStampMilliseconds()noexcept { return NetHelper::GetTimeStampMilliseconds(); }
        constexpr static const float GetTimeStampSeconds()noexcept { return NetHelper::GetTimeStampSeconds(); }
    public:
        T GetInterPolatedData()noexcept { return static_cast<T* const>(this)->GetInterPolatedData(); }
        void UpdateInterpolationParam()noexcept
        {
            m_fCurInterpolationParam = std::min(GetInterpolationParam(), m_fPrevInterpolationParam + 0.016f);
            m_fCurSmoothInterpolationParam = SmoothStep(0.f, 1.f, m_fCurInterpolationParam);
            m_fPrevInterpolationParam = m_fCurInterpolationParam;
        }
        void UpdateNewData(const T& newData_, const uint64 arrivedNewTimeStamp = GetTimeStampMilliseconds())noexcept
        {
            m_iOldTimeStamp = m_iLastDataArrivedTime;
            m_iLastDataArrivedTime = arrivedNewTimeStamp;
            m_curData = m_newData;
            m_newData = newData_;
            m_fPrevInterpolationParam = 0.f;
        }
        void UpdateOnlyTimeStamp(const uint64 arrivedNewTimeStamp)noexcept
        {
            m_iOldTimeStamp = m_iLastDataArrivedTime;
            m_iLastDataArrivedTime = arrivedNewTimeStamp;
            m_fPrevInterpolationParam = 0.f;
        }
        auto& GetCurData()noexcept { return m_curData; }
        auto& GetNewData()noexcept { return m_newData; }
        const auto& GetCurData()const noexcept { return m_curData; }
        const auto& GetNewData()const noexcept { return m_newData; }
    protected:
        T m_curData;
        T m_newData;
    protected:
        template <typename U>
        constexpr U LinearInterpolation(const U& start, const U& end)const noexcept {
            return LinearInterpolation(start, end, m_fCurInterpolationParam);
        }
        template <typename U>
        constexpr U LinearInterpolationDeg(const U& start, const U& end)const noexcept {
            U diff = end - start;
            U newEnd = end;
            if (diff > 180)
                newEnd -= 360;
			else if (diff < -180)
                newEnd += 360;
            return SmoothLinearInterpolation(start, newEnd, m_fCurInterpolationParam);
		}
        template <typename U>
        constexpr U SmoothLinearInterpolation(const U& start, const U& end)const noexcept {
            return LinearInterpolation(start, end, m_fCurSmoothInterpolationParam);
        }
        constexpr const float GetInterpolationParam()const noexcept {
            return std::clamp(static_cast<const float>(GetTimeStampMilliseconds() - m_iLastDataArrivedTime) / (float)std::max((m_iLastDataArrivedTime - m_iOldTimeStamp), MIN_UPDATE_INTERVAL), 0.f, 1.f);
        }
        constexpr const float GetSmoothInterpolationParam()const noexcept {
            return  SmoothStep(0.f, 1.f, GetInterpolationParam());
        }
    private:
        constexpr static const float SmoothStep(const float edge0, const float edge1, float x)noexcept {
            x = std::clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
            return x * x * (3 - 2 * x);
        }
        template <typename U>
        constexpr static U LinearInterpolation(const U& start, const U& end, const float t)noexcept {
            return start + (end - start) * t;
        }
        template <typename U>
        constexpr static U SmoothLinearInterpolation(const U& start, const U& end, const float t)noexcept {
            return LinearInterpolation(start, end, SmoothStep(0.f, 1.f, t));
        }
    private:
        float m_fCurInterpolationParam = 1.f;
        float m_fCurSmoothInterpolationParam = 1.f;
        float m_fPrevInterpolationParam = 0.f;

        uint64 m_iLastDataArrivedTime = GetTimeStampMilliseconds();
        uint64 m_iOldTimeStamp = GetTimeStampMilliseconds();
       
        static inline constexpr const uint64_t MIN_UPDATE_INTERVAL = 100;
    };
}