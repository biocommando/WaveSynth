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
	MinimalParameter(char *name, double defaultVal, double scaling, double addition, char *label);
	~MinimalParameter();
	void GetDisplayText(char *out, int bsize);
	void AddDisplayMapping (char *mapping);
	void ClearDisplayMappings();
	int GetSelection();
	double SetSelection(int index);
	bool IsSelection();
};

