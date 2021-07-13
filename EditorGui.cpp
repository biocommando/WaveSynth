#include "EditorGui.h"
#include "Resource.h"
#include "WaveSynth.h"
#include "Scripts.h"
#include "strrpl.h"
#include "IniFileReader.h"

extern void WriteLog(char*, double);

const int knobSize = 48;

AEffGUIEditor* createEditor(AudioEffectX* effect)
{
	return new EditorGui(effect);
}

//------------------------------------------------------------------------------------
EditorGui::EditorGui(void* ptr)
	: AEffGUIEditor(ptr)
{
	checkBanks = true;
	presetNum = 0;
	presetGroupNum = 0;
	lastTweakedTag = -1;
	for (int i = 0; i < NUM_OF_MENUS; i++)
	{
		menuCache[i] = new MacroCache();
	}
}

EditorGui::~EditorGui()
{
	for (int i = 0; i < NUM_OF_MENUS; i++)
	{
		delete menuCache[i];
	}
}

MacroMenu::MacroMenu(const CRect &size, CControlListener *listener, long tag)
	: COptionMenu(size, listener, tag)
{
	macroCount = 0;
	macroCommands = (char *)malloc(sizeof(char));
}

MacroMenu::~MacroMenu()
{
	free(macroCommands);
}

void MacroMenu::addEntry(const char *entry, bool hasMacro)
{
	if (!hasMacro)
	{
		COptionMenu::addEntry(entry);
		return;
	}
	macroCount++;
	macroCommands = (char*)realloc(macroCommands, MACRO_COMMAND_LENGTH * macroCount);
	char *macroText = (char*)malloc(strlen(entry) + 1);
	strcpy(macroText, entry);
	int macroCommandBegin = 0;
	for (int i = 0; i < 100 && macroText[i] != 0; i++)
	{
		if (macroText[i] == ';')
		{
			macroText[i] = 0;
			macroCommandBegin = i + 1;
		}
	}
	int macroCmdIdx = MACRO_COMMAND_LENGTH * (macroCount - 1);
	for (int i = 0; i < MACRO_COMMAND_LENGTH && macroText[macroCommandBegin + i] != 0; i++)
	{
		macroCommands[macroCmdIdx] = macroText[macroCommandBegin + i];
		macroCmdIdx++;
	}
	macroCommands[macroCmdIdx] = 0;
	COptionMenu::addEntry(macroText);
	free(macroText);
}

void EditorGui::SetValueLabelText(int index)
{
	char display[10], label[10], text[32] = "";
	effect->getParameterDisplay(index, display);
	effect->getParameterLabel(index, label);
	strcpy(text, display);
	strcat(text, " ");
	strcat(text, label);
	char s[64];
	strcpy(s, valueLabels[index]->getText());
	valueLabels[index]->setText(text);
}

void MacroMenu::saveToCache(MacroCache *cache)
{
	long entries = getNbEntries();
	char *titleCache = (char *)malloc(1);
	*titleCache = 0;
	long titleCacheSize = 1;
	for (long i = 0; i < entries; i++) 
	{
		char title[256];
		strcpy(title, getEntry(i)->getTitle());
		long titleLen = strlen(title);
		titleCacheSize += titleLen + 1;
		titleCache = (char *)realloc(titleCache, titleCacheSize);
		if (*titleCache)
		{
			sprintf(titleCache, "%s%s;", titleCache, title);
		}
		else 
		{
			sprintf(titleCache, "%s;", title);
		}
	}
	cache->setCache(macroCommands, macroCount * MACRO_COMMAND_LENGTH, titleCache, titleCacheSize);
	free(titleCache);
}

void MacroMenu::loadFromCache(MacroCache *cache)
{
	char *titles = (char*)malloc(1);
	long commandSize = cache->getCache(&macroCommands, &titles);
	macroCount = commandSize / MACRO_COMMAND_LENGTH;
	char *p = titles;
	char *start = NULL;
	while (*p) 
	{
		if (start == NULL)
		{
			start = p;
		}
		if (*p == ';')
		{
			*p = 0;
			if (*start == 0)
			{
				addSeparator();
			} 
			else
			{
				addEntry(start, false);
			}
			start = NULL;
		}
		p++;
	}
	free(titles);
}

