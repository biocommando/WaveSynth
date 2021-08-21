#include "IniFileReader.h"
#include <cstring>


IniFileReader::IniFileReader() : file(NULL)
{
}


IniFileReader::~IniFileReader()
{
	closeFile();
}


extern void WriteLog(const char*, double);

void IniFileReader::readCharValue(char *segment, char *key, char *value)
{
	if (file == NULL) return;

	fseek(file, 0, SEEK_SET);
	char s[256] = "";
	int segmentFound = 0;
	while (!feof(file)) {
		int i; 
		if (s[0] != '#')
		{
			char *xSegment = NULL, *xKey = s, *xValue = NULL;
			for (i = 0; i < 256; i++) {
				char c = s[i];
				if (c == 0) break;
				if (c == '[' && i == 0) {
					xSegment = &s[i + 1];
					segmentFound = 0;
				}
				else if (c == ']' && xSegment != NULL) {
					s[i] = 0;
					if (!strcmp(segment, xSegment)) {
						segmentFound = 1;
					}
					break;
				}
				else if (xSegment != NULL || segmentFound) {
					if (c == '=') {
						s[i] = 0;
						if (!strcmp(key, xKey)) {
							xValue = &s[i + 1];
						}
					}
					else if (c == '\n') {
						s[i] = 0;
					}
				}
			}
			if (xValue != NULL) {
				strcpy(value, xValue);
				break;
			}
		}
		fgets(s, 256, file);
	}
}

void IniFileReader::readCharValueWithIndex(char *segment, char *key, int index, char *value)
{
	char keyWithIndex[256];
	sprintf(keyWithIndex, "%s%d", key, index);
	readCharValue(segment, keyWithIndex, value);
}

int IniFileReader::readIntValue(char *segment, char *key)
{
	char value[256] = "";
	readCharValue(segment, key, value);
	int iValue = 0;
	sscanf(value, "%d", &iValue);
	return iValue;
}

double IniFileReader::readDoubleValue(char *segment, char *key)
{
	char value[256] = "";
	readCharValue(segment, key, value);
	double dValue = 0;
	sscanf(value, "%lf", &dValue);
	return dValue;
}

bool IniFileReader::readBoolValue(char *segment, char *key)
{
	char value[256] = "";
	readCharValue(segment, key, value);
	if (!strcmp(value, "true") || !strcmp(value, "True") || !strcmp(value, "TRUE") || !strcmp(value, "1")) {
		return true;
	}
	return false;
}

int IniFileReader::openFile(char *fileName)
{
	file = fopen(fileName, "r");
	if (file != NULL)
	{
		return 0;
	}
	return 1;
}

int IniFileReader::openFile(FILE *f)
{
	file = f;
	if (file != NULL)
	{
		return 0;
	}
	return 1;
}

int IniFileReader::closeFile()
{
	if (file == NULL) return 0;
	return fclose(file);
}