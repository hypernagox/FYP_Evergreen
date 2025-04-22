#pragma once

#include "pch.h"

class PartyListGUI;
class LogFloatGUI;
class RequestPopupGUI;
class PartyStatusGUI;

class GameGUIFacade
{
public:
	GameGUIFacade() { }

public:
	PartyListGUI* PartyList = nullptr;
	LogFloatGUI* LogFloat = nullptr;
	RequestPopupGUI* RequestPopup = nullptr;
	PartyStatusGUI* PartyStatus = nullptr;
};

