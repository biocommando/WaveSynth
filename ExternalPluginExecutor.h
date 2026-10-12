#pragma once
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <string>
#include <vector>
#include <windows.h>

class ExternalPluginDto
{
	std::string name;
	double value;
	bool ok;
public:
	ExternalPluginDto(const std::string& name, double value)
		: name(name), value(value), ok(true)
	{}

	ExternalPluginDto(const std::string& serialized) : value(0), name(""), ok(false)
	{
		try
		{
			if (serialized == "" || serialized[0] == '#')
				return;
			const auto space_idx = serialized.find(' ');
			if (space_idx == std::string::npos)
				return;
			name = serialized.substr(0, space_idx);
			value = std::stod(serialized.substr(space_idx + 1));
			ok = true;
		}
		catch (const std::exception & e)
		{
			// pass; failure will be reported in ok flag
		}
	}

	std::string serialize() const
	{
		return name + " " + std::to_string(value);
	}

	std::string getName() const
	{
		return name;
	}

	double getValue() const
	{
		return value;
	}

	bool isOk() const
	{
		return ok;
	}
};

class ExternalPluginExecutor
{
private:
	// Signature of the function exported by DLL plugins. The buffer holds consecutive
	// null-terminated "name value" strings: the input parameters on entry and the
	// output parameters on return. Returns the number of output strings or -1 on failure.
	typedef int (*exec_plugin_func)(char*, size_t);
	// Large enough for the maximum number of parameters a plugin supports (1000)
	static constexpr size_t dllBufferSize = 256 * 1024;

	std::string fullPluginPath;
	std::vector<ExternalPluginDto> output;
	std::vector<ExternalPluginDto> params;
public:
	ExternalPluginExecutor(const std::string & fullPluginPath) : fullPluginPath(fullPluginPath)
	{}

	std::vector<ExternalPluginDto> getOutput()
	{
		return output;
	}

	void addParameter(const ExternalPluginDto& dto)
	{
		params.push_back(dto);
	}

	bool execute()
	{
		output = {};

		std::string buffer;
		for (const auto& dto : params)
			buffer += dto.serialize() + '\0';
		params.clear();
		if (buffer.size() >= dllBufferSize)
			return false;
		buffer.resize(dllBufferSize);

		HMODULE dll = LoadLibraryA(fullPluginPath.c_str());
		if (!dll)
			return false;
		const auto execPlugin = (exec_plugin_func)GetProcAddress(dll, "exec_plugin");
		if (!execPlugin)
		{
			FreeLibrary(dll);
			return false;
		}
		// The DLL is unloaded after every call so that each run starts with fresh global state
		const int count = execPlugin(&buffer[0], buffer.size());
		FreeLibrary(dll);
		if (count < 0)
			return false;

		size_t offset = 0;
		for (int i = 0; i < count && offset < buffer.size(); i++)
		{
			const char* s = buffer.c_str() + offset;
			const ExternalPluginDto dto(s);
			if (dto.isOk())
				output.push_back(dto);
			offset += strlen(s) + 1;
		}
		return true;
	}
};