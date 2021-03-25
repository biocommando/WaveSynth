#pragma once

class MacroCache
{
private:
	char *commandCache, *titleCache;
	long commandSize, titleSize;
public:
	void setCache(char * commands, long commandsSize, char * titles, long titlesSize);
	long getCache(char ** commands, char ** titles);
	bool isCached();
	MacroCache();
	~MacroCache();
};

