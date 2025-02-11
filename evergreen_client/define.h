#pragma once

#include "pch.h"

#define DT INSTANCE(udsdx::TimeMeasure)->GetElapsedSeconds()
#define ET INSTANCE(udsdx::TimeMeasure)->GetTotalSeconds()

inline udsdx::Vector3 GetLookVector(float yaw)
{
	return udsdx::Vector3(-sinf(yaw), 0, cosf(yaw));
}