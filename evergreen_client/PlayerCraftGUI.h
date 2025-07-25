#pragma once

#include "pch.h"

class AuthenticPlayer;

class PlayerCraftGUI : public udsdx::Component
{
private:
	struct RecipeGUI
	{
		std::shared_ptr<udsdx::SceneObject> Panel;
		std::shared_ptr<udsdx::SceneObject> OutputSlotContents;
		std::shared_ptr<udsdx::SceneObject> OutputNameText;
		std::shared_ptr<udsdx::SceneObject> CraftButton;
		std::shared_ptr<udsdx::SceneObject> CraftButtonText;
		std::vector<std::shared_ptr<udsdx::SceneObject>> InputSlotBackground;
		std::vector<std::shared_ptr<udsdx::SceneObject>> InputSlotContents;
		std::vector<std::shared_ptr<udsdx::SceneObject>> InputSlotText;
	};

public:
	void OnInitialize() override;
	void UpdateSlotContents(AuthenticPlayer* target, const std::vector<int>& table);

private:
	std::shared_ptr<udsdx::SceneObject> m_panel;
	std::vector<RecipeGUI> m_recipePanels;
};
