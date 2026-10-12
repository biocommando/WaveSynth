#pragma once
#define DISPLAY_MAPPINGS_MAX 8
#define DISPLAY_MAPPINGS_LEN 8
class MinimalParameter
{
private:
	double scaling, addition;
	char displayMap[DISPLAY_MAPPINGS_MAX][DISPLAY_MAPPINGS_LEN];
	int displayCount;
public:
	char name[32], label[8];
	double value;
	int showAsInt, tag;
	MinimalParameter(const char *name, double defaultVal, double scaling, double addition, const char *label);
	~MinimalParameter();
	void GetDisplayText(char *out, int bsize);
	void GetDisplayText(char *out, int bsize, double value);
	int GetSelection(double value);
	void AddDisplayMapping (const char *mapping);
	void ClearDisplayMappings();
	int GetSelection();
	double SetSelection(int index);
	bool IsSelection();
};

