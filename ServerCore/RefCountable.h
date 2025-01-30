#pragma once
#include "ServerCorePch.h"
#include "ObjectPool.hpp"
#include "SRWLock.hpp"

namespace ServerCore
{
	
	template <typename T>
	class S_ptr;

	class RefCountable
	{
	public:
		RefCountable(const RefCountable&) = delete;
		RefCountable& operator=(const RefCountable&) = delete;
		RefCountable(RefCountable&&)noexcept = delete;
		RefCountable& operator=(RefCountable&&)noexcept = delete;
	public:
		friend static inline RefCountable* const IncAndGetPtrExternal(const RefCountable* const ref_ptr)noexcept;
		friend static inline void DecRefExternal(const RefCountable* const ref_ptr)noexcept;
		friend static inline volatile LONG& GetRefCountExternal(const RefCountable* const ref_ptr) noexcept;
	protected:
		RefCountable()noexcept = default;
		~RefCountable()noexcept = default;
	public:
		template<typename T> requires (false == std::same_as<std::decay_t<T>, RefCountable>)
		S_ptr<T> SharedFromThis()const noexcept { static_assert(false == std::same_as<std::decay_t<T>, RefCountable>); return S_ptr<T>{this}; }
		inline const int32_t UseCount()const noexcept { return m_refCount; }
		inline void IncRef()const noexcept { InterlockedIncrement(&m_refCount); }
		inline void IncRef(const uint32_t cnt)const noexcept { InterlockedAdd(&m_refCount, cnt); }
		template <typename T> requires (false == std::same_as<std::decay_t<T>, RefCountable>)
		constexpr inline void DecRef()const noexcept {
			static_assert(false == std::same_as<std::decay_t<T>, RefCountable>);
			const int32_t old_count = InterlockedDecrement(&m_refCount);
			if (0 == old_count) {
				//if constexpr (std::same_as<std::remove_cv_t<T>, ContentsEntity>)
				//	aligned_xdelete_sized<T>(static_cast<T* const>(const_cast<RefCountable* const>(this)), sizeof(ContentsEntity), alignof(ContentsEntity));
				//else if constexpr (std::derived_from<std::remove_cv_t<T>, Session>)
				//	aligned_xdelete<T>(static_cast<T* const>(const_cast<RefCountable* const>(this)), alignof(T));
				//else
					xdelete<T>(static_cast<T* const>(const_cast<RefCountable* const>(this)));
			}
			NAGOX_ASSERT(0 <= old_count);
		}
		//template <typename T = RefCountable>
		//constexpr inline void DecRef(const int32_t dec_cnt)const noexcept {
		//	const int32_t old_count = m_refCount.fetch_sub(dec_cnt, std::memory_order_acq_rel);
		//	if (dec_cnt == old_count) {
		//		if constexpr (std::same_as<T, ContentsEntity>)
		//			aligned_xdelete_sized<T>(static_cast<T* const>(const_cast<RefCountable* const>(this)), sizeof(ContentsEntity), alignof(T));
		//		else
		//			xdelete<T>(const_cast<RefCountable* const>(this));
		//	}
		//	NAGOX_ASSERT(dec_cnt <= old_count);
		//}
		inline void IncRefEnterCluster()const noexcept { m_refCount += ServerCore::NUM_OF_THREADS; }
	private:
		inline const RefCountable* const IncAndGetPtrInternal()const noexcept { IncRef(); return this; }
		inline RefCountable* const IncAndGetPtr()const noexcept { return const_cast<RefCountable* const>(IncAndGetPtrInternal()); }
	private:
		mutable volatile LONG m_refCount = 1;
	};

	static inline RefCountable* const IncAndGetPtrExternal(const RefCountable* const ref_ptr) noexcept { return ref_ptr->IncAndGetPtr(); }
	
	template<typename T> requires (false == std::same_as<std::decay_t<T>, RefCountable>)
	static inline void DecRefExternal(const RefCountable* const ref_ptr) noexcept { ref_ptr->DecRef<T>(); }
	static inline volatile LONG& GetRefCountExternal(const RefCountable* const ref_ptr) noexcept { return ref_ptr->m_refCount; }

	template <typename T>
	class S_ptr
	{
	public:
		using Type = T;

