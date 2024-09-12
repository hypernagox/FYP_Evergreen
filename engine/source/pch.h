#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers.
#endif

// C++ Standard Library
#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <cmath>
#include <concepts>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <limits>
#include <set>
#include <map>
#include <memory>
#include <optional>
#include <queue>
#include <random>
#include <span>
#include <sstream>
#include <string>
#include <string_view>
#include <thread>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

// Windows Library
#include <Windows.h>
#include <windowsx.h>
#include <wrl.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <comdef.h>
#include <commdlg.h>
#include <wincodec.h>
#include <wincodecsdk.h>

using namespace Microsoft::WRL;

// DirectX12 Library
#include <directx/d3dx12.h>
#include <d3dcompiler.h>

#include <DirectXMath.h>
#include <DirectXColors.h>
#include <DirectXCollision.h>
#include <DirectXPackedVector.h>

using namespace DirectX;

// DirectXTK Library
#include <directxtk12/Audio.h>
#include <directxtk12/Mouse.h>
#include <directxtk12/Keyboard.h>
#include <directxtk12/SimpleMath.h>
#include <directxtk12/ResourceUploadBatch.h>
#include <directxtk12/DDSTextureLoader.h>
#include <directxtk12/WICTextureLoader.h>

// Tracy Profiler
#include "../tracy/public/tracy/Tracy.hpp"
#include "../tracy/public/tracy/TracyD3D12.hpp"

// In-Engine Library
#include "define.h"
#include "d3dUtil.h"
#include "custom_math.h"
#include "singleton.h"
#include "vertex.h"
#include "UploadBuffer.h"

// Link Static Library
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "runtimeobject.lib")