#pragma once

#include<cstring>
#include<cstdlib>
#include<cstdio>

typedef struct {
	int intValue;
	float floatValue;
	char id[32];
	void create(const char* name, float value)
	{
		if (value < 0.0) value = 0;
		else if (value > 1.0) value = 1;
		floatValue = value;
		intValue = -1;
		strcpy(id, name);
	}
	/* The block (one dto) is saved into memory in the following way:
	type			count	description
	unsigned short	1		size of block, for forward compatibility
	char			1		type indication, currently f=float, i=int
	char			1		N, length of id excluding terminating zero
	char			N		id excluding terminating zero
	float			1		floatValue
	*/
	char* fromMem(char* mem)
	{
		char* ptr = mem;
		unsigned short blockSize = 0;
		memcpy(&blockSize, ptr, sizeof(unsigned short));
		ptr += sizeof(unsigned short);
		char type = *ptr;
		ptr++;
		char idLen = *ptr;
		ptr++;
		memcpy(id, ptr, idLen);
		id[idLen] = 0;
		ptr += idLen;
		memcpy(&floatValue, ptr, sizeof(float));
		if (type == 'i')
			intValue = (int)floatValue;
		else
			intValue = -1;
		return mem + blockSize;
	}
	unsigned int toMem(char** mem, unsigned int size)
	{
		int len = strlen(id);
		unsigned short blockSize = len + 2 + sizeof(double) + sizeof(unsigned short);
		unsigned int newSize = size + blockSize;
		*mem = (char*)realloc(*mem, newSize);
		char* buf = *mem + size;
		memcpy(buf, &blockSize, sizeof(unsigned short));
		buf += sizeof(unsigned short);
		if (intValue != -1)
		{
			buf[0] = 'i';
			floatValue = (float)intValue;
		}
		else
			buf[0] = 'f';
		buf[1] = (char)len;
		memcpy(&buf[2], id, len);
		memcpy(&buf[2 + len], &floatValue, sizeof(float));
		return newSize;
	}
	void fromString(const char* str)
	{
		char type = 0;
		intValue = -1;
		sscanf(str, "%c %s %f", &type, id, &floatValue);
		if (type == 'i') intValue = (int)floatValue;
	}
	void toString(char* str)
	{
		if (intValue >= 0)
			sprintf(str, "i %s %d", id, intValue);
		else
			sprintf(str, "f %s %.3f", id, floatValue);
	}
} ParamDTO;