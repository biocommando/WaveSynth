#pragma once
#include <string>
#include <vector>
#include "ParamDto.h"

class PresetLoader
{
	std::vector<std::string> presets;
public:
	void readPresets();
	std::vector<std::string> getPresets() { return presets; }
	std::vector<ParamDTO> loadPreset(int presetIndex);
	std::string savePreset(std::vector<ParamDTO> params);
	PresetLoader();
	~PresetLoader();
};