void EditorGui::fillMacroMenu(MacroMenu *menu, MacroCache *cache, int tag)
{
	if (!cache->isCached())
	{
		IniFileReader *reader = new IniFileReader();
		reader->openFile(((WaveSynth*)effect)->getMacroDefinitionFile());
		char menuTag[32];
		sprintf(menuTag, "menu_%d", tag);
		char macroText[MACRO_COMMAND_LENGTH * 2] = "1";
		reader->readCharValue(menuTag, "title", macroText);
		menu->addEntry(macroText, false);
		for (int keyIdx = 0; macroText[0] != 0; keyIdx++) {
			macroText[0] = 0;
			reader->readCharValueWithIndex(menuTag, "", keyIdx, macroText);
			if (macroText[0] == 0) break;
			if (macroText[0] == '-')
			{
				menu->addSeparator();
			}
			else if (macroText[0] == '$')
			{
				char macroTitle[100];
				char macroScript[MACRO_COMMAND_LENGTH] = "";
				char segment[100];
				strcpy(segment, macroText);
				reader->readCharValue(segment, "title", macroTitle);
				reader->readCharValue(segment, "script", macroScript);
				if (macroScript[0] == 0)
				{
					sprintf(macroText, "%s;last", macroTitle);
				}
				else
				{
					sprintf(macroText, "%s;script %s", macroTitle, macroScript);
				}
				int paramIdx = 0;
				while(1)
				{
					char variable[32] = "";
					char value[32] = "";
					reader->readCharValueWithIndex(segment, "param", paramIdx, variable);
					if (!variable[0])
					{
						break;
					}
					reader->readCharValue(segment, variable, value);
					if (!value[0])
					{
						break;
					}
					else
					{
						if (paramIdx == 0)
							strcat(macroText, " ");
						char paramStr[100];
						sprintf(paramStr, "%s<%s>", variable, value);
						strcat(macroText, paramStr);
						paramIdx++;
					}
				}
				menu->addEntry(macroText);
			}
			else
			{
				menu->addEntry(macroText);
			}
		}
		menu->saveToCache(cache);
		reader->closeFile();
		delete reader;
	}
	else
	{
		menu->loadFromCache(cache);
	}
}

