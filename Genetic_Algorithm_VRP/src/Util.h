#pragma once

namespace Util
{
	bool StartsWith(const std::string& original, const std::string& value);
	size_t FindNextNonWhitespace(const std::string& original, size_t offset = 0U);
}