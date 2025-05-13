#pragma once

#define _SILENCE_CXX20_CISO646_REMOVED_WARNING
#define WIN32_LEAN_AND_MEAN // 거의 사용되지 않는 내용을 Windows 헤더에서 제외합니다.
#define NOMINMAX

#include <directxtk12/SimpleMath.h>
#include <recastnavigation/Recast.h>
#include <recastnavigation/DetourNavMesh.h>
#include <recastnavigation/DetourNavMeshQuery.h>

#include <rapidjson/document.h>

#include <execution>
#include <string_view>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>	
#include <filesystem>
#include <Windows.h>
#include <array>
#include <stack>
#include <unordered_set>
#include <span>
#include <format>
#include <variant>
#include <algorithm>
#include "recastnavigation/DetourCrowd.h"
#include "SharedDefines.h"

using Vector3 = DirectX::SimpleMath::Vector3;

#include "CommonMath.h"
#include "json.hpp"
#include "DataRegistry.h"