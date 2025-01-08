#include "pch.h"
#include "ServerSession.h"
#include "s2c_DummyPacketHandler.h"
#include "CreateBuffer4Dummy.h"

extern Vector3 O_VEC3(const Nagox::Struct::Vec3* const v);
extern Nagox::Struct::Vec3 F_VEC3(const Vector3& v);

ServerSession::ServerSession()
	:ServerCore::PacketSession{ s2c_DummyPacketHandler::GetPacketHandlerList(),true }
{
}

ServerSession::~ServerSession()
{
}

void ServerSession::OnConnected()
{
	std::cout << "Im Dummy !" << std::endl;
	SendAsync(Create_c2s_LOGIN("Hello"));
}

void ServerSession::OnSend(c_int32 len)noexcept
{
}

void ServerSession::OnDisconnected(const ServerCore::Cluster* const curCluster_)noexcept
{
	IncRef();
	std::cout << "DisConnect !" << std::endl;
}

void ServerSession::StartMovePacket(const uint64_t delay_time) noexcept
{
	UpdateMove();

	SendAsync(Create_c2s_MOVE(F_VEC3(pos), {}, {}, {}, ::GetTickCount64()));

	Mgr(TaskTimerMgr)->ReserveAsyncTask(delay_time, &ServerSession::SendMovePacketRoutine, SharedFromThis<ServerSession>(), uint64_t{ delay_time });
}

void ServerSession::SendMovePacketRoutine(const uint64_t delay_time) noexcept
{
	UpdateMove();

	SendAsync(Create_c2s_MOVE(F_VEC3(pos), {}, {}, {}, ::GetTickCount64()));

	Mgr(TaskTimerMgr)->ReserveAsyncTask(delay_time, &ServerSession::SendMovePacketRoutine, SharedFromThis<ServerSession>(), uint64_t{ delay_time });

	if (m_moveCount % m_print_count == 0 && rand() & 1)
	{
		std::cout << std::format("ID: {}, Delay Avg: {:.3f}ms \n", m_id, GetDelayAvg());
	}
}

void ServerSession::UpdateMove() noexcept
{
	++m_moveCount;
	// TODO: 적당히 랜덤무브
}
