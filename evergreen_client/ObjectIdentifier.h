#pragma once
#include "pch.h"
#include "ServerComponent.h"

class ObjectIdentifier
	:public ServerComponent
{
public:
	CONSTRUCTOR_SERVER_COMPONENT(ObjectIdentifier)
public:
	Nagox::Enum::GROUP_TYPE m_obj_group_type;
	uint8_t m_obj_type_info;
	std::string m_obj_name;
	uint32_t m_obj_id;
};

