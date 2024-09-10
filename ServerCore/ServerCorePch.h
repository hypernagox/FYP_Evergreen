#pragma once

#ifdef _DEBUG
#pragma comment(lib, "ServerCore.lib")
#else
#pragma comment(lib, "ServerCore.lib")
#endif

#pragma comment(lib, "DbgHelp.lib")

#define WIN32_LEAN_AND_MEAN // 거의 사용되지 않는 내용을 Windows 헤더에서 제외합니다.
#define NOMINMAX
//#define _CRTDBG_MAP_ALLOC
#include "CorePch.h"