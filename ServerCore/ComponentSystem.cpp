#include "ServerCorePch.h"
#include "ComponentSystem.h"
#include "ContentsComponent.h"

ComponentSystem::ComponentSystem(const NagoxAtomic::Atomic<bool>& bValidFlag_) noexcept
	:m_bOwnerValidFlag{ bValidFlag_ }
{}

ComponentSystem::~ComponentSystem() noexcept
{
	auto b = m_contentsComponents.data();
	const auto e = b + m_contentsComponents.size();
	while (e != b) {
		ServerCore::xdelete<ContentsComponent>((*b++).second);
	}
}

void ComponentSystem::Update(const float dt_) const noexcept
{
	auto b = m_vecUpdateComponents.data();
	const auto e = b + m_vecUpdateComponents.size();
	while (e != b) { (*b++)->Update(this, dt_); }
}
