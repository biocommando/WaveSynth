#pragma once

#include "aeffguieditor.h"
#include <string>
#include "PresetLoader.h"

class EditorGui;

// A flat button that draws its title and notifies the listener when clicked
class PresetButton : public CControl
{
	std::string title;
	bool pressed;
	CColor fg, bg;
public:
	PresetButton(const CRect& size, CControlListener* listener, long tag, const char* title, CColor fg, CColor bg);
	void setTitle(const std::string& newTitle);
	void draw(CDrawContext* pContext) override;
	CMouseEventResult onMouseDown(CPoint& where, const long& buttons) override;
	CMouseEventResult onMouseMoved(CPoint& where, const long& buttons) override;
	CMouseEventResult onMouseUp(CPoint& where, const long& buttons) override;
	CLASS_METHODS(PresetButton, CControl)
};

// Scrollable list of preset names. Clicking a row selects it and notifies the listener.
class PresetList : public CControl
{
	PresetLoader* loader;
	int selected;
	int firstVisibleRow;
	bool draggingScrollbar;
	CCoord dragOffset;
	CColor fg, bg;
	int visibleRows() const;
	int maxFirstVisibleRow() const;
	bool hasScrollbar() const;
	CRect scrollbarRect() const;
	CRect thumbRect() const;
	void scrollTo(int row);
	void scrollThumbTo(CCoord thumbTop);
public:
	PresetList(const CRect& size, CControlListener* listener, long tag, PresetLoader* loader, CColor fg, CColor bg);
	int getSelected() const { return selected; }
	void setSelected(int index);
	void ensureVisible(int index);
	void draw(CDrawContext* pContext) override;
	CMouseEventResult onMouseDown(CPoint& where, const long& buttons) override;
	CMouseEventResult onMouseMoved(CPoint& where, const long& buttons) override;
	CMouseEventResult onMouseUp(CPoint& where, const long& buttons) override;
	using CControl::onWheel;
	bool onWheel(const CPoint& where, const CMouseWheelAxis& axis, const float& distance, const long& buttons) override;
	CLASS_METHODS(PresetList, CControl)
};

// Modal preset manager drawn on top of the whole editor
class PresetBrowser : public CViewContainer, public CControlListener
{
	EditorGui* editor;
	PresetLoader* loader;
	PresetList* list;
	CTextEdit* nameEdit;
	PresetButton* deleteButton;
	CTextLabel* status;
	bool confirmingDelete;
	void setStatus(const char* text);
	void resetDeleteConfirmation();
	void selectAndLoad(int index);
	std::string getEnteredName();
public:
	PresetBrowser(const CRect& size, CFrame* frame, EditorGui* editor, PresetLoader* loader, CColor fg, CColor bg);
	// call before showing the browser
	void refresh(int currentPreset);
	void endNameEdit();
	void valueChanged(CControl* pControl) override;
	long onKeyDown(VstKeyCode& keyCode) override;
};
