#pragma once

#include <string>
#include <algorithm>

class ArgumentParser
{
public:
	int argc;
	char** argv;

	ArgumentParser(int argc = 0, char** argv = nullptr)
		: argc(argc)
		, argv(argv)
	{
	}

	int GetPosition(const std::string& shortArg = "", const std::string& longArg = "")
	{
		for (int i = 0; i < argc; i++)
		{
			std::string current(argv[i]);
			if ((shortArg != "" && current == shortArg)
				|| (current != "" && current == longArg))
			{
				return i;
			}
		}
		return -1;
	}

	bool CheckIfExists(const std::string& shortArg = "", const std::string& longArg = "")
	{
		return GetPosition(shortArg, longArg) != -1;
	}

	std::string GetString(const std::string& shortArg = "", const std::string& longArg = "", const std::string& defaultValue = "")
	{
		int pos = GetPosition(shortArg, longArg);
		if (pos == -1 || pos == argc)
		{
			return defaultValue;
		}
		return std::string(argv[pos + 1]);
	}

	int GetInt(const std::string& shortArg = "", const std::string& longArg = "", int defaultValue = -1, int base = 10)
	{
		int pos = GetPosition(shortArg, longArg);
		if (pos == -1 || pos == argc)
		{
			return defaultValue;
		}
		return std::stoi(std::string(argv[pos + 1]), nullptr, base);
	}

	float GetFloat(const std::string& shortArg = "", const std::string& longArg = "", float defaultValue = -1.0f)
	{
		int pos = GetPosition(shortArg, longArg);
		if (pos == -1 || pos == argc)
		{
			return defaultValue;
		}
		return std::stof(std::string(argv[pos + 1]));
	}

	float GetBool(const std::string& shortArg = "", const std::string& longArg = "", bool defaultValue = false)
	{
		int pos = GetPosition(shortArg, longArg);
		if (pos == -1 || pos == argc)
		{
			return defaultValue;
		}
		std::string value(argv[pos + 1]);
		std::transform(value.begin(), value.end(), value.begin(), ::tolower);
		return value == "true" || value == "1";
	}
};

static int GetPosition(int argc, char** argv, const std::string& shortArgument, const std::string& longArgument = "")
{
	for (int i = 0; i < argc; i++)
	{
		std::string current(argv[i]);
		if (current == shortArgument || (current != "" && current == longArgument))
		{
			return i;
		}
	}
	return -1;
}

static bool CheckIfExists(int argc, char** argv, const std::string& shortArgument, const std::string& longArgument = "")
{
	return GetPosition(argc, argv, shortArgument, longArgument) != -1;
}