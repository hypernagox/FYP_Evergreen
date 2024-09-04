#pragma once

#include <iostream>
#include <filesystem>

#define RESOURCE_PATH(RELATIVE) (PathManager::GetInstance()->GetResourcePath(RELATIVE))

class PathManager
{
public:
	PathManager();
	~PathManager();

	static PathManager* GetInstance();

	std::wstring GetResourcePath(std::wstring_view path) const;

private:
	static constexpr const wchar_t* INI_PATH = L".\\path.ini";

	std::filesystem::path m_resourcePrefix;
};