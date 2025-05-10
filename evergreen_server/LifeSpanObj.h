#pragma once

class LifeSpanObj
	:public ContentsComponent
{
	CONSTRUCTOR_CONTENTS_COMPONENT(LifeSpanObj)

public:
	void InitLifeTimer(const uint64_t life_time);
private:
	void TryOnDestroyOwner(S_ptr<ContentsEntity> entity);
};