//------------------------------------------------------------------------------------
bool EditorGui::open(void* ptr)
{
	CRect frameSize(0, 0, knobSize * SET_PARAMS_COUNT, knobSize * ((NUM_PARAM_SETS + 1) * 2 + 1) + knobSize / 2);
	CColor cBg = kWhiteCColor, cFg = { 4, 87, 128, 255 };
	ERect *wSize;
	getRect(&wSize);
	wSize->top = wSize->left = 0;
	wSize->bottom = (VstInt16)frameSize.bottom;
	wSize->right = (VstInt16)frameSize.right;
	CFrame* newFrame = new CFrame(frameSize, ptr, this);
	newFrame->setBackgroundColor(cBg);

	//-- load some bitmaps we need
	CBitmap *background, *backgrounds[10];
	backgrounds[0] = new CBitmap(IDB_BG_WAVEFORM);
	backgrounds[1] = new CBitmap(IDB_BG_DISTORTION);
	backgrounds[2] = new CBitmap(IDB_BG_ENVELOPE);
	backgrounds[3] = new CBitmap(IDB_BG_LFO);
	backgrounds[4] = new CBitmap(IDB_BG_FILTER);
	backgrounds[5] = new CBitmap(IDB_BG_LEVEL);
	backgrounds[6] = new CBitmap(IDB_BG_MODULATION);
	backgrounds[7] = new CBitmap(IDB_BG_TUNING);
	backgrounds[8] = new CBitmap(IDB_BG_PAN);
	backgrounds[9] = new CBitmap(IDB_BG_BLANK);
	background = backgrounds[0];
	CBitmap* handle = new CBitmap(IDB_HANDLE);
	int controlIndex = 0, firstGlobalParam = P_SET(NUM_PARAM_SETS - 1, SET_PARAMS_COUNT);
	for (int oscid = 0; oscid < NUM_PARAM_SETS + 1; oscid++)
	{
		CRect r(0, knobSize, knobSize, knobSize * 2);
		r.offset(0, knobSize * oscid * 2);
		CRect r_label(0, knobSize * 2, knobSize, knobSize * 2 + knobSize / 2);
		r_label.offset(0, knobSize * oscid * 2);
		CRect r_valueLabel(0, knobSize * 2 + knobSize / 2, knobSize, knobSize * 3);
		r_valueLabel.offset(0, knobSize * oscid * 2);
		for (int i = 0; i < SET_PARAMS_COUNT; i++)
		{
			controlIndex = P_SET(oscid, i);
			if (controlIndex >= NUM_PARAMS) break;
			if (oscid >= NUM_PARAM_SETS)
				background = backgrounds[9];
			else if (i == P_BANK || i == P_PATCH || i == P_LOOPING || i == P_LOOP_POSITION)
				background = backgrounds[0];
			else if (i == P_DISTORTION)
				background = backgrounds[1];
			else if (i == P_ATTACK || i == P_DECAY || i == P_SUSTAIN || i == P_TAIL)
				background = backgrounds[2];
			else if (i == P_LFO_FREQ || i == P_LFO_TYPE)
				background = backgrounds[3];
			else if (i == P_CUTOFF || i == P_RESONANCE)
				background = backgrounds[4];
			else if (i == P_LEVEL)
				background = backgrounds[5];
			else if (i == P_LFO_TO_CUE || i == P_LFO_TO_FILTER || i == P_LFO_TO_LEVEL || i == P_LFO_TO_PITCH || i == P_ENV_TO_CUTOFF)
				background = backgrounds[6];
			else if (i == P_DETUNE_OCT || i == P_DETUNE_CENT)
				background = backgrounds[7];
			else if (i == P_PAN)
				background = backgrounds[8];
			CKnob* knob = new CKnob(r, this, controlIndex, background, handle, CPoint(0, 0));
			newFrame->addView(knob);
			controls[controlIndex] = knob;
			CTextLabel *label;// = new CTextLabel(r_label, longParamNames[i]);
			if (controlIndex < firstGlobalParam)
			{
				label = new CTextLabel(r_label, longParamNames[i]);
				label->setBackColor(cBg);
				label->setFrameColor(cBg);
				label->setFontColor(cFg);
			}
			else
			{
				char text[16] = "";
				effect->getParameterName(controlIndex, text);
				label = new CTextLabel(r_label, text);
				label->setBackColor(cBg);
				label->setFrameColor(cBg);
				label->setFontColor(cFg);
			}
			newFrame->addView(label);
			CTextLabel *label2 = new CTextLabel(r_valueLabel, "");
			label2->setBackColor(cBg);
			label2->setFrameColor(cBg);
			label2->setFontColor(cFg);
			newFrame->addView(label2);
			valueLabels[controlIndex] = label2;
			SetValueLabelText(controlIndex);
			r.offset(knobSize, 0);
			r_label.offset(knobSize, 0);
			r_valueLabel.offset(knobSize, 0);
		}
	}
	for (int i = 0; i < NUM_PARAM_SETS; i++)
	{
		CRect r(wSize->right / 3 * i, knobSize / 2, wSize->right / 3 * (i + 1), knobSize / 4 * 3);
		char labelText[100], labelTextTemp[100];
		((WaveSynth*)effect)->getBankAndPatchName(labelTextTemp, effect->getParameter(P_SET(i, P_BANK)), effect->getParameter(P_SET(i, P_PATCH)));
		sprintf(labelText, "Osc %d: %s", i + 1, labelTextTemp);
		banksAndPatches[i] = new CTextLabel(r, labelText);
		banksAndPatches[i]->setBackColor(cBg);
		banksAndPatches[i]->setFrameColor(cBg);
		banksAndPatches[i]->setFontColor(cFg);
		newFrame->addView(banksAndPatches[i]);
	}

	CRect r(0, wSize->bottom - valueLabels[0]->getHeight(), wSize->right, wSize->bottom);
	CTextLabel *label = new CTextLabel(r, "Little, ugly gnome - Wavetable synth - Joonas Salonpaa - 2016");
	newFrame->addView(label);
	//macroCommands = (char*)malloc(1);
	for (int i = 0; i < NUM_OF_MENUS; i++)
	{
		r = CRect(knobSize * 22 / 3 * i, 0, knobSize * 22 / 3 * (i + 1) - 1, knobSize / 2 - 4);
		menu[i] = new MacroMenu(r, this, MENU_1_ID + i);
		menu[i]->setBackColor(cBg);
		menu[i]->setFrameColor(cFg);
		menu[i]->setFontColor(cFg);
		fillMacroMenu(menu[i], menuCache[i], i + 1);
		newFrame->addView(menu[i]);
	}
	r = CRect(0, wSize->bottom - valueLabels[0]->getHeight(), knobSize * 22 / 3, wSize->bottom);
	presetMenu = new COptionMenu(r, this, PRESET_MENU_ID);
	presetMenu->setBackColor(cFg);
	presetMenu->setFrameColor(cBg);
	presetMenu->setFontColor(cBg);
	presetMenu->addEntry(new CMenuItem("Select preset...", 1 << 1));
	presetMenu->addEntry(new CMenuItem("Save as new"));
	presetLoader.readPresets();
	auto presets = presetLoader.getPresets();
	for (int i = 0; i < presets.size(); i++)
	{
		presetMenu->addEntry(new CMenuItem(presets[i].c_str()));
	}
	newFrame->addView(presetMenu);

	/*r = CRect(0, 0, knobSize * 5 - 1, knobSize / 2 - 4);
	menu[0] = new COptionMenu(r, this, MENU_1_ID);
	menu[0]->setBackColor(cBg);
	menu[0]->setFrameColor(cFg);
	menu[0]->setFontColor(cFg);
	newFrame->addView(menu[0]);
	r = CRect(knobSize*5, 0, knobSize * 15 - 1, knobSize / 2 - 4);
	menu[1] = new COptionMenu(r, this, MENU_2_ID);
	menu[1]->setBackColor(cBg);
	menu[1]->setFrameColor(cFg);
	menu[1]->setFontColor(cFg);
	loadPresetNames();
	newFrame->addView(menu[1]);
	r = CRect(knobSize * 15, 0, knobSize * 22, knobSize / 2 - 4);
	menu[2] = new COptionMenu(r, this, MENU_3_ID);
	menu[2]->setBackColor(cBg);
	menu[2]->setFrameColor(cFg);
	menu[2]->setFontColor(cFg);*/
	/*
	macroCommands = (char*)malloc(1);
	fillMacroMenu(menu[2], 1, 0);
	newFrame->addView(menu[2]);*/

	//-- forget the bitmaps
	for (int i = 0; i < 10; i++)
		backgrounds[i]->forget();
	handle->forget();

	//-- set the member frame to the newly created frame
	//-- we do it this way because it is possible that the setParameter method is called
	//-- in between of constructing the frame and it's controls
	frame = newFrame;

	//-- sync parameters
	for (int i = 0; i < NUM_PARAMS; i++)
	{
		setParameter(i, effect->getParameter(i));
	}

	frame = newFrame;

	return true;
}

