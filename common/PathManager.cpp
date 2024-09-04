#include "PathManager.h"
#include <Windows.h>

PathManager::PathManager()
{
	std::wstring pResourcePath;
	pResourcePath.resize(MAX_PATH);

	GetPrivateProfileString(L"prefix", L"resource", nullptr, pResourcePath.data(), MAX_PATH, INI_PATH);

	// reduce the size of the string to the actual length
	pResourcePath.resize(wcslen(pResourcePath.data()));
	m_resourcePrefix.assign(pResourcePath);
}

PathManager::~PathManager()
{

}

PathManager* PathManager::GetInstance()
{
	static PathManager instance;
	return &instance;
}

std::wstring PathManager::GetResourcePath(std::wstring_view path) const
{
	std::wstring fullPath = m_resourcePrefix;
	fullPath += path;
	return fullPath;
}
