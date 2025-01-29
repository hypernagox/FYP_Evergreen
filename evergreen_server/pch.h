#pragma once
#define WIN32_LEAN_AND_MEAN 
#define NOMINMAX

#include <flatbuffers/flatbuffers.h>
#include <enum_generated.h>
#include <struct_generated.h>
#include <protocol_generated.h>
#include <CreateBuffer4Server.h>
#include <c2s_PacketHandler.h>

#include "ServerCorePch.h"
#include "protocol_define.h"
#include "ContentsComponent.h"
#include "PathManager.h"
#include <fstream>
#include <string_view>
#include <directxtk12/SimpleMath.h>
#include "ContentsFunc.h"



using Vector3 = DirectX::SimpleMath::Vector3;
using ServerCore::SendBuffer;
using ServerCore::rcast;
