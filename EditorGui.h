#pragma once

#include "aeffguieditor.h"
#include "WaveSynthConsts.h"
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include "MacroCache.h"

#define MENU_1_ID 1000
#define MENU_2_ID 1001
#define MENU_3_ID 1002
#define NUM_OF_MENUS 3
#define MACRO_COMMAND_LENGTH 500

class MacroMenu : public COptionMenu {
private:
	char *macroCommands;
	long macroCount;
public:
	//CMenuItem* addEntry(const char *entry, long index = -1L, long itemFlags = 0L) override;
	void addEntry(const char *entry, bool hasMacro = true);
	void doMacroEdits(AEffGUIEditor *editor, long lastTweakedTag);
	void saveToCache(MacroCache *cache);
	void loadFromCache(MacroCache *cache);

	MacroMenu(const CRect &size, CControlListener *listener, long tag);
	~MacroMenu();
};

class EditorGui : public AEffGUIEditor, public CControlListener
{
private:
	long lastTweakedTag;
	void SetValueLabelText(int index);
	void savePreset();
	bool checkBanks;
	MacroMenu *menu[NUM_OF_MENUS];
	MacroCache *menuCache[NUM_OF_MENUS];
	unsigned int presetGroupNum, presetNum;
	//void doMacroEdits(const char *command);
	void fillMacroMenu(MacroMenu *menu, MacroCache *cache, int tag);
public:
	EditorGui(void*);
	~EditorGui();

	void loadPresetNames();
	void loadPreset();

	// from AEffGUIEditor
	bool open(void* ptr);
	void close();
	void setParameter(VstInt32 index, float value);
	//virtual bool getRect(ERect **rect);

	// from CControlListener
	void valueChanged(CControl* pControl);
	void valueChanged(int index);

protected:
	CControl* controls[NUM_PARAMS];
	CTextLabel* valueLabels[NUM_PARAMS];
	CTextLabel* banksAndPatches[NUM_PARAM_SETS];
};