//------------------------------------------------------------------------------------
void EditorGui::close()
{
	//-- on close we need to delete the frame object.
	//-- once again we make sure that the member frame variable is set to zero before we delete it
	//-- so that calls to setParameter won't crash.
	CFrame* oldFrame = frame;
	frame = 0;
	delete oldFrame;
	//free(macroCommands);
}

void MacroMenu::doMacroEdits(AEffGUIEditor *editor, long lastTweakedTag)
{
	if (getCurrentIndex() <= 0) return;
	char cmd[20] = "", scriptName[200] = "", otherScriptVariables[200] = "";
	char *command = &macroCommands[MACRO_COMMAND_LENGTH * (getCurrentIndex() - 1)];
	int numCmd = sscanf(command, "%s %s %s", cmd, scriptName, otherScriptVariables);
	AudioEffect *effect = editor->getEffect();
	extern void WriteLog(char*, double = 0);

	if (!strcmp(cmd, "last") && lastTweakedTag >= 0)
	{
		int paramSet, paramId = -1;
		float paramValue = ((WaveSynth*)effect)->getParameter(lastTweakedTag);
		for (paramSet = 0; paramSet < NUM_PARAM_SETS; paramSet++)
		{
			paramId = lastTweakedTag - (SET_PARAMS_START + SET_PARAMS_COUNT * paramSet);
			if (paramId >= 0 && paramId < SET_PARAMS_COUNT)
			{
				for (int paramSetToChange = 0; paramSetToChange < NUM_PARAM_SETS; paramSetToChange++)
				{
					if (paramSetToChange != paramSet)
						((WaveSynth*)effect)->setParameter(P_SET(paramSetToChange, paramId), paramValue);
				}
				break;
			}
		}
	}
	else if (!strcmp(cmd, "script"))
	{
		extern void getWorkDir(char*);
		char filename[1024];
		getWorkDir(filename);
		sprintf(filename, "%sscripts\\%s.syn", filename, scriptName);
		char *mem = NULL, *ptr;
		char *initial = (char*)malloc(1);
		initial[0] = 0;
		int initial_sz = 1;
		for (int i = 0; i < NUM_PARAMS + 3; i++)
		{
			char s[100], name[100];
			float value;
			if (i < NUM_PARAMS)
			{
				// this will set the default float value to all integer parameters 
				ParamDTO dto;
				((WaveSynth*)effect)->getParameter(&dto, i);
				((WaveSynth*)effect)->setParameter(&dto);

				effect->getParameterName(i, name);
				value = effect->getParameter(i);
			}
			else if (i == NUM_PARAMS)
			{
				strcpy(name, "tempo");
				VstTimeInfo *timeInfo = ((WaveSynth*)effect)->getTimeInfo(kVstTempoValid);
				value = (float)timeInfo->tempo;
			}
			else if (i == NUM_PARAMS + 1)
			{
				strcpy(name, "randomSeed");
				value = -1;
			}
			
			if (i == NUM_PARAMS + 2)
				sprintf(s, "%s", otherScriptVariables);
			else
			{
				sprintf(s, "%s<%f>", name, value);
			}
			initial_sz += strlen(s);
			initial = (char *)realloc(initial, initial_sz);
			strcat(initial, s);
		}
		//WriteLog(initial, 1);
		initial[initial_sz - 1] = 0;
		unsigned int byteSize = ExecuteScript(filename, &mem, initial);
		free(initial);

		ptr = mem;
		while (ptr - mem < (long)byteSize)
		{
			ParamDTO dto;
			ptr = dto.fromMem(ptr);
			//WriteLog(dto.id, dto.floatValue);
			int index = ((WaveSynth*)effect)->setParameter(&dto);
			if (index > -1)
			{
				editor->setParameter(index, effect->getParameter(index));
				((EditorGui*)editor)->valueChanged(index);
			}
		}
		free(mem);
	}
}

