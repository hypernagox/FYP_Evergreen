#include "pch.h"
#include "RecvBuffer.h"
#include "DBPacket.h"
#include "s2q_PacketHandler.h"
#include "DBMgr.h"
#include "ThreadMgr.h"
#include "Receiver.h"
#include "DBBindRAII.h"
#include "pch.h"
#include "DBPacket.h"
#include "s2q_PacketHandler.h"
#include "DBMgr.h"
#include "DBBindRAII.h"
#include "Procedures.h"

constexpr const int g_NUM_OF_THREADS = 2;

int main()
{
    NAGOX_ASSERT(Mgr(DBMgr)->Connect(g_NUM_OF_THREADS, L"DRIVER={ODBC Driver 18 for SQL Server};SERVER=(localdb)\\MSSQLLocalDB;DATABASE=localtest;Trusted_Connection=Yes;"));

   {
       InsertUser u;
       u.In_Id(L"Hello");
       u.In_Pw(L"World");
       u.In_Type(L"Warrior");
       u.In_UID(10);
       u.Execute();
   }
   {
       GetUserById u;
      
       int uid = 1;
       WCHAR id[100] = {};
       WCHAR pw[100] = {};
       WCHAR type[100] = {};
       u.In_Id(L"Hello");
       u.Out_Id(id);
       u.Out_Pw(pw);
       u.Out_Type(type);
       u.Out_UID(uid);
     
       u.Execute();
       while (u.Fetch()) { std::cout << "!"; }
       int a = 10;
   }
    Mgr(ThreadMgr)->Launch(g_NUM_OF_THREADS);
    NAGOX_ASSERT(Mgr(Receiver)->Start(L"0.0.0.0", 8888));
    std::cout << "Start Query Server !" << std::endl;
    Mgr(Receiver)->DoRecv();
}