#pragma once
#include<cstdio>

class IniFileReader
{
private:
	FILE *file;
public:
	void readCharValue(char *segment, char *key, char *value);
	void readCharValueWithIndex(char *segment, char *key, int index, char *value);
	int readIntValue(char *segment, char *key);
	double readDoubleValue(char *segment, char *key);
	bool readBoolValue(char *segment, char *key);
	int openFile(char *fileName);
	int openFile(FILE *f);
	int closeFile();

	IniFileReader();
	~IniFileReader();
};

