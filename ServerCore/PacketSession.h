#pragma once
#include "Session.h"

namespace ServerCore
{
	class PacketSession
		:public Session
	{
	public:
		PacketSession()noexcept = default;
		PacketSession(const bool bNeedConnect)noexcept;
		virtual ~PacketSession()noexcept = default;
		virtual const RecvStatus OnRecv(BYTE* const buffer, c_int32 len, const S_ptr<PacketSession>& pThisSessionPtr)noexcept override final;
		template<typename T = PacketSession>
		constexpr inline S_ptr<T> SharedFromThis()const noexcept { return S_ptr<T>{this}; }
		virtual void OnDisconnected(const ServerCore::Cluster* const curCluster_)noexcept = 0;
		template <typename T = Cluster>
		constexpr inline T* const GetCurCluster()const noexcept { return static_cast<T*>(GetOwnerEntity()->GetCurCluster()); }
	protected:
		virtual void OnConnected() = 0;
		virtual void OnSend(c_int32 len)noexcept override {}
	};
}
