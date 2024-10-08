#include "ServerCorePch.h"
#include "ComponentSystem.h"
#include "ContentsComponent.h"

ComponentSystem::ComponentSystem(const std::atomic_bool& bValidFlag_) noexcept
	:m_bOwnerValidFlag{ bValidFlag_ }
{}

ComponentSystem::~ComponentSystem() noexcept
{
	for (const auto contents_comp : m_mapContentsComponents | std::views::values)ServerCore::xdelete<ContentsComponent>(contents_comp);
}

void ComponentSystem::Update(const float dt_) const noexcept
{
	auto b = m_vecUpdateComponents.data();
	const auto e = b + m_vecUpdateComponents.size();
	while (e != b) { (*b++)->Update(this, dt_); }
}
