#pragma once
#include <string>
#include <vector>
#include "ParamDto.h"

class PresetLoader
{
	struct Preset
	{
		std::string name;
		int wavePack;
		// parameter lines are kept as they were read so that rewriting the file doesn't lose precision
		std::vector<std::string> paramLines;
	};
	std::vector<Preset> presets;
	bool writePresets();
	static Preset makePreset(const std::string& name, const std::vector<ParamDTO>& params, int wavePack);
public:
	void readPresets();
	int count() const { return (int)presets.size(); }
	std::string getName(int presetIndex) const;
	std::vector<ParamDTO> loadPreset(int presetIndex, int *wavePack);
	// returns the index of the new preset or -1 if the file couldn't be written
	int savePreset(const std::string& name, const std::vector<ParamDTO>& params, int wavePack);
	bool overwritePreset(int presetIndex, const std::string& name, const std::vector<ParamDTO>& params, int wavePack);
	bool deletePreset(int presetIndex);
	std::string defaultName() const;
	PresetLoader();
	~PresetLoader();
};