void EditorGui::valueChanged(int index)
{
	valueChanged(controls[index]);
}

//------------------------------------------------------------------------------------
void EditorGui::valueChanged(CControl* pControl)
{
	long tag = pControl->getTag();
	if (tag >= MENU_1_ID && tag <= MENU_3_ID)
	{
		int menuIdx = tag - MENU_1_ID;
		menu[menuIdx]->doMacroEdits(this, lastTweakedTag);
		menu[menuIdx]->setCurrent(0);
		return;
	}
	if (tag == PRESET_MENU_ID)
	{
		int presetIndex = presetMenu->getCurrentIndex() - 2;
		if (presetIndex == -1)
		{
			std::vector<ParamDTO> dtos;
			for (int i = 0; i < NUM_PARAMS; i++)
			{
				ParamDTO dto;
				((WaveSynth*)effect)->getParameter(&dto, i);
				dtos.push_back(dto);
			}
			auto newName = presetLoader.savePreset(dtos);
			presetMenu->addEntry(new CMenuItem(newName.c_str()));
		}
		else
		{
			auto dtos = presetLoader.loadPreset(presetIndex);
			for (int i = 0; i < dtos.size(); i++)
			{
				auto paramIdx = ((WaveSynth*)effect)->setParameter(&dtos[i]);
				setParameter(paramIdx, effect->getParameter(paramIdx));
				valueChanged(paramIdx);
			}
		}
		presetMenu->setCurrent(0);
		return;
	}
	//-- valueChanged is called whenever the user changes one of the controls in the User Interface (UI)
	float value = pControl->getValue();
	if (value > 1) value = 1;
	else if (value < 0) value = 0;
	effect->setParameterAutomated(tag, value);
	SetValueLabelText(tag);
	lastTweakedTag = tag;
	for (int i = 0; i < NUM_PARAM_SETS; i++)
	{
		if (tag == P_SET(i, P_BANK) || tag == P_SET(i, P_PATCH))
		{
			char labelText[100], labelTextTemp[100];
			((WaveSynth*)effect)->getBankAndPatchName(labelTextTemp, effect->getParameter(P_SET(i, P_BANK)), effect->getParameter(P_SET(i, P_PATCH)));
			sprintf(labelText, "Osc %d: %s", i + 1, labelTextTemp);
			banksAndPatches[i]->setText(labelText);
			if (checkBanks)
			{
				effect->setParameterAutomated(P_SET(i, P_BANK), controls[P_SET(i, P_BANK)]->getValue());
				effect->setParameterAutomated(P_SET(i, P_PATCH), controls[P_SET(i, P_PATCH)]->getValue());
			}
			SetValueLabelText(P_SET(i, P_BANK));
			SetValueLabelText(P_SET(i, P_PATCH));
		}
	}
}

