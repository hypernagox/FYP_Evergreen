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
		friend static inline RefCountable* const IncAndGetPtrExternal(const RefCountable* const ref_ptr)noexcept;
		friend static inline void DecRefExternal(const RefCountable* const ref_ptr)noexcept;
		friend static inline volatile LONG& GetRefCountExternal(const RefCountable* const ref_ptr) noexcept;
		virtual ~RefCountable()noexcept = default;
	public:
		template<typename T = RefCountable>
		S_ptr<T> SharedFromThis()const noexcept { return S_ptr<T>{this}; }
		inline const int32_t UseCount()const noexcept { return m_refCount; }
		inline void IncRef()const noexcept { InterlockedIncrement(&m_refCount); }
		inline void IncRef(const uint32_t cnt)const noexcept { InterlockedAdd(&m_refCount, cnt); }
		template <typename T = RefCountable>
		constexpr inline void DecRef()const noexcept {
			const int32_t old_count = InterlockedDecrement(&m_refCount);
			if (0 == old_count) {
				if constexpr (std::same_as<std::remove_cv_t<T>, ContentsEntity>)
					aligned_xdelete_sized<T>(static_cast<T* const>(const_cast<RefCountable* const>(this)), sizeof(ContentsEntity), alignof(ContentsEntity));
				else if constexpr (std::derived_from<std::remove_cv_t<T>, Session>)
					aligned_xdelete<T>(static_cast<T* const>(const_cast<RefCountable* const>(this)), alignof(T));
				else
					xdelete<T>(const_cast<RefCountable* const>(this));
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
	private:
		inline const RefCountable* const IncAndGetPtrInternal()const noexcept { IncRef(); return this; }
		inline RefCountable* const IncAndGetPtr()const noexcept { return const_cast<RefCountable* const>(IncAndGetPtrInternal()); }
	private:
		mutable volatile LONG m_refCount = 1;
	};

	static inline RefCountable* const IncAndGetPtrExternal(const RefCountable* const ref_ptr) noexcept { return ref_ptr->IncAndGetPtr(); }
	static inline void DecRefExternal(const RefCountable* const ref_ptr) noexcept { ref_ptr->DecRef(); }
	static inline volatile LONG& GetRefCountExternal(const RefCountable* const ref_ptr) noexcept { return ref_ptr->m_refCount; }
	template <typename T>
	class S_ptr
	{
	public:
		using Type = T;

		constexpr S_ptr()noexcept = default;
		constexpr S_ptr(std::nullptr_t) noexcept : m_count_ptr{ nullptr } {}
		constexpr inline ~S_ptr()noexcept { DecRef(); }
		constexpr inline explicit S_ptr(const RefCountable* const ptr)noexcept
			:m_count_ptr{ IncAndGetPtrExternal(ptr)}
		{}
		constexpr inline explicit S_ptr(RefCountable* const ptr)noexcept
			:m_count_ptr{ IncAndGetPtrExternal(ptr) }
		{}
	public:
		S_ptr(const S_ptr& other)noexcept
			:m_count_ptr{ other.IncRef() }
		{}
		S_ptr& operator=(const S_ptr& other)noexcept {
			if (this != &other) {
				DecRef();
				m_count_ptr = other.IncRef();
			}
			other.m_count_ptr = nullptr;
			return *this;
		}
		S_ptr(S_ptr&& other)noexcept
			:m_count_ptr{ std::exchange(other.m_count_ptr,nullptr) }
		{}
		S_ptr& operator=(S_ptr&& other)noexcept {
			if (this != &other) {
				DecRef();
				m_count_ptr = other.m_count_ptr;
			}
			other.m_count_ptr = nullptr;
			return *this;
		}
	public:
		template <typename U> requires std::derived_from<U,T>
		S_ptr(const S_ptr<U>& other)noexcept
			:m_count_ptr{ other.IncRef() }
		{}
		template <typename U> requires std::derived_from<U, T>
		S_ptr& operator=(const S_ptr<U>& other)noexcept {
			if (this != static_cast<const void*const>(&other)) {
				DecRef();
				m_count_ptr = other.IncRef();
			}
			other.m_count_ptr = nullptr;
			return *this;
		}
		template <typename U> requires std::derived_from<U, T> || std::derived_from<T, U>
		S_ptr(S_ptr<U>&& other)noexcept
			:m_count_ptr{ std::exchange(other.m_count_ptr,nullptr) }
		{}
		template <typename U> requires std::derived_from<U, T>
		S_ptr& operator=(S_ptr<U>&& other)noexcept {
			if (this != static_cast<const void* const>(&other)) {
				DecRef();
				m_count_ptr = other.m_count_ptr;
			}
			other.m_count_ptr = nullptr;
			return *this;
		}
		explicit S_ptr(const uint64_t ptr)noexcept
			:m_count_ptr{ reinterpret_cast<RefCountable* const>(ptr) }
		{}
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
			if (m_count_ptr) {
				if constexpr (std::same_as<std::remove_cv_t<T>, ContentsEntity> || std::derived_from<std::remove_cv_t<T>, Session>)
					m_count_ptr->DecRef<T>();
				else
					m_count_ptr->DecRef();
			}
		}
		inline RefCountable* const IncRef()const noexcept {
			return m_count_ptr ? IncAndGetPtrExternal(m_count_ptr) : nullptr;
		}
	public:
		RefCountable* m_count_ptr = nullptr;
	};
	
	template<typename T, typename... Args> requires std::derived_from<T, RefCountable>
	constexpr inline S_ptr<T> MakeShared(Args&&... args)noexcept { 
		S_ptr<T> temp;
		temp.m_count_ptr = xnew<T>(std::forward<Args>(args)...);
		return temp;
	}

	template<typename T, typename... Args> requires std::derived_from<T, RefCountable>
	constexpr inline S_ptr<T> MakeSharedAligned(Args&&... args)noexcept {
		S_ptr<T> temp;
		temp.m_count_ptr = aligned_xnew<T>(std::forward<Args>(args)...);
		return temp;
	}

	template <typename U, typename T> requires std::derived_from<U, T>
	static inline S_ptr<U> StaticCast(const S_ptr<T>& ptr)noexcept { return S_ptr<U>{ptr.get()}; }
	template <typename U, typename T> requires std::derived_from<U,T>
	static inline S_ptr<U> StaticCast(S_ptr<T>&& ptr)noexcept { return S_ptr<U>{std::move(ptr)}; }

	template<typename T>
	class AtomicS_ptr
	{
	public:
		template <typename U>
		void store(const S_ptr<U>& p)noexcept {
			m_srwLock.lock();
			m_ptr = p;
			m_srwLock.unlock();
		}
		template <typename U>
		void store(S_ptr<U>&& p)noexcept {
			m_srwLock.lock();
			m_ptr = std::move(p);
			m_srwLock.unlock();
		}
		S_ptr<T> load()const noexcept {
			m_srwLock.lock_shared();
			S_ptr<T> temp{ m_ptr };
			m_srwLock.unlock_shared();
			return temp;
		}
		void reset()noexcept {
			m_srwLock.lock();
			m_ptr.reset();
			m_srwLock.unlock();
		}
		void store(std::nullptr_t)noexcept { reset(); }
		operator S_ptr<T>()const noexcept { return load(); }
	private:
		mutable SRWLock m_srwLock;
		S_ptr<T> m_ptr;
	};
}

namespace std {
	template <typename T>
	struct hash<ServerCore::S_ptr<T>> {
		constexpr inline const std::size_t operator()(const ServerCore::S_ptr<T>& ptr) const noexcept {
			return std::hash<T*>{}(static_cast<T* const>(ptr.m_count_ptr));
		}
	};
}