#pragma once
#include "pch.h"
#include "../ClientNetwork/ClientNetworkPch.h"

using udsdx::Vector3;

Vector3 ToOriginVec3(const Nagox::Struct::Vec3* const v)noexcept;

const Nagox::Struct::Vec3 ToFlatVec3(const Vector3& v)noexcept;
