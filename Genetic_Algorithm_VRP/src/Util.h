#pragma once
#include <string>

namespace Util
{
	bool StartsWith(const std::string& original, const std::string& value);
	size_t FindNextNonWhitespace(const std::string& original, size_t offset = 0U);

	template<typename T>
	T Clamp(T value, T min, T max)
	{
		if (value < min)
		{
			return min;
		}
		if (value > max)
		{
			return max;
		}
		return value;
	}
}