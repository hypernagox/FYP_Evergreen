#pragma once
#include "Session.h"

namespace ServerCore
{
	class PacketSession
		:public Session
	{
	public:
		PacketSession(const PacketHandleFunc* const sessionPacketHandler_)noexcept;
		PacketSession(const PacketHandleFunc* const sessionPacketHandler_, const bool bNeedConnect)noexcept;
		virtual ~PacketSession()noexcept = default;
		virtual const RecvStatus OnRecv(BYTE* const buffer, c_int32 len, const S_ptr<PacketSession>& pThisSessionPtr)noexcept override final;
		template<typename T = PacketSession>
		constexpr inline S_ptr<T> SharedFromThis()const noexcept { return S_ptr<T>{this}; }
		virtual void OnDisconnected(const ID_Ptr<ServerCore::Sector> curSectorInfo_)noexcept = 0;
		template <typename T = Sector>
		constexpr inline T* const GetCurSector()const noexcept { return GetOwnerEntity()->GetCurSector<T>(); }
	protected:
		virtual void OnConnected() = 0;
		virtual void OnSend(c_int32 len)noexcept override {}
	};
}
