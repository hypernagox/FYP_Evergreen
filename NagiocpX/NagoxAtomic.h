#pragma once
#include "NagiocpXPch.h"

namespace NagoxAtomic
{
    namespace NagoxAtomicDetail
    {
        template<typename T>
        concept IsPointer = std::is_pointer_v<T>;

        template<typename T>
        concept NotPointer = !IsPointer<T>;

        template <typename T, std::size_t Alignment>
        concept Aligned = (alignof(T) == Alignment);

        template<typename T>
        concept CustomAtomicCompatible = std::is_trivially_copyable_v<T> && std::is_default_constructible_v<T>;

        template <typename T>
        concept AtomicType = CustomAtomicCompatible<T> && (sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4 || sizeof(T) == 8);

        template <typename T>
        concept BIT8 = AtomicType<T> && (sizeof(T) == 1);

        template <typename T>
        concept BIT16 = AtomicType<T> && (sizeof(T) == 2) && Aligned<T, 2>;

        template <typename T>
        concept BIT32 = AtomicType<T> && (sizeof(T) == 4) && Aligned<T, 4>;

        template <typename T>
        concept BIT64 = AtomicType<T> && (sizeof(T) == 8) && Aligned<T, 8>;

        template <typename T>
        concept PTR = std::is_pointer_v<T> && Aligned<T, 8>;

        template<typename To, typename From> requires (sizeof(From) == sizeof(To)) && CustomAtomicCompatible<From> && CustomAtomicCompatible<To>
        union ReinterpretConverter {
            From from;
            To to;
        };
    }

#define RETURN_VALUE_TYPE_CONVERT(ConvertType, func_) (NagoxAtomicDetail::ReinterpretConverter<ConvertType,std::decay_t<decltype(func_)>>{func_}.to)
#define CONVERT_VALUE_TYPE(ConvertType, origin_) (reinterpret_cast<const ConvertType&>(const_cast<const std::decay_t<decltype(origin_)>&>(origin_)))

    template <NagoxAtomicDetail::AtomicType T>
    class Atomic { 
    public: 
        constexpr Atomic()noexcept { static_assert(std::_Always_false<T>); }
        constexpr ~Atomic()noexcept { static_assert(std::_Always_false<T>); } 
    };

    template <NagoxAtomicDetail::BIT8 T>
    class Atomic<T>
    {
    public:
        Atomic(const Atomic&) = delete;
        Atomic& operator=(const Atomic&) = delete;
        Atomic(Atomic&&)noexcept = delete;
        Atomic& operator=(Atomic&&)noexcept = delete;

    public:
        inline explicit Atomic(const T initialValue = {}) noexcept
            : value{ CONVERT_VALUE_TYPE(CHAR, initialValue) } {
        }

        T load_relaxed() const noexcept {
            return CONVERT_VALUE_TYPE(T, value);
        }

        void store_relaxed(const T newValue) noexcept {
            value = CONVERT_VALUE_TYPE(CHAR, newValue);
        }

        T load() const noexcept {
            _Compiler_barrier();
            return CONVERT_VALUE_TYPE(T, value);
        }

        T load_atomic()const noexcept {
            return RETURN_VALUE_TYPE_CONVERT(T, InterlockedExchangeAdd8(&(const_cast<Atomic* const>(this)->value), 0));
        }

        void store(const T newValue) noexcept {
            InterlockedExchange8(&value, CONVERT_VALUE_TYPE(CHAR, newValue));
        }

        T fetch_add(const CHAR operand) noexcept requires std::is_integral_v<T> {
            return InterlockedExchangeAdd8(&value, CONVERT_VALUE_TYPE(CHAR, operand));
        }

        T fetch_sub(const CHAR operand) noexcept requires std::is_integral_v<T> {
            return fetch_add(-operand);
        }

        T exchange(const T newValue) noexcept {
            return RETURN_VALUE_TYPE_CONVERT(T, InterlockedExchange8(&value, CONVERT_VALUE_TYPE(CHAR, newValue)));
        }

        bool compare_exchange(const T expected, const T desired) noexcept {
            return _InterlockedCompareExchange8(&value, CONVERT_VALUE_TYPE(CHAR, desired), CONVERT_VALUE_TYPE(CHAR, expected)) ==
                CONVERT_VALUE_TYPE(CHAR, expected);
        }

    public:
        T operator++() noexcept requires std::is_integral_v<T> {
            return fetch_add(1) + 1;
        }

        T operator++(int) noexcept requires std::is_integral_v<T> {
            return fetch_add(1);
        }

        T operator--() noexcept requires std::is_integral_v<T> {
            return fetch_sub(1) - 1;
        }

        T operator--(int) noexcept requires std::is_integral_v<T> {
            return fetch_sub(1);
        }

        operator T()const noexcept { return load(); }

    private:
        volatile CHAR value;
    };

    template <NagoxAtomicDetail::BIT16 T>
    class Atomic<T>
    {
    public:
        Atomic(const Atomic&) = delete;
        Atomic& operator=(const Atomic&) = delete;
        Atomic(Atomic&&)noexcept = delete;
        Atomic& operator=(Atomic&&)noexcept = delete;

    public:
        inline explicit Atomic(const T initialValue = {}) noexcept
            : value{ CONVERT_VALUE_TYPE(SHORT, initialValue) } {
        }

        T load_relaxed() const noexcept {
            return CONVERT_VALUE_TYPE(T, value);
        }

        void store_relaxed(const T newValue) noexcept {
            value = CONVERT_VALUE_TYPE(SHORT, newValue);
        }

        T load() const noexcept {
            _Compiler_barrier();
            return CONVERT_VALUE_TYPE(T, value);
        }

        T load_atomic()const noexcept {
            return RETURN_VALUE_TYPE_CONVERT(T, _InterlockedExchangeAdd16(&(const_cast<Atomic* const>(this)->value), 0));
        }

        void store(const T newValue) noexcept {
            InterlockedExchange16(&value, CONVERT_VALUE_TYPE(SHORT, newValue));
        }

        T fetch_add(const SHORT operand) noexcept requires std::is_integral_v<T> {
            return _InterlockedExchangeAdd16(&value, CONVERT_VALUE_TYPE(SHORT, operand));
        }

        T fetch_sub(const SHORT operand) noexcept requires std::is_integral_v<T> {
            return fetch_add(-operand);
        }

        T exchange(const T newValue) noexcept {
            return RETURN_VALUE_TYPE_CONVERT(T, InterlockedExchange16(&value, CONVERT_VALUE_TYPE(SHORT, newValue)));
        }

        bool compare_exchange(const T expected, const T desired) noexcept {
            return InterlockedCompareExchange16(&value, CONVERT_VALUE_TYPE(SHORT, desired), CONVERT_VALUE_TYPE(SHORT, expected)) ==
                CONVERT_VALUE_TYPE(SHORT, expected);
        }

    public:
        T operator++() noexcept requires std::is_integral_v<T> {
            return InterlockedIncrement16(&value);
        }

        T operator++(int) noexcept requires std::is_integral_v<T> {
            return fetch_add(1);
        }

        T operator--() noexcept requires std::is_integral_v<T> {
            return InterlockedDecrement16(&value);
        }

        T operator--(int) noexcept requires std::is_integral_v<T> {
            return fetch_sub(1);
        }

        operator T()const noexcept { return load(); }

    private:
        alignas(2) volatile SHORT value;
    };

    template <NagoxAtomicDetail::BIT32 T>
    class Atomic<T>
    {
    public:
        Atomic(const Atomic&) = delete;
        Atomic& operator=(const Atomic&) = delete;
        Atomic(Atomic&&)noexcept = delete;
        Atomic& operator=(Atomic&&)noexcept = delete;

    public:
        inline explicit Atomic(const T initialValue = {}) noexcept
            : value{ CONVERT_VALUE_TYPE(LONG, initialValue) } {
        }

        T load_relaxed() const noexcept {
            return CONVERT_VALUE_TYPE(T, value);
        }

        void store_relaxed(const T newValue) noexcept {
            value = CONVERT_VALUE_TYPE(LONG, newValue);
        }

        T load() const noexcept {
            _Compiler_barrier();
            return CONVERT_VALUE_TYPE(T, value);
        }

        T load_atomic()const noexcept {
            return RETURN_VALUE_TYPE_CONVERT(T, InterlockedExchangeAdd(&(const_cast<Atomic* const>(this)->value), 0));
        }

        void store(const T newValue) noexcept {
            InterlockedExchange(&value, CONVERT_VALUE_TYPE(LONG, newValue));
        }

        T fetch_add(const LONG operand) noexcept requires std::is_integral_v<T> {
            return InterlockedExchangeAdd(&value, CONVERT_VALUE_TYPE(LONG, operand));
        }

        T fetch_sub(const LONG operand) noexcept requires std::is_integral_v<T> {
            return fetch_add(-operand);
        }

        T exchange(const T newValue) noexcept {
            return RETURN_VALUE_TYPE_CONVERT(T, InterlockedExchange(&value, CONVERT_VALUE_TYPE(LONG, newValue)));
        }

        bool compare_exchange(const T expected, const T desired) noexcept {
            return InterlockedCompareExchange(&value, CONVERT_VALUE_TYPE(LONG, desired), CONVERT_VALUE_TYPE(LONG, expected)) ==
                CONVERT_VALUE_TYPE(LONG, expected);
        }

