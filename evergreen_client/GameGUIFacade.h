#pragma once

#include "pch.h"

class QuestGUI;
class LogFloatGUI;
class RequestPopupGUI;
class PartyStatusGUI;
class DamageCountGUI;
class TransitionOverlayGUI;
class PopupGUIManager;

class GameGUIFacade
{
public:
	GameGUIFacade() { }

public:
	QuestGUI* QuestGUI = nullptr;
	LogFloatGUI* LogFloat = nullptr;
	RequestPopupGUI* RequestPopup = nullptr;
	PartyStatusGUI* PartyStatus = nullptr;
	DamageCountGUI* DamageCount = nullptr;
	TransitionOverlayGUI* TransitionOverlay = nullptr;
	PopupGUIManager* PopupManager = nullptr;
};

