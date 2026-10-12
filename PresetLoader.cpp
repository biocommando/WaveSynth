#include "PresetLoader.h"
#include "WaveSynthConsts.h"
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <windows.h>
extern void WriteLog(const char* msg, double val = 0);
std::string getPresetFilename()
{
	extern void getWorkDir(char*);
	char filename[1024];
	getWorkDir(filename);
	sprintf(filename, "%spresets\\presets.dat", filename);

	return std::string(filename);
}

static std::string stripLineEnd(std::string s)
{
	while (!s.empty() && (s.back() == '\r' || s.back() == '\n'))
		s.pop_back();
	return s;
}

static std::string sanitizeName(const std::string& name)
{
	std::string res;
	for (char c : name)
		if (c != '\r' && c != '\n')
			res += c;
	auto first = res.find_first_not_of(' ');
	if (first == std::string::npos)
		return "";
	return res.substr(first, res.find_last_not_of(' ') - first + 1);
}

std::string PresetLoader::getName(int presetIndex) const
{
	if (presetIndex < 0 || presetIndex >= count())
		return "";
	return presets[presetIndex].name;
}

std::vector<ParamDTO> PresetLoader::loadPreset(int presetIndex, int *wavePack)
{
	std::vector<ParamDTO> res;
	if (presetIndex < 0 || presetIndex >= count())
		return res;
	auto& preset = presets[presetIndex];
	*wavePack = preset.wavePack;
	for (auto& line : preset.paramLines)
	{
		ParamDTO dto;
		dto.fromString(line.c_str());
		res.push_back(dto);
	}
	return res;
}

PresetLoader::Preset PresetLoader::makePreset(const std::string& name, const std::vector<ParamDTO>& params, int wavePack)
{
	Preset preset;
	preset.name = name;
	preset.wavePack = wavePack;
	for (auto dto : params)
	{
		char str[100];
		dto.toString(str);
		preset.paramLines.push_back(str);
	}
	return preset;
}

std::string PresetLoader::defaultName() const
{
	return "Preset #" + std::to_string(presets.size() + 1);
}

int PresetLoader::savePreset(const std::string& name, const std::vector<ParamDTO>& params, int wavePack)
{
	auto cleanName = sanitizeName(name);
	if (cleanName.empty())
		cleanName = defaultName();
	presets.push_back(makePreset(cleanName, params, wavePack));
	if (!writePresets())
	{
		presets.pop_back();
		return -1;
	}
	return count() - 1;
}

bool PresetLoader::overwritePreset(int presetIndex, const std::string& name, const std::vector<ParamDTO>& params, int wavePack)
{
	if (presetIndex < 0 || presetIndex >= count())
		return false;
	auto cleanName = sanitizeName(name);
	if (cleanName.empty())
		cleanName = presets[presetIndex].name;
	auto old = presets[presetIndex];
	presets[presetIndex] = makePreset(cleanName, params, wavePack);
	if (!writePresets())
	{
		presets[presetIndex] = old;
		return false;
	}
	return true;
}

bool PresetLoader::deletePreset(int presetIndex)
{
	if (presetIndex < 0 || presetIndex >= count())
		return false;
	auto old = presets[presetIndex];
	presets.erase(presets.begin() + presetIndex);
	if (!writePresets())
	{
		presets.insert(presets.begin() + presetIndex, old);
		return false;
	}
	return true;
}

// Writes to a temporary file first and then replaces presets.dat so that a failed write can't destroy the presets
bool PresetLoader::writePresets()
{
	auto filename = getPresetFilename();
	auto tempFilename = filename + ".tmp";
	{
		std::ofstream f(tempFilename, std::ios_base::out | std::ios_base::trunc);
		if (!f)
			return false;
		for (auto& preset : presets)
		{
			f << "$" << preset.name << std::endl;
			f << preset.wavePack << std::endl;
			f << preset.paramLines.size() << std::endl;
			for (auto& line : preset.paramLines)
				f << line << std::endl;
		}
		f.close();
		if (f.fail())
		{
			DeleteFileA(tempFilename.c_str());
			return false;
		}
	}
	if (!MoveFileExA(tempFilename.c_str(), filename.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
	{
		WriteLog("Failed to replace preset file", (double)GetLastError());
		DeleteFileA(tempFilename.c_str());
		return false;
	}
	return true;
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
		Preset preset;
		preset.name = stripLineEnd(s.substr(pos + 1));
		preset.wavePack = 0;
		if (!std::getline(f, s))
			break;
		preset.wavePack = std::atoi(s.c_str());
		if (!std::getline(f, s))
			break;
		auto numParams = std::atoi(s.c_str());
		for (int i = 0; i < numParams && std::getline(f, s); i++)
			preset.paramLines.push_back(stripLineEnd(s));
		presets.push_back(preset);
	}
	f.close();
}
