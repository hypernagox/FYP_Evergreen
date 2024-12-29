#pragma once
#include "ServerCorePch.h"

template<typename T>
class ID_Ptr
{
public:
    constexpr static const uint64_t CombineIdPtr(const uint16_t id, const T* const ptr)noexcept { return (static_cast<const uint64_t>(id) << 48) | reinterpret_cast<const uint64_t>(ptr); }
public:
    ID_Ptr()noexcept = default;
    explicit ID_Ptr(const uint16_t id_, const T* const ptr_) noexcept :combine{ CombineIdPtr(id_,ptr_) } {}
public:
    constexpr inline const uint16_t GetID()const noexcept { return static_cast<const uint16_t>(combine >> 48); }
    constexpr inline T* const GetPtr()const noexcept { return reinterpret_cast<T* const>(combine & 0xFFFFFFFFFFFF); }
    constexpr inline operator T* () const noexcept { return GetPtr(); }
private:
    uint64_t combine = 0;
};

struct alignas(2) Point2D
{
    uint8_t x, y;
    Point2D()noexcept :x{ 0 }, y{ 0 } {}
    Point2D(const uint8_t x_, const uint8_t y_)noexcept :x{ x_ }, y{ y_ } {}
    Point2D(const uint16_t xy) noexcept : x{ static_cast<const uint8_t>(xy >> 8) }, y{ static_cast<const uint8_t>(xy & 0xFF) } {}
    inline const auto operator<=>(const Point2D&)const noexcept = default;
    inline const uint16_t GetHashVal()const noexcept { return CombineXY(x, y); }
    inline static const uint16_t CombineXY(const uint8_t x_, const uint8_t y_)noexcept{ return (static_cast<const uint16_t>(x_) << 8) | y_; }
};