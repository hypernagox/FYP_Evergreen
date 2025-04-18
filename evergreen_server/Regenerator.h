#pragma once
#include "NagoxDeleter.h"

class NagiocpX::Field;

class Regenerator
	:public NagiocpX::NagoxDeleter
{
public:
	virtual void ProcessDestroy(S_ptr<ContentsEntity> entity)noexcept override;
public:
	Regenerator(uint32_t duration, Vector3 summon_pos) :m_duration{ duration }, m_summon_pos{ summon_pos } {}
private:
	void RegenerateNPC(S_ptr<ContentsEntity> entity)noexcept;
	bool IsFieldRunning(S_ptr<ContentsEntity>& entity)noexcept;
public:
	S_ptr<NagiocpX::Field> m_targetField;
	const uint32_t m_duration;
	const Vector3  m_summon_pos;
};

