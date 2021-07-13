#pragma once

#include "aeffguieditor.h"
#include "WaveSynthConsts.h"
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include "MacroCache.h"
#include "PresetLoader.h"

constexpr auto MENU_1_ID = 1000;
constexpr auto MENU_2_ID = 1001;
constexpr auto MENU_3_ID = 1002;
constexpr auto PRESET_MENU_ID = 2000;
constexpr auto NUM_OF_MENUS = 3;
constexpr auto MACRO_COMMAND_LENGTH = 500;

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
	PresetLoader presetLoader;
	bool checkBanks;
	MacroMenu *menu[NUM_OF_MENUS];
	COptionMenu *presetMenu;
	MacroCache *menuCache[NUM_OF_MENUS];
	unsigned int presetGroupNum, presetNum;
	//void doMacroEdits(const char *command);
	void fillMacroMenu(MacroMenu *menu, MacroCache *cache, int tag);
public:
	EditorGui(void*);
	~EditorGui();


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
