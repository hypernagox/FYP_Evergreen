#pragma once

#ifdef _DEBUG
#pragma comment(lib, "ServerCore.lib")
#else
#pragma comment(lib, "ServerCore.lib")
#endif

#ifdef _DEBUG
#pragma comment(lib, "lua54.lib")
#else
#pragma comment(lib, "lua54.lib")
#endif


#ifdef _DEBUG
#pragma comment(lib, "tbb12_debug.lib")
#else
#pragma comment(lib, "tbb12.lib")
#endif

#ifdef _DEBUG
#pragma comment(lib, "libtcmalloc_minimal.lib")
#else
#pragma comment(lib, "libtcmalloc_minimal.lib")
#endif

#pragma comment(lib, "DbgHelp.lib")

#define WIN32_LEAN_AND_MEAN // 거의 사용되지 않는 내용을 Windows 헤더에서 제외합니다.
#define NOMINMAX
//#define _CRTDBG_MAP_ALLOC
#include "CorePch.h"