#include "GUI.h"


bool GUI::open(void* ptr)
{
	systemWindow = ptr;
	return true;
}

GUI::GUI(AudioEffect* ptr)
	: AEffEditor(ptr)
{

}

GUI::~GUI()
{
}

void GUI::close()
{
}