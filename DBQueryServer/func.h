#pragma once

struct PacketHeader;

template<typename T>requires std::is_enum_v<T>
constexpr const uint16 etoi(const T eType_)noexcept { return static_cast<const uint16>(eType_); }

static inline const uint64_t CombineObjectID(const uint16_t type_id, const uint64_t obj_id)noexcept {
	return (static_cast<const uint64_t>(type_id) << 48) | obj_id;
}
static inline const uint64_t GetObjectID(const uint64_t combine_id)noexcept { return combine_id & 0xFFFFFFFFFFFF; }
static inline const uint16_t GetObjectType(const uint64_t combine_id)noexcept { return static_cast<const uint16_t>(combine_id >> 48); }


std::string WideToUtf8(const std::wstring_view wstr) noexcept;
std::wstring Utf8ToWide(const std::string_view utf8Str)noexcept;

void LogStackTrace()noexcept;

void PrintError(const char* const msg, const int err_no) noexcept;
