#pragma once

#define WIN32_LEAN_AND_MEAN // ���� ������ �ʴ� ������ Windows ������� �����մϴ�.
#define NOMINMAX

#include <flatbuffers/flatbuffers.h>

#include <enum_generated.h>
#include <struct_generated.h>
#include <protocol_generated.h>
#include <s2c_PacketHandler.h>
#include <CreateBuffer4Client.h>
#include <protocol_define.h>

#include <StateMachine.hpp>
#include <StateTransition.hpp>

#include <ClientNetworkPch.h>

#include <PathManager.h>

#include <updown_studio.h>

#include "define.h"
#include "func.h"