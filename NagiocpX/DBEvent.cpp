#include "NagiocpXPch.h"
#include "DBEvent.h"
#include "DBMgr.h"

namespace NagiocpX
{
	void DBEvent::UnBind()noexcept
	{
		Mgr(DBMgr)->UnBind();
	}
}