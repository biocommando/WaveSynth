#include "MacroCache.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>

MacroCache::MacroCache()
{
	commandCache = NULL;
	titleCache = NULL;
}


MacroCache::~MacroCache()
{
	if (commandCache != NULL)
	{
		free(commandCache);
	}
	if (titleCache != NULL)
	{
		free(titleCache);
	}
}


void MacroCache::setCache(char * commands, long commandsSize, char * titles, long titlesSize)
{
	if (commandCache == NULL && titleCache == NULL)
	{
		commandSize = commandsSize;
		commandCache = (char*)malloc(commandSize);
		memcpy(commandCache, commands, commandSize);
		titleSize = titlesSize;
		titleCache = (char*)malloc(titleSize);
		memcpy(titleCache, titles, titleSize);
	}
}
long MacroCache::getCache(char ** commands, char ** titles)
{
	if (isCached())
	{
		*commands = (char*)realloc(*commands, commandSize);
		memcpy(*commands, commandCache, commandSize);
		*titles = (char*)realloc(*titles, titleSize);
		memcpy(*titles, titleCache, titleSize);
	}
	return commandSize;
}

bool MacroCache::isCached() 
{
	return commandCache != NULL && titleCache != NULL;
}
