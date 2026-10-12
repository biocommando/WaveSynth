#pragma once
#include<cstdio>

class IniFileReader
{
private:
	FILE *file;
public:
	void readCharValue(const char *segment, const char *key, char *value);
	void readCharValueWithIndex(const char *segment, const char *key, int index, char *value);
	int readIntValue(const char *segment, const char *key);
	double readDoubleValue(const char *segment, const char *key);
	bool readBoolValue(const char *segment, const char *key);
	int openFile(const char *fileName);
	int openFile(FILE *f);
	int closeFile();

	IniFileReader();
	~IniFileReader();
};
