#pragma once

#define WIN32_LEAN_AND_MEAN // 거의 사용되지 않는 내용을 Windows 헤더에서 제외합니다.
#define NOMINMAX

#include <iostream>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "DbgHelp.lib")

#include <windows.h>
#include <ws2tcpip.h>
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
#include "Types.h"
#include "CoreMacro.h"
#include <sql.h>
#include <sqlext.h>
#include <concurrent_queue.h>
#include <concurrent_unordered_map.h>
#include "Singleton.hpp"
#include "func.h"

template <typename T>
using S_ptr = std::shared_ptr<T>;