		constexpr S_ptr()noexcept = default;
		constexpr S_ptr(std::nullptr_t) noexcept : m_count_ptr{ nullptr } {}
		constexpr inline ~S_ptr()noexcept { 
			static_assert((false == std::same_as<std::decay_t<T>, RefCountable>) && std::derived_from<std::decay_t<T>, RefCountable>);
			DecRef();
		}
		constexpr inline explicit S_ptr(const RefCountable* const ptr)noexcept
			:m_count_ptr{ (T*)IncAndGetPtrExternal(ptr)}
		{}
		constexpr inline explicit S_ptr(RefCountable* const ptr)noexcept
			:m_count_ptr{ (T*)IncAndGetPtrExternal(ptr) }
		{}
	public:
		S_ptr(const S_ptr& other)noexcept
			:m_count_ptr{ other.IncRef() }
		{}
		S_ptr& operator=(const S_ptr& other)noexcept {
			if (m_count_ptr != other.m_count_ptr) [[likely]] {
				DecRef();
				m_count_ptr = other.IncRef();
				return *this;
			}
			return *this;
		}
		S_ptr(S_ptr&& other)noexcept
			:m_count_ptr{ std::exchange(other.m_count_ptr,nullptr) }
		{}
		S_ptr& operator=(S_ptr&& other)noexcept {
			if (m_count_ptr != other.m_count_ptr) [[likely]] {
				DecRef();
				m_count_ptr = other.m_count_ptr;
				other.m_count_ptr = nullptr;
				return *this;
			}
			else if(other.m_count_ptr) [[unlikely]] {
				other.m_count_ptr->RefCountable::DecRef<T>();
				other.m_count_ptr = nullptr;
			}
			return *this;
		}
	public:
		template <typename U> requires std::derived_from<U,T>
		S_ptr(const S_ptr<U>& other)noexcept
			:m_count_ptr{ other.IncRef() }
		{}
		template <typename U> requires std::derived_from<U, T>
		S_ptr& operator=(const S_ptr<U>& other)noexcept {
			if (m_count_ptr != other.m_count_ptr) [[likely]] {
				DecRef();
				m_count_ptr = other.IncRef();
				return *this;
			}
			return *this;
		}
		template <typename U> requires std::derived_from<U, T> || std::derived_from<T, U>
		S_ptr(S_ptr<U>&& other)noexcept
			:m_count_ptr{ (T*)std::exchange(other.m_count_ptr,nullptr) }
		{}
		template <typename U> requires std::derived_from<U, T>
		S_ptr& operator=(S_ptr<U>&& other)noexcept {
			if (m_count_ptr != other.m_count_ptr) [[likely]] {
				DecRef();
				m_count_ptr = other.m_count_ptr;
				other.m_count_ptr = nullptr;
				return *this;
			}
			else if (other.m_count_ptr) [[unlikely]] {
				other.m_count_ptr->RefCountable::DecRef<U>();
				other.m_count_ptr = nullptr;
			}
			return *this;
		}
		explicit S_ptr(const uint64_t ptr)noexcept
			:m_count_ptr{ reinterpret_cast<T* const>(ptr) }
		{
			static_assert(false == std::same_as<std::decay_t<T>, RefCountable>);
		}
	public:
		inline const int32_t UseCount()const noexcept { return m_count_ptr ? m_count_ptr->UseCount() : 0; }
		inline void reset()noexcept { DecRef(); release(); }
		inline void release()noexcept { m_count_ptr = nullptr; }
		template <typename U> requires std::derived_from<U, T>
		inline void swap(S_ptr<U>& ptr)noexcept { std::swap(m_count_ptr, ptr.m_count_ptr); }
	public:
		constexpr inline T& operator*()const noexcept { return *static_cast<T* const>(m_count_ptr); }
		constexpr inline T* const operator->()const noexcept { return static_cast<T* const>(m_count_ptr); }
		constexpr inline T* const get()const noexcept { return static_cast<T* const>(m_count_ptr); }
		constexpr inline const auto operator<=>(const S_ptr&)const noexcept = default;
		constexpr inline operator bool()const noexcept { return m_count_ptr; }
	public:
		inline void DecRef()const noexcept { 
			static_assert(false == std::same_as<std::decay_t<T>, RefCountable>);
			if (m_count_ptr) {
				m_count_ptr->RefCountable::DecRef<T>();
				//if constexpr (std::same_as<std::remove_cv_t<T>, ContentsEntity> || std::derived_from<std::remove_cv_t<T>, Session>)
				//	m_count_ptr->DecRef<T>();
				//else
				//	m_count_ptr->DecRef();
			}
		}
		inline T* const IncRef()const noexcept {
			static_assert(false == std::same_as<std::decay_t<T>, RefCountable>);
			return m_count_ptr ? (T*)IncAndGetPtrExternal(m_count_ptr) : nullptr;
		}
	public:
		T* m_count_ptr = nullptr;
	};
	
