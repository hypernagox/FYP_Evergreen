#pragma once

enum class PROJECT_TYPE
{
	MAIN_SERVER,
	QUERY_SERVER,

	END
};

static constexpr const PROJECT_TYPE G_PROJECT = PROJECT_TYPE::QUERY_SERVER;
