#pragma once
#include "NetAddress.h"

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

	using SessionFactory = std::function<S_ptr<Session>(void)>;

	class Service
	{
		struct alignas(64) AtomicSessionPtr
		{
			AtomicS_ptr<ContentsEntity> ptr;
		};
	public:
		Service(const IocpCore& iocpCore_, SERVICE_TYPE eServiceType_, NetAddress addr_, SessionFactory factory_, c_int32 maxSessionCount_ = 1);
		virtual ~Service();

	public:
		virtual bool Start() = 0;
		virtual void CloseService();
		bool CanStart()const { return nullptr != m_sessionFactory; }
		S_ptr<Session> CreateSession()noexcept;
		const bool AddSession(const S_ptr<PacketSession>& pSession_)noexcept;
		void ReleaseSession(const Session* const pSession_)noexcept;
		int32 GetMaxSessionCount()const noexcept { return m_maxSessionCount; }
		SERVICE_TYPE GetServiceType()const noexcept { return m_eServiceType; }
		NetAddress GetNetAddress()const noexcept { return m_netAddr; }
		const IocpCore& GetIocpCore()const noexcept { return m_iocpCore; }
		S_ptr<ContentsEntity> GetSession(const uint64_t sessionID_)const noexcept;
	public:
		void IterateSession(std::function<void(Session* const)> fpIterate_)noexcept;
	protected:
		mutable tbb::concurrent_unordered_map<uint32_t, uint16_t> m_id2Index;
		const std::span<AtomicSessionPtr> m_arrSession;
		tbb::concurrent_bounded_queue<int32> m_idxQueue;
		//tbb::concurrent_bounded_queue<int32,StlAllocator64<int32>> m_idxQueue;
		const IocpCore& m_iocpCore;
		const SERVICE_TYPE m_eServiceType;
		const NetAddress m_netAddr;
		const SessionFactory m_sessionFactory;
		const int32 m_maxSessionCount;

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
		ClientService(const IocpCore& iocpCore_, NetAddress targetServerAddr_, SessionFactory factory_, c_int32 maxSessionCount_ = 1);
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
		ServerService(const IocpCore& iocpCore_, NetAddress addr_, SessionFactory factory_, c_int32 maxSessionCount_ = 1);
		virtual ~ServerService();
	public:
		virtual bool Start()override;
		virtual void CloseService()override;
	private:
		const S_ptr<Listener> m_pListener;
	};
}