	template<typename T, typename... Args> requires std::derived_from<T, RefCountable>
	constexpr inline S_ptr<T> MakeShared(Args&&... args)noexcept { 
		return S_ptr<T>{reinterpret_cast<const uint64_t>(xnew<T>(std::forward<Args>(args)...))};
	}

	template<typename T, typename... Args> requires std::derived_from<T, RefCountable>
	constexpr inline S_ptr<T> MakeSharedAligned(Args&&... args)noexcept {
		static_assert(false == std::derived_from<std::decay_t<T>, Session>);
		static_assert(false == std::derived_from<std::decay_t<T>, ContentsEntity>);
		return S_ptr<T>{reinterpret_cast<const uint64_t>(aligned_xnew<T>(std::forward<Args>(args)...))};
	}

	template <typename U, typename T> requires std::derived_from<U, T>
	static inline S_ptr<U> StaticCast(const S_ptr<T>& ptr)noexcept { return S_ptr<U>{ptr.get()}; }
	template <typename U, typename T> requires std::derived_from<U,T>
	static inline S_ptr<U> StaticCast(S_ptr<T>&& ptr)noexcept { return S_ptr<U>{std::move(ptr)}; }

	template<typename T>
	class AtomicS_ptr
	{
	public:
		constexpr ~AtomicS_ptr()noexcept { if (m_ptr)m_ptr->RefCountable::DecRef<T>(); }
	public:
		template <typename U>
		void store(const S_ptr<U>& p)noexcept {
			const auto other_ptr = static_cast<U* const>(p.m_count_ptr);
			if (other_ptr)other_ptr->IncRef();
			m_srwLock.lock();
			const auto prev_ptr = m_ptr;
			m_ptr = other_ptr;
			m_srwLock.unlock();
			if (prev_ptr)prev_ptr->RefCountable::DecRef<T>();
		}
		template <typename U>
		void store(S_ptr<U>&& p)noexcept {
			const auto other_ptr = static_cast<U* const>(p.m_count_ptr);
			p.m_count_ptr = nullptr;
			m_srwLock.lock();
			const auto prev_ptr = m_ptr;
			m_ptr = other_ptr;
			m_srwLock.unlock();
			if (prev_ptr && prev_ptr != other_ptr)prev_ptr->RefCountable::DecRef<T>();
		}
		S_ptr<T> load()const noexcept {
			m_srwLock.lock_shared();
			const uint64_t temp = reinterpret_cast<const uint64_t>(m_ptr);
			if (temp)reinterpret_cast<T* const>(temp)->IncRef();
			m_srwLock.unlock_shared();
			return S_ptr<T>{temp};
		}
		void reset()noexcept {
			m_srwLock.lock();
			const auto prev_ptr = m_ptr;
			m_ptr = nullptr;
			m_srwLock.unlock();
			prev_ptr->RefCountable::DecRef<T>(); // 현재는 nullptr이 아님을 확신하고있음
		}
		void store(std::nullptr_t)noexcept { reset(); }
		operator S_ptr<T>()const noexcept { return load(); }
	private:
		mutable SRWLock m_srwLock;
		T* m_ptr = nullptr;
	};

#define CHECK_CORRUPTION
	template<const uint32_t NUM_OF_PAD> requires (8 > NUM_OF_PAD)
	class alignas(8) PadByte 
	{
	public:
#ifdef CHECK_CORRUPTION
		PadByte()noexcept :pad{ 0 } {}
		~PadByte()noexcept {
			for (const auto data : pad) {
				if (!data)continue;
				PrintLogEndl("CORRUPTION DETECTED");
				break;
			}
		}
#else
		PadByte()noexcept = default;
#endif
	private:
		alignas(8) int64_t pad[NUM_OF_PAD];
	};
}

using ServerCore::S_ptr;

namespace std {
	template <typename T>
	struct hash<ServerCore::S_ptr<T>> {
		constexpr inline const std::size_t operator()(const ServerCore::S_ptr<T>& ptr) const noexcept {
			return std::hash<T*>{}(static_cast<T* const>(ptr.m_count_ptr));
		}
	};
	template <typename T>
	struct hash<std::pair<uint32_t, T*>> {
		constexpr inline const std::size_t operator()(const std::pair<uint32_t, T*>& pair) const noexcept {
			return std::hash<std::size_t>{}(static_cast<std::size_t>(pair.first));
		}
	};
}