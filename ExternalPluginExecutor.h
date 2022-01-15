#pragma once
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <string>
#include <vector>

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
	std::string getParamDataTransferFilePath(const std::string& fullPluginPath)
	{
		const std::string paramDataTransferFileName = "plugin-param-data-transfer.dat";
		std::string paramDataTransferFilePath;
		const auto lastBackslash = fullPluginPath.find_last_of('\\');
		if (lastBackslash != std::string::npos)
			paramDataTransferFilePath = fullPluginPath.substr(0, lastBackslash + 1);
		else
			paramDataTransferFilePath = ".\\";
		return paramDataTransferFilePath + paramDataTransferFileName;
	}

	std::string transferFile = "";
	std::string fullPluginPath;
	std::vector<ExternalPluginDto> output;
	std::vector<ExternalPluginDto> params;
public:
	ExternalPluginExecutor(const std::string & fullPluginPath) : fullPluginPath(fullPluginPath)
	{
		transferFile = getParamDataTransferFilePath(fullPluginPath);
	}

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

		std::ofstream ofs;
		ofs.open(transferFile, std::ios_base::out);
		for (const auto& dto : params)
			ofs << dto.serialize() << '\n';
		ofs.close();
		params.clear();

		const auto fullCmd = fullPluginPath + " \"" + transferFile + "\"";
		auto ret = system(fullCmd.c_str());
		if (ret != 0)
			return false;

		std::ifstream ifs;
		ifs.open(transferFile, std::ios_base::out);

		for (std::string s; std::getline(ifs, s);)
		{
			const ExternalPluginDto dto(s);
			if (dto.isOk())
				output.push_back(dto);
		}

		return true;
	}
};