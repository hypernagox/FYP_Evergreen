#include "pch.h"
#include "LifeSpanObj.h"
#include "Queueabler.h"
#include "ClusterInfoHelper.h"

void LifeSpanObj::InitLifeTimer(const uint64_t life_time)
{
	auto entity = GetOwnerEntity();
	entity->GetQueueabler()->EnqueueAsyncTimer(life_time,
		&LifeSpanObj::TryOnDestroyOwner, this, std::move(entity));
}

void LifeSpanObj::TryOnDestroyOwner(S_ptr<ContentsEntity> entity)
{
	entity->GetComp<NagiocpX::ClusterInfoHelper>()->BroadcastAllCluster(Create_s2c_REMOVE_OBJECT(entity->GetObjectID()));
	entity->TryOnDestroy();
}
