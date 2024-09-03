#pragma once
#include "ServerCorePch.h"
#include "Types.h"
#include "Allocator.h"

namespace ServerCore
{
	template <typename Type, uint32 Size>
	using Array = std::array <Type, Size>;

	template<typename Type>
	using Vector = std::vector<Type, StlAllocator<Type>>;

	template<typename Type>
	using List = std::list<Type, StlAllocator<Type>>;

	template<typename Key, typename Type, typename Pred = std::less<Key>>
	using Map = std::map<Key, Type, Pred, StlAllocator<std::pair<const Key, Type>>>;

	template<typename Key, typename Pred = std::less<Key>>
	using Set = std::set<Key, Pred, StlAllocator<Key>>;

	template<typename Type>
	using Deque = std::deque<Type, StlAllocator<Type>>;

	template<typename Type, typename Container = Deque<Type>>
	using Queue = std::queue<Type, Container>;

	template<typename Type, typename Container = Deque<Type>>
	using Stack = std::stack<Type, Container>;

	template<typename Type, typename Container = Vector<Type>, typename Pred = std::less<typename Container::value_type>>
	using PriorityQueue = std::priority_queue<Type, Container, Pred>;

	using String = std::basic_string<char, std::char_traits<char>, StlAllocator<char>>;

	using WString = std::basic_string<wchar_t, std::char_traits<wchar_t>, StlAllocator<wchar_t>>;

	template<typename Key, typename Type, typename Hasher = std::hash<Key>, typename KeyEq = std::equal_to<Key>>
	using HashMap = std::unordered_map<Key, Type, Hasher, KeyEq, StlAllocator<std::pair<const Key, Type>>>;

	template<typename Key, typename Hasher = std::hash<Key>, typename KeyEq = std::equal_to<Key>>
	using HashSet = std::unordered_set<Key, Hasher, KeyEq, StlAllocator<Key>>;

	template<typename T>
	using U_ptr = std::unique_ptr<T, UDeleter<T>>;

	template<typename T>
	using Us_ptr = std::unique_ptr<T, USDeleter<T>>;

	//template<typename T>
	//using S_ptr = std::shared_ptr<T>;

	template <typename T>
	using W_ptr = std::weak_ptr<T>;
}