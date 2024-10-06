#pragma once

#define WIN32_LEAN_AND_MEAN // 거의 사용되지 않는 내용을 Windows 헤더에서 제외합니다.
#define NOMINMAX
//#define _CRTDBG_MAP_ALLOC

#include <windows.h>
#include <iostream>
#include <functional>
#include <string>
#include <string_view>
#include <tchar.h>
#include <ranges>
#include <algorithm>
#include <shared_mutex>
#include <optional>
#include <concepts>
#include <coroutine>
#include <cassert>
#include <thread>
#include <chrono>
#include <future>
#include <vector>
#include <array>
#include <list>
#include <queue>
#include <stack>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <DbgHelp.h>
#include <winsock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>
#include <concurrent_priority_queue.h>
#pragma comment(lib, "ws2_32.lib")

#include "Types.h"
#include "define.h"

#include "func.h"
#include "PacketHeader.h"
#include "Singleton.hpp"
#include "SendBufferMgr.h"
#include "SendBufferChunk.h"
#include "SendBuffer.h"
#include "NetworkMgr.h"