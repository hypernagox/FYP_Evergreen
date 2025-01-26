#pragma once
#include "NetAddress.h"
#include "MPSCQueue.hpp"

namespace ServerCore
{
	class ContentsEntity;
	class Session;
	class IocpCore;
	class Listener;

	enum class SERVICE_TYPE : uint8
	{
		SERVER,
		CLIENT,
		END
	};

	/*-------------------
		Service
	-------------------*/

	using SessionFactory = std::function<Session*(void)>;

	class Service
	{
		struct alignas(64) AtomicSessionPtr
		{
			AtomicS_ptr<ContentsEntity> ptr;
		};
		constinit static inline const Service* g_mainService;
	public:
		static inline constexpr const Service* const GetMainService()noexcept { return g_mainService; }
	public:
		Service(
			  const IocpCore& iocpCore_
			, SERVICE_TYPE eServiceType_
			, NetAddress addr_
			, SessionFactory factory_
			, const Session::PacketHandleFunc* const global_packet_func,
			  c_int32 maxSessionCount_ = 1);
		virtual ~Service();
	public:
		virtual bool Start() = 0;
		virtual void CloseService();
		bool CanStart()const { return nullptr != m_sessionFactory; }
		const bool AddSession(const S_ptr<PacketSession>& pSession_)noexcept;
		void ReleaseSession(const Session* const pSession_)noexcept;
		int32 GetMaxSessionCount()const noexcept { return m_maxSessionCount; }
		SERVICE_TYPE GetServiceType()const noexcept { return m_eServiceType; }
		NetAddress GetNetAddress()const noexcept { return m_netAddr; }
		const IocpCore& GetIocpCore()const noexcept { return m_iocpCore; }
		S_ptr<ContentsEntity> GetSession(const uint64_t sessionID_)const noexcept;
	public:
		S_ptr<Session> PopSession()noexcept{
			Session* pSession;
			if (m_sessionPool.try_pop_single(pSession) ){
				return S_ptr<Session>{reinterpret_cast<const uint64_t>(pSession)};
			}
			else {
				return S_ptr<Session>{};
			}
		}
		const bool IsSessionPoolEmpty()const noexcept { return m_sessionPool.empty_single(); }
		void ReturnSession(Session* const pSession)noexcept;
	public:
		void IterateSession(std::function<void(Session* const)> fpIterate_)noexcept;
	protected:
		Session* const CreateSession()noexcept;
	protected:
		MPSCStack<Session*> m_sessionPool;
		mutable tbb::concurrent_unordered_map<uint32_t, uint16_t> m_id2Index;
		AtomicSessionPtr* const m_arrSession;
		const int32_t m_maxSessionCount;
		//MPSCStack<int32_t> m_idxStack;
		//tbb::concurrent_bounded_queue<int32,StlAllocator64<int32>> m_idxQueue;
		const SERVICE_TYPE m_eServiceType;
		const NetAddress m_netAddr;
		int32_t m_curNumOfSessions = 0;
		const SessionFactory m_sessionFactory;
		const IocpCore& m_iocpCore;
		template<typename T>
		static const std::span<T> CreateDynamicSpan(const size_t size_)noexcept {
			const auto arr_ptr = static_cast<T* const>(::_aligned_malloc(sizeof(T) * size_, alignof(T)));
			const auto e = arr_ptr + size_;
			for (auto b = arr_ptr; e != b;)std::construct_at<T>(b++);
			return { arr_ptr,e };
		}

		template<typename T>
		static void DestroyDynamicSpan(const std::span<T> target_arr)noexcept {
			const auto arr_ptr = target_arr.data();
			const size_t num_of_elements = target_arr.size();
			const auto e = arr_ptr + num_of_elements;
			for (auto b = arr_ptr; e != b;)std::destroy_at<T>(b++);
			::_aligned_free(arr_ptr);
		}
	};


	/*-------------------
		ClientService
	-------------------*/
	class ClientService
		:public Service
	{
	public:
		ClientService(
			  const IocpCore& iocpCore_
			, NetAddress targetServerAddr_
			, SessionFactory factory_
			, const Session::PacketHandleFunc* const global_packet_func
			, c_int32 maxSessionCount_ = 1);
		virtual ~ClientService();
	public:
		virtual bool Start()override;
		virtual void CloseService()override;
	private:

	};


	/*-------------------
		ServerService
	-------------------*/

	class ServerService
		:public Service
	{
	public:
		ServerService(
			  const IocpCore& iocpCore_
			, NetAddress addr_
			, SessionFactory factory_
			, const Session::PacketHandleFunc* const global_packet_func
			, c_int32 maxSessionCount_ = 1);
		virtual ~ServerService();
	public:
		virtual bool Start()override;
		virtual void CloseService()override;
	public:
		const auto& GetListener()const noexcept { return m_pListener; }
	private:
		const S_ptr<Listener> m_pListener;
	};
}

