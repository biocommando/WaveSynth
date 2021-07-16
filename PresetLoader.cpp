#include "PresetLoader.h"
#include "WaveSynthConsts.h"
#include <cstdio>
#include <cstdlib>
#include <fstream>
extern void WriteLog(const char* msg, double val = 0);
std::string getPresetFilename()
{
	extern void getWorkDir(char*);
	char filename[1024];
	getWorkDir(filename);
	sprintf(filename, "%spresets\\presets.dat", filename);
	
	return std::string(filename);
}

std::vector<ParamDTO> PresetLoader::loadPreset(int presetIndex, int *wavePack)
{
	std::vector<ParamDTO> res;

	std::ifstream f;
	f.open(getPresetFilename());
	std::string s;
	int idx = -1;
	while (std::getline(f, s))
	{
		auto pos = s.find_first_of('$');
		if (pos == std::string::npos)
			continue;
		idx++;
		if (idx == presetIndex)
		{
			std::getline(f, s);
			*wavePack = std::stoi(s);
			std::getline(f, s);
			auto numParams = std::stoi(s);
			for (int i = 0; i < numParams; i++)
			{
				std::getline(f, s);
				ParamDTO dto;
				dto.fromString(s.c_str());
				res.push_back(dto);
			}
		}
	}
	f.close();
	return res;
}

std::string PresetLoader::savePreset(std::vector<ParamDTO> params, int wavePack)
{
	std::ofstream f;
	f.open(getPresetFilename(), std::ios_base::app | std::ios_base::out);
	auto idx = presets.size() + 1;
	std::string presetName = "Preset #" + std::to_string(idx);
	presets.push_back(presetName);
	f << "$" << presetName << std::endl;
	f << std::to_string(wavePack) << std::endl;
	f << std::to_string(params.size()) << std::endl;
	for (unsigned i = 0; i < params.size(); i++)
	{
		char str[100];
		params[i].toString(str);
		f << str << std::endl;
	}
	f.close();
	return presetName;
}

PresetLoader::PresetLoader()
{
}


PresetLoader::~PresetLoader()
{
}

void PresetLoader::readPresets()
{
	presets.clear();
	std::ifstream f;
	f.open(getPresetFilename());
	std::string s;
	while (std::getline(f, s))
	{
		auto pos = s.find_first_of('$');
		if (pos == std::string::npos)
			continue;
		presets.push_back(s.substr(pos + 1));
	}
	f.close();
}
