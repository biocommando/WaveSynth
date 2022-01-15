#include "Plugins.h"
#include "ExternalPluginExecutor.h"

extern void WriteLog(const char*, double = 0);

PluginVariable createVariable(const std::string& name, double value = 0.0, char type = TYPE_TEMPORARY)
{
	return {
		type,
		name, 
		value
	};
}

std::vector<ParamDTO> ExecutePlugin(const std::string& filename, std::vector<PluginVariable>& initial)
{
	ExternalPluginExecutor executor(filename);
	for (const auto& param : initial)
	{
		executor.addParameter({ param.name, param.value });
	}
	if (!executor.execute())
		return {};

	std::vector<ParamDTO> dtos;
	for (const auto& execDto : executor.getOutput())
	{
		ParamDTO dto;
		auto name = execDto.getName();
		bool isIntParam = false;
		if (name.front() == '*')
		{
			name = name.substr(1);
			isIntParam = true;
		}
		dto.create(name.c_str(), execDto.getValue());
		if (isIntParam)
		{
			dto.intValue = static_cast<int>(execDto.getValue());
		}
		dtos.push_back(dto);
	}

	return dtos;
}

