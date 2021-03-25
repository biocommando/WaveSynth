#include"strrpl.h"

void putchbuf(char *buf, int size, int pos, char c)
{
	if (pos < size - 1)
		buf[pos] = c;
}

void strrpl(char *src, char *dst, int size, char *oldS, char *newS, int replaceAll)
{
	int srcLen = strlen(src), oldLen = strlen(oldS), newLen = strlen(newS);
	char *temp = (char*)malloc(size);
	strcpy(temp, src);
	int foundPos = 0;
	int dstPos = -1;
	int pos;
	for (pos = 0; pos < srcLen; pos++)
	{
		if (oldS[foundPos] == src[pos])
		{
			if (oldS[++foundPos] == 0)
			{
				int i;
				for (i = 0; i < newLen; i++)
					putchbuf(temp, size, ++dstPos, newS[i]);
				foundPos = 0;
				if (!replaceAll) break;
			}
		}
		else if (foundPos > 0)
		{
			while (foundPos >= 0)
			{
				putchbuf(temp, size, ++dstPos, src[pos - foundPos]);
				foundPos--;
			}
			foundPos = 0;
		}
		else
			putchbuf(temp, size, ++dstPos, src[pos]);
	}
	putchbuf(temp, size, ++dstPos, 0);
	temp[size - 1] = 0;
	strcpy(dst, temp);
	free(temp);
}