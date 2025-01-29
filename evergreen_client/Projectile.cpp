#include "pch.h"
#include "Projectile.h"
#include "time_measure.h"
#include "scene_object.h"
#include "ServerObject.h"

void Projectile::Update() noexcept
{
	const auto pos = m_owner->GetTransform()->GetLocalPosition();
	m_owner->GetTransform()->SetLocalPosition(pos + m_speed * DT);
}
