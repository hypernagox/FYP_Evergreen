#pragma once
#include "ServerComponent.h"

class DropItem
	: public ServerComponent
{
	CONSTRUCTOR_SERVER_COMPONENT(DropItem)
public:
	virtual void Update()noexcept override;
public:
	void SetItemPos(const Vector3& pos);
	void SetMainHero(const std::shared_ptr<SceneObject>& hero_) { m_mainHero = hero_; }
private:
	Vector3 m_itemPos;
	std::shared_ptr<SceneObject> m_mainHero;
};