    public:
        T operator++() noexcept requires std::is_integral_v<T> {
            return InterlockedIncrement(&value);
        }

        T operator++(int) noexcept requires std::is_integral_v<T> {
            return fetch_add(1);
        }

        T operator--() noexcept requires std::is_integral_v<T> {
            return InterlockedDecrement(&value);
        }

        T operator--(int) noexcept requires std::is_integral_v<T> {
            return fetch_sub(1);
        }

        operator T()const noexcept { return load(); }

    private:
        alignas(4) volatile LONG value;
    };

    template <NagoxAtomicDetail::BIT64 T> requires NagoxAtomicDetail::NotPointer<T>
    class Atomic<T>
    {
    public:
        Atomic(const Atomic&) = delete;
        Atomic& operator=(const Atomic&) = delete;
        Atomic(Atomic&&)noexcept = delete;
        Atomic& operator=(Atomic&&)noexcept = delete;

    public:
        inline explicit Atomic(const T initialValue = {}) noexcept
            : value{ CONVERT_VALUE_TYPE(LONG64, initialValue) } {
        }

        T load_relaxed() const noexcept {
            return CONVERT_VALUE_TYPE(T, value);
        }

        void store_relaxed(const T newValue) noexcept {
            value = CONVERT_VALUE_TYPE(LONG64, newValue);
        }

        T load() const noexcept {
            _Compiler_barrier();
            return CONVERT_VALUE_TYPE(T, value);
        }

        T load_atomic()const noexcept {
            return RETURN_VALUE_TYPE_CONVERT(T, InterlockedExchangeAdd64(&(const_cast<Atomic* const>(this)->value), 0));
        }

        void store(const T newValue) noexcept {
            InterlockedExchange64(&value, CONVERT_VALUE_TYPE(LONG64, newValue));
        }

        T fetch_add(const LONG64 operand) noexcept requires std::is_integral_v<T> {
            return InterlockedExchangeAdd64(&value, CONVERT_VALUE_TYPE(LONG64, operand));
        }

        T fetch_sub(const LONG64 operand) noexcept requires std::is_integral_v<T> {
            return fetch_add(-operand);
        }

        T exchange(const T newValue) noexcept {
            return RETURN_VALUE_TYPE_CONVERT(T, InterlockedExchange64(&value, CONVERT_VALUE_TYPE(LONG64, newValue)));
        }

        bool compare_exchange(const T expected, const T desired) noexcept {
            return InterlockedCompareExchange64(&value, CONVERT_VALUE_TYPE(LONG64, desired), CONVERT_VALUE_TYPE(LONG64, expected)) ==
                CONVERT_VALUE_TYPE(LONG64, expected);
        }

    public:
        T operator++() noexcept requires std::is_integral_v<T> {
            return InterlockedIncrement64(&value);
        }

        T operator++(int) noexcept requires std::is_integral_v<T> {
            return fetch_add(1);
        }

        T operator--() noexcept requires std::is_integral_v<T> {
            return InterlockedDecrement64(&value);
        }

        T operator--(int) noexcept requires std::is_integral_v<T> {
            return fetch_sub(1);
        }

        operator T()const noexcept { return load(); }

    private:
        alignas(8) volatile LONG64 value;
    };

    template <NagoxAtomicDetail::PTR T>
    class Atomic<T>
    {
    public:
        Atomic(const Atomic&) = delete;
        Atomic& operator=(const Atomic&) = delete;
        Atomic(Atomic&&)noexcept = delete;
        Atomic& operator=(Atomic&&)noexcept = delete;

    public:
        inline explicit Atomic(const T initialValue = nullptr) noexcept
            : value{ (PVOID)(initialValue) } {
        }

        T load_relaxed() const noexcept {
            return static_cast<T>(value);
        }

        void store_relaxed(const T newValue) noexcept {
            value = (PVOID)(newValue);
        }

        T load() const noexcept {
            _Compiler_barrier();
            return static_cast<T>(value);
        }

        T load_atomic()const noexcept {
            return reinterpret_cast<T>(InterlockedExchangeAdd64((LONG64*)(&(const_cast<Atomic* const>(this)->value)), 0));
        }

        void store(const T newValue) noexcept {
            InterlockedExchangePointer(&value, newValue);
        }

        T exchange(const T newValue) noexcept {
            return static_cast<T>(InterlockedExchangePointer(&value, newValue));
        }

        bool compare_exchange(const T expected, const T desired) noexcept {
            return InterlockedCompareExchangePointer(&value, desired, expected) == (PVOID)expected;
        }

        operator T()const noexcept { return load(); }

    private:
        alignas(8) volatile PVOID value;
    };
}