#pragma once
#include "ClientNetworkPch.h"

namespace NetHelper
{
	template <typename T, unsigned short Size_ = 32>
	class PacketArray
	{
	public:
		PacketArray()noexcept = default;
		PacketArray(const PacketArray& other)noexcept :m_size{ other.m_size }
		{
			if (0 != m_size) [[likely]]
				::memcpy(m_arr, other.m_arr, sizeof(T) * m_size);
		}
		T& operator=(const PacketArray& other)noexcept
		{
			if (this != &other)
			{
				m_size = other.m_size;
				if (0 != m_size) [[likely]]
					::memcpy(m_arr, other.m_arr, sizeof(T) * m_size);
			}
			return *this;
		}
		const auto data()noexcept { return m_arr; }
		const auto data()const noexcept { return m_arr; }
		const auto size()const noexcept { return m_size; }
		const auto begin()noexcept { return m_arr; }
		const auto end()noexcept { return m_arr + m_size; }
		const auto begin()const noexcept { return m_arr; }
		const auto end()const noexcept { return m_arr + m_size; }
		void SetSize(c_uint16 size_)noexcept { m_size = size_; }
		T& operator[](c_uint16 idx_)noexcept { return m_arr[idx_]; }
		const T& operator[](c_uint16 idx_)const noexcept { return m_arr[idx_]; }
	private:
		static inline constinit const unsigned short ARRAY_SIZE = Size_;
		T m_arr[ARRAY_SIZE];
		unsigned short m_size = 0;
	};

	template <unsigned short Size_ = 32>
	class PacketString
	{
	public:
		PacketString()noexcept = default;
		PacketString(std::string_view other)noexcept :m_size{ std::min(static_cast<c_uint16>(other.size()),ARRAY_SIZE) }
		{
			if (0 != m_size) [[likely]]
				::memcpy(m_arr, other.data(), sizeof(char) * m_size);
		}
		PacketString& operator=(std::string_view other)noexcept
		{
			m_size = std::min(static_cast<c_uint16>(other.size()), ARRAY_SIZE);
			if (0 != m_size) [[likely]]
				::memcpy(m_arr, other.data(), sizeof(char) * m_size);
			return *this;
		}
		PacketString(const PacketString& other)noexcept :m_size{ other.m_size }
		{
			if (0 != m_size) [[likely]]
				::memcpy(m_arr, other.m_arr, sizeof(char) * m_size);
		}
		PacketString& operator=(const PacketString& other)noexcept
		{
			if (this != &other)
			{
				m_size = other.m_size;
				if(0 != m_size) [[likely]]
					::memcpy(m_arr, other.m_arr, sizeof(char) * m_size);
			}
			return *this;
		}
		const auto data()noexcept { return m_arr; }
		const auto data()const noexcept { return m_arr; }
		const auto size()const noexcept { return m_size; }
		const auto begin()noexcept { return m_arr; }
		const auto end()noexcept { return m_arr + m_size; }
		const auto begin()const noexcept { return m_arr; }
		const auto end()const noexcept { return m_arr + m_size; }
		char& operator[](c_uint16 idx_)noexcept { return m_arr[idx_]; }
		const char& operator[](c_uint16 idx_)const noexcept { return m_arr[idx_]; }
		operator std::string()const noexcept { return std::string{ m_arr,m_arr + m_size }; }
		friend std::ostream& operator<<(std::ostream& os, const PacketString& p)noexcept {
			for (uint16 i = 0; i < p.m_size; ++i)
				os << p.m_arr[i];
			return os;
		}
	private:
		static inline constinit const unsigned short ARRAY_SIZE = Size_;
		char m_arr[ARRAY_SIZE];
		unsigned short m_size = 0;
	};
}