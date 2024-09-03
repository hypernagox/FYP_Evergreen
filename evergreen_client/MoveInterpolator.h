#pragma once

#include "pch.h"
#include "Interpolator.hpp"
#include "ServerComponent.h"

using udsdx::Vector3;

struct MoveData
{
    Vector3 pos;
    float body_angleY;
};

class InterpolatorConcrete
    :public NetHelper::Interpolator<MoveData>
{
public:
    MoveData GetInterPolatedData()noexcept
    {
        UpdateInterpolationParam();
        return MoveData{
            SmoothLinearInterpolation(m_curData.pos, m_newData.pos) ,
            SmoothLinearInterpolation(m_curData.body_angleY, m_newData.body_angleY)
        };
    }
};

class MoveInterpolator
    :public ServerComponent
{
public:
    CONSTRUCTOR_SERVER_COMPONENT(MoveInterpolator)
public:
    virtual void Update()noexcept override;
    void UpdateNewMoveData(const Nagox::Protocol::s2c_MOVE& pkt_)noexcept;
    void InitInterpolator(const Vector3& v) {
        m_interpolator.GetCurData().pos = v;
        m_interpolator.GetNewData().pos = v;
    }
private:
    InterpolatorConcrete m_interpolator;
};