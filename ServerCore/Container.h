#pragma once
#include "ServerCorePch.h"
#include "PreDefinition.h"
#include "Allocator.h"

namespace ServerCore
{
	template <typename Type, uint32 Size>
	using Array = std::array <Type, Size>;

	template<typename Type>
	using XVector = std::vector<Type, StlAllocator<Type>>;

	template<typename Type>
	using XList = std::list<Type, StlAllocator<Type>>;

	template<typename Key, typename Type, typename Pred = std::less<Key>>
	using XMap = std::map<Key, Type, Pred, StlAllocator<std::pair<const Key, Type>>>;

	template<typename Key, typename Pred = std::less<Key>>
	using XSet = std::set<Key, Pred, StlAllocator<Key>>;

	template<typename Type>
	using XDeque = std::deque<Type, StlAllocator<Type>>;

	template<typename Type, typename Container = XDeque<Type>>
	using XQueue = std::queue<Type, Container>;

	template<typename Type, typename Container = XDeque<Type>>
	using XStack = std::stack<Type, Container>;

	template<typename Type, typename Container = XVector<Type>, typename Pred = std::less<typename Container::value_type>>
	using XPriorityQueue = std::priority_queue<Type, Container, Pred>;

	using XString = std::basic_string<char, std::char_traits<char>, StlAllocator<char>>;

	using XWString = std::basic_string<wchar_t, std::char_traits<wchar_t>, StlAllocator<wchar_t>>;

	template<typename Key, typename Type, typename Hasher = std::hash<Key>, typename KeyEq = std::equal_to<Key>>
	using XHashMap = std::unordered_map<Key, Type, Hasher, KeyEq, StlAllocator<std::pair<const Key, Type>>>;

	template<typename Key, typename Hasher = std::hash<Key>, typename KeyEq = std::equal_to<Key>>
	using XHashSet = std::unordered_set<Key, Hasher, KeyEq, StlAllocator<Key>>;

	template<typename T>
	using U_ptr = std::unique_ptr<T, UDeleter<T>>;

	template<typename T>
	using Us_ptr = std::unique_ptr<T, USDeleter<T>>;

	//template<typename T>
	//using S_ptr = std::shared_ptr<T>;

	template <typename T>
	using W_ptr = std::weak_ptr<T>;
}

using ServerCore::XVector;
using ServerCore::XList;
using ServerCore::XMap;
using ServerCore::XSet;
using ServerCore::XDeque;
using ServerCore::XQueue;
using ServerCore::XStack;
using ServerCore::XPriorityQueue;
using ServerCore::XString;
using ServerCore::XWString;
using ServerCore::XHashMap;
using ServerCore::XHashSet;
using ServerCore::U_ptr;
using ServerCore::Us_ptr;

// using ServerCore::S_ptr;
// using ServerCore::W_ptr;