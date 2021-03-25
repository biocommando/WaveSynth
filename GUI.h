#pragma once

#include "aeffeditor.h"


class GUI : public AEffEditor
{
public:
	GUI(AudioEffect* ptr);
	~GUI();
	bool open(void* ptr);
	void close();
};