//------------------------------------------------------------------------------------
void EditorGui::setParameter(VstInt32 index, float value)
{
	//-- setParameter is called when the host automates one of the effects parameter.
	//-- The UI should reflect this state so we set the value of the control to the new value.
	//-- VSTGUI will automaticly redraw changed controls in the next idle (as this call happens to be in the process thread).
	extern void WriteLog(char*, double = 0);
	((WaveSynth*)effect)->startTransaction();
	if (frame && index < NUM_PARAMS)
	{
		controls[index]->setValue(value);
		SetValueLabelText(controls[index]->getTag());
		for (int i = 0; i < NUM_PARAM_SETS; i++)
		{
			if (controls[index]->getTag() == P_SET(i, P_BANK) || controls[index]->getTag() == P_SET(i, P_PATCH))
			{
				char labelText[100], labelTextTemp[100];
				((WaveSynth*)effect)->getBankAndPatchName(labelTextTemp, effect->getParameter(P_SET(i, P_BANK)), effect->getParameter(P_SET(i, P_PATCH)));
				sprintf(labelText, "Osc %d: %s", i + 1, labelTextTemp);
				banksAndPatches[i]->setText(labelText);
				/*controls[P_SET(i, P_BANK)]->setValue(value);
				controls[P_SET(i, P_PATCH)]->setValue(value);*/
			}
		}
	}
	((WaveSynth*)effect)->endTransaction();
}
