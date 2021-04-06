#include <string>

namespace Util
{
	bool StartsWith(const std::string& original, const std::string& value)
	{
		for (size_t i = 0; i < value.size(); i++)
		{
			if (original[i] != value[i])
			{
				return false;
			}
		}
		return true;
	}

	size_t FindNextNonWhitespace(const std::string& original, size_t offset)
	{
		for (size_t i = offset + 1; i < original.size(); i++)
		{
			if (original[i] != ' ' && original[i] != '\t')
			{
				return i;
			}
		}
		return std::string::npos;
	}
}
