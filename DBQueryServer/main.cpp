#include "pch.h"
#include "RecvBuffer.h"
#include "s2q_PacketHandler.h"
#include "DBMgr.h"
#include "ThreadMgr.h"
#include "Receiver.h"
#include "DBBindRAII.h"
#include "Procedures.h"

constexpr const int g_NUM_OF_THREADS = 2;

int main()
{
    NAGOX_ASSERT(Mgr(DBMgr)->Connect(g_NUM_OF_THREADS, L"DRIVER={ODBC Driver 18 for SQL Server};SERVER=(localdb)\\MSSQLLocalDB;DATABASE=localtest;Trusted_Connection=Yes;"));
   // NAGOX_ASSERT(Mgr(DBMgr)->Connect(g_NUM_OF_THREADS, L"DRIVER={ODBC Driver 17 for SQL Server};SERVER=HYPER-NAGOX-SAM\\SQLEXPRESS;DATABASE=localtest;Trusted_Connection=Yes;"));
    s2q_PacketHandler::Init();
    Mgr(ThreadMgr)->Launch(g_NUM_OF_THREADS);
    NAGOX_ASSERT(Mgr(Receiver)->Start(L"0.0.0.0", 8888));
    std::cout << "Start Query Server !" << std::endl;
    Mgr(Receiver)->DoRecv();
}