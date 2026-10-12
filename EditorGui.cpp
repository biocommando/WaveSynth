#include "EditorGui.h"
#include "CBitmapLoader.h"
#include "WaveSynth.h"
#include "Plugins.h"
#include "strrpl.h"
#include "IniFileReader.h"
#include "build_time.h"

extern void WriteLog(const char*, double);

const int knobSize = 48;

//------------------------------------------------------------------------------------
EditorGui::EditorGui(WaveSynth* effect)
	: AEffGUIEditorFst(effect), effect(effect)
{
	checkBanks = true;
	presetNum = 0;
	presetGroupNum = 0;
	lastTweakedTag = -1;
	presetButton = nullptr;
	presetBrowser = nullptr;
	currentPreset = -1;
	presetModified = false;
}

EditorGui::~EditorGui()
{
}

MacroMenu::MacroMenu(const CRect& size, CControlListener* listener, long tag)
	: COptionMenu(size, listener, tag)
{
}

MacroMenu::~MacroMenu()
{
}
void MacroMenu::addEntry(const std::string& title)
{
	COptionMenu::addEntry(title.c_str());
}
void MacroMenu::addEntry(MacroCommand entry)
{
	/*if (!hasMacro)
	{
		COptionMenu::addEntry(entry.title);
		return;
	}*/
	entry.idx = getNbEntries();
	macroCommands.push_back(entry);
	COptionMenu::addEntry(entry.title.c_str());
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

void EditorGui::fillMacroMenu(MacroMenu* menu, int tag)
{
	IniFileReader reader;
	reader.openFile(((WaveSynth*)effect)->getMacroDefinitionFile());
	char menuTag[32];
	sprintf(menuTag, "menu_%d", tag);
	char macroText[MACRO_COMMAND_LENGTH * 2] = "1";
	reader.readCharValue(menuTag, "title", macroText);
	menu->addEntry(macroText);
	for (int keyIdx = 0; macroText[0] != 0; keyIdx++) {
		macroText[0] = 0;
		reader.readCharValueWithIndex(menuTag, "", keyIdx, macroText);
		if (macroText[0] == 0) break;
		if (macroText[0] == '-')
		{
			menu->addSeparator();
		}
		else if (macroText[0] == '$')
		{
			MacroCommand macro;
			char macroTitle[100];
			char macroScript[MACRO_COMMAND_LENGTH] = "";
			char segment[100];
			strcpy(segment, macroText);
			reader.readCharValue(segment, "title", macroTitle);
			macro.title = macroTitle;
			reader.readCharValue(segment, "script", macroScript);
			if (macroScript[0] == 0)
			{
				macro.script = "internal:lastTweakToAll";
			}
			else
			{
				macro.script = macroScript;
			}
			int paramIdx = 0;
			while (1)
			{
				char variable[32] = "";
				char value[32] = "";
				reader.readCharValueWithIndex(segment, "param", paramIdx, variable);
				if (!variable[0])
				{
					break;
				}
				reader.readCharValue(segment, variable, value);
				if (!value[0])
				{
					break;
				}
				else
				{
					MacroCommandParam p = { variable, std::stod(value) };
					macro.params.push_back(p);
					paramIdx++;
				}
			}
			menu->addEntry(macro);
		}
		else
		{
			menu->addEntry(macroText);
		}
	}
	reader.closeFile();
}

//------------------------------------------------------------------------------------
void EditorGui::getEditorSize(int *width, int *height)
{
	*width = knobSize * SET_PARAMS_COUNT;
	*height = knobSize * ((NUM_PARAM_SETS + 1) * 2 + 1) + knobSize / 2;
}

bool EditorGui::open(void* ptr)
{
	int width, height;
	getEditorSize(&width, &height);
	CRect frameSize(0, 0, width, height);
	CColor cBg = kWhiteCColor, cFg = { 4, 87, 128, 255 };
	ERect * wSize;
	getRect(&wSize);
	wSize->top = wSize->left = 0;
	wSize->bottom = (short)frameSize.bottom;
	wSize->right = (short)frameSize.right;
	CFrame * newFrame = new CFrame(frameSize, ptr, this);
	newFrame->setBackgroundColor(cBg);

	//-- load some bitmaps we need
	CBitmap * background, *backgrounds[10];
	backgrounds[0] = CBitmapLoader::load("images/background-wave.bmp");
	backgrounds[1] = CBitmapLoader::load("images/background-distort.bmp");
	backgrounds[2] = CBitmapLoader::load("images/background-envelope.bmp");
	backgrounds[3] = CBitmapLoader::load("images/background-lfo.bmp");
	backgrounds[4] = CBitmapLoader::load("images/background-filter.bmp");
	backgrounds[5] = CBitmapLoader::load("images/background-level.bmp");
	backgrounds[6] = CBitmapLoader::load("images/background-modulation.bmp");
	backgrounds[7] = CBitmapLoader::load("images/background-tuning.bmp");
	backgrounds[8] = CBitmapLoader::load("images/background-pan.bmp");
	backgrounds[9] = CBitmapLoader::load("images/background.bmp");
	background = backgrounds[0];
	CBitmap * handle = CBitmapLoader::load("images/handle.bmp");
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
			CKnob * knob = new CKnob(r, this, controlIndex, background, handle, CPoint(0, 0));
			newFrame->addView(knob);
			controls[controlIndex] = knob;
			CTextLabel * label;// = new CTextLabel(r_label, longParamNames[i]);
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
			CTextLabel* label2 = new CTextLabel(r_valueLabel, "");
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

	CRect r(knobSize * 22 / 3, wSize->bottom - valueLabels[0]->getHeight(), wSize->right, wSize->bottom);
	CTextLabel * label = new CTextLabel(r, "Little, ugly gnome - Wavetable synth - Joonas Salonpaa - 2016-2022 - build " BUILD_TIME);
	newFrame->addView(label);
	//macroCommands = (char*)malloc(1);
	for (int i = 0; i < NUM_OF_MENUS; i++)
	{
		r = CRect(knobSize * 22 / 3 * i, 0, knobSize * 22 / 3 * (i + 1) - 1, knobSize / 2 - 4);
		menu[i] = new MacroMenu(r, this, MENU_1_ID + i);
		menu[i]->setBackColor(cBg);
		menu[i]->setFrameColor(cFg);
		menu[i]->setFontColor(cFg);
		fillMacroMenu(menu[i], i + 1);
		newFrame->addView(menu[i]);
	}
	r = CRect(0, wSize->bottom - valueLabels[0]->getHeight(), knobSize * 22 / 3, wSize->bottom);
	presetLoader.readPresets();
	if (currentPreset >= presetLoader.count())
		currentPreset = -1;
	presetButton = new PresetButton(r, this, PRESET_BUTTON_ID, "", cFg, cBg);
	newFrame->addView(presetButton);
	updatePresetButton();
	presetBrowser = new PresetBrowser(frameSize, newFrame, this, &presetLoader, cFg, cBg);

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

	//frame = newFrame;

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
	if (oldFrame)
	{
		// the browser is owned by the editor, not by the frame
		if (oldFrame->getModalView() == presetBrowser)
		{
			presetBrowser->endNameEdit();
			oldFrame->setFocusView(0);
			oldFrame->setModalView(0);
		}
		delete oldFrame;
	}
	if (presetBrowser)
	{
		presetBrowser->forget();
		presetBrowser = nullptr;
	}
	presetButton = nullptr;
}

MacroCommand* MacroMenu::getCurrentCommand()
{
	for (int i = 0; i < macroCommands.size(); i++)
	{
		if (macroCommands[i].idx == getCurrentIndex(true))
		{
			return &macroCommands[i];
		}
	}
	return nullptr;
}

#define ADD_PLUGIN_PARAM()											\
	do																\
	{																\
		PluginVariable var = { type, std::string(name), value };	\
		initial.push_back(var);										\
	} while(0)

void MacroMenu::doMacroEdits(EditorGui * editor, long lastTweakedTag)
{
	auto cmd = getCurrentCommand();
	if (cmd == nullptr)
	{
		return;
	}
	auto* effect = editor->getSynth();

	if (cmd->script == "internal:lastTweakedToAll" && lastTweakedTag >= 0)
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
	else
	{
		extern void getWorkDir(char*);
		char workDir[1024];
		getWorkDir(workDir);
		const std::string filename = std::string(workDir) + "plugins\\" + cmd->script;
		std::vector<PluginVariable> initial;
		char name[100];
		char type = TYPE_TEMPORARY;
		float value;
		for (int i = 0; i < NUM_PARAMS; i++)
		{
			// this will set the default float value to all integer parameters 
			ParamDTO dto;
			((WaveSynth*)effect)->getParameter(&dto, i);
			((WaveSynth*)effect)->setParameter(&dto);

			((WaveSynth*)effect)->getParameterName(i, name);
			value = effect->getParameter(i);
			ADD_PLUGIN_PARAM();
		}

		strcpy(name, "tempo");
		value = (float)effect->getTempo();
		ADD_PLUGIN_PARAM();

		strcpy(name, "randomSeed");
		value = -1;
		ADD_PLUGIN_PARAM();

		strcpy(name, "wavePackIndexReadOnly");
		value = ((WaveSynth*)effect)->getSelectedPack();
		ADD_PLUGIN_PARAM();

		for (int i = 0; i < cmd->params.size(); i++)
		{
			auto param = &cmd->params[i];
			strcpy(name, param->name.c_str());
			value = param->value;
			ADD_PLUGIN_PARAM();
		}

		auto dtos = ExecutePlugin(filename, initial);

		for (int i = 0; i < dtos.size(); i++)
		{
			auto dto = &dtos[i];
			int index = ((WaveSynth*)effect)->setParameter(dto);
			if (index > -1)
			{
				editor->setParameter(index, effect->getParameter(index));
				editor->valueChanged(index);
			}
		}
	}
}

std::vector<ParamDTO> EditorGui::getCurrentParams(int *wavePack)
{
	std::vector<ParamDTO> dtos;
	for (int i = 0; i < NUM_PARAMS; i++)
	{
		ParamDTO dto;
		effect->getParameter(&dto, i);
		dtos.push_back(dto);
	}
	*wavePack = effect->getSelectedPack();
	return dtos;
}

void EditorGui::loadPreset(int presetIndex)
{
	int packIdx = 0;
	auto dtos = presetLoader.loadPreset(presetIndex, &packIdx);
	if (dtos.empty())
		return;
	effect->setSelectedPack(packIdx);
	for (int i = 0; i < dtos.size(); i++)
	{
		auto paramIdx = effect->setParameter(&dtos[i]);
		if (paramIdx < 0 || paramIdx >= NUM_PARAMS)
			continue;
		setParameter(paramIdx, effect->getParameter(paramIdx));
		valueChanged(paramIdx);
	}
	setCurrentPreset(presetIndex);
	// Redraw everything. Relying on the dirty views doesn't work here: VSTGUI paints the bounding box
	// of the invalid region into the back buffer, marking the views in it clean, but copies only the
	// invalid region to the window. The browser views invalidated right away would get painted
	// before the idle has invalidated the changed knobs, and some knobs would stay stale on screen.
	if (frame)
		frame->invalid();
}

void EditorGui::setCurrentPreset(int presetIndex)
{
	currentPreset = presetIndex;
	presetModified = false;
	updatePresetButton();
}

void EditorGui::presetDeleted(int presetIndex)
{
	if (currentPreset == presetIndex)
		setCurrentPreset(-1);
	else if (currentPreset > presetIndex)
		setCurrentPreset(currentPreset - 1);
}

void EditorGui::updatePresetButton()
{
	if (!presetButton)
		return;
	if (currentPreset < 0)
		presetButton->setTitle("Presets...");
	else
		presetButton->setTitle("Preset: " + presetLoader.getName(currentPreset) + (presetModified ? " (modified)" : ""));
}

void EditorGui::openPresetBrowser()
{
	if (!frame || !presetBrowser || frame->getModalView())
		return;
	presetBrowser->refresh(currentPreset);
	frame->setModalView(presetBrowser);
	presetBrowser->invalid();
}

void EditorGui::closePresetBrowser()
{
	if (!frame || frame->getModalView() != presetBrowser)
		return;
	presetBrowser->endNameEdit();
	frame->setFocusView(0);
	frame->setModalView(0);
	frame->invalid();
}

void EditorGui::valueChanged(int index)
{
	valueChanged(controls[index]);
}

//------------------------------------------------------------------------------------
void EditorGui::valueChanged(CControl * pControl)
{
	long tag = pControl->getTag();
	if (tag >= MENU_1_ID && tag <= MENU_3_ID)
	{
		int menuIdx = tag - MENU_1_ID;
		menu[menuIdx]->doMacroEdits(this, lastTweakedTag);
		menu[menuIdx]->setCurrent(0);
		return;
	}
	if (tag == PRESET_BUTTON_ID)
	{
		openPresetBrowser();
		return;
	}
	//-- valueChanged is called whenever the user changes one of the controls in the User Interface (UI)
	float value = pControl->getValue();
	if (value > 1) value = 1;
	else if (value < 0) value = 0;
	effect->setParameterAutomated(tag, value);
	SetValueLabelText(tag);
	lastTweakedTag = tag;
	if (!presetModified && currentPreset >= 0)
	{
		presetModified = true;
		updatePresetButton();
	}
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
			}
		}
	}
	((WaveSynth*)effect)->endTransaction();
}
