#include "PresetBrowser.h"
#include "EditorGui.h"
#include "vstkeycode.h"

enum
{
	TAG_BACKDROP = 1,
	TAG_LIST,
	TAG_NAME,
	TAG_SAVE,
	TAG_SAVE_NEW,
	TAG_DELETE,
	TAG_CLOSE
};

const CCoord rowHeight = 18;
const CCoord scrollbarWidth = 10;
const int wheelRows = 3;

// Truncates the text with "..." so that it fits into maxWidth
static std::string fitText(CDrawContext* pContext, const std::string& text, CCoord maxWidth)
{
	if (pContext->getStringWidth(text.c_str()) <= maxWidth)
		return text;
	std::string s = text;
	while (!s.empty() && pContext->getStringWidth((s + "...").c_str()) > maxWidth)
		s.pop_back();
	return s + "...";
}

//------------------------------------------------------------------------------------
PresetButton::PresetButton(const CRect& size, CControlListener* listener, long tag, const char* title, CColor fg, CColor bg)
	: CControl(size, listener, tag), title(title), pressed(false), fg(fg), bg(bg)
{
}

void PresetButton::setTitle(const std::string& newTitle)
{
	if (title == newTitle)
		return;
	title = newTitle;
	invalid();
}

void PresetButton::draw(CDrawContext* pContext)
{
	pContext->setFillColor(pressed ? bg : fg);
	pContext->setFrameColor(fg);
	pContext->setLineWidth(1);
	pContext->drawRect(size, kDrawFilledAndStroked);
	pContext->setFont(kNormalFontSmall);
	pContext->setFontColor(pressed ? fg : bg);
	CRect textRect(size);
	textRect.inset(4, 0);
	pContext->drawString(fitText(pContext, title, textRect.getWidth()).c_str(), textRect, false, kCenterText);
	setDirty(false);
}

CMouseEventResult PresetButton::onMouseDown(CPoint& where, const long& buttons)
{
	if (!(buttons & kLButton))
		return kMouseEventNotHandled;
	pressed = true;
	invalid();
	return kMouseEventHandled;
}

CMouseEventResult PresetButton::onMouseMoved(CPoint& where, const long& buttons)
{
	if (buttons & kLButton)
	{
		bool inside = where.isInside(size);
		if (inside != pressed)
		{
			pressed = inside;
			invalid();
		}
	}
	return kMouseEventHandled;
}

CMouseEventResult PresetButton::onMouseUp(CPoint& where, const long& buttons)
{
	bool clicked = pressed && where.isInside(size);
	pressed = false;
	invalid();
	if (clicked && listener)
		listener->valueChanged(this);
	return kMouseEventHandled;
}

//------------------------------------------------------------------------------------
PresetList::PresetList(const CRect& size, CControlListener* listener, long tag, PresetLoader* loader, CColor fg, CColor bg)
	: CControl(size, listener, tag), loader(loader), selected(-1), firstVisibleRow(0),
	draggingScrollbar(false), dragOffset(0), fg(fg), bg(bg)
{
}

int PresetList::visibleRows() const
{
	return (int)((size.getHeight() - 2) / rowHeight);
}

int PresetList::maxFirstVisibleRow() const
{
	int max = loader->count() - visibleRows();
	return max > 0 ? max : 0;
}

bool PresetList::hasScrollbar() const
{
	return loader->count() > visibleRows();
}

CRect PresetList::scrollbarRect() const
{
	return CRect(size.right - scrollbarWidth - 1, size.top + 1, size.right - 1, size.bottom - 1);
}

CRect PresetList::thumbRect() const
{
	CRect track = scrollbarRect();
	int count = loader->count();
	if (count <= 0)
		return track;
	CCoord thumbHeight = track.getHeight() * visibleRows() / count;
	if (thumbHeight < 20)
		thumbHeight = 20;
	int maxFirst = maxFirstVisibleRow();
	CCoord top = track.top;
	if (maxFirst > 0)
		top += (track.getHeight() - thumbHeight) * firstVisibleRow / maxFirst;
	return CRect(track.left, top, track.right, top + thumbHeight);
}

void PresetList::scrollTo(int row)
{
	if (row > maxFirstVisibleRow())
		row = maxFirstVisibleRow();
	if (row < 0)
		row = 0;
	if (row != firstVisibleRow)
	{
		firstVisibleRow = row;
		invalid();
	}
}

void PresetList::scrollThumbTo(CCoord thumbTop)
{
	CRect track = scrollbarRect();
	CRect thumb = thumbRect();
	CCoord range = track.getHeight() - thumb.getHeight();
	if (range <= 0)
		return;
	double pos = (double)(thumbTop - track.top) / range;
	scrollTo((int)(pos * maxFirstVisibleRow() + 0.5));
}

void PresetList::setSelected(int index)
{
	if (index >= loader->count())
		index = loader->count() - 1;
	selected = index;
	scrollTo(firstVisibleRow); // the list may have shrunk
	invalid();
}

void PresetList::ensureVisible(int index)
{
	if (index < 0)
		return;
	if (index < firstVisibleRow)
		scrollTo(index);
	else if (index >= firstVisibleRow + visibleRows())
		scrollTo(index - visibleRows() + 1);
}

void PresetList::draw(CDrawContext* pContext)
{
	CColor stripe = { 235, 242, 246, 255 };
	pContext->setLineWidth(1);
	pContext->setFillColor(bg);
	pContext->setFrameColor(fg);
	pContext->drawRect(size, kDrawFilledAndStroked);
	pContext->setFont(kNormalFontSmall);

	CCoord rowRight = size.right - 1 - (hasScrollbar() ? scrollbarWidth : 0);
	int count = loader->count();
	for (int i = 0; i < visibleRows() && firstVisibleRow + i < count; i++)
	{
		int index = firstVisibleRow + i;
		CRect row(size.left + 1, size.top + 1 + i * rowHeight, rowRight, size.top + 1 + (i + 1) * rowHeight);
		if (index == selected || index % 2 == 1)
		{
			pContext->setFillColor(index == selected ? fg : stripe);
			pContext->drawRect(row, kDrawFilled);
		}
		pContext->setFontColor(index == selected ? bg : fg);
		CRect textRect(row);
		textRect.inset(6, 0);
		auto name = fitText(pContext, std::to_string(index + 1) + ". " + loader->getName(index), textRect.getWidth());
		pContext->drawString(name.c_str(), textRect, false, kLeftText);
	}
	if (count == 0)
	{
		pContext->setFontColor(fg);
		pContext->drawString("No presets yet. Use \"Save new\" to add one.", size, false, kCenterText);
	}

	if (hasScrollbar())
	{
		pContext->setFillColor(stripe);
		pContext->drawRect(scrollbarRect(), kDrawFilled);
		pContext->setFillColor(fg);
		pContext->drawRect(thumbRect(), kDrawFilled);
	}
	setDirty(false);
}

CMouseEventResult PresetList::onMouseDown(CPoint& where, const long& buttons)
{
	if (!(buttons & kLButton))
		return kMouseEventNotHandled;
	if (hasScrollbar() && where.isInside(scrollbarRect()))
	{
		CRect thumb = thumbRect();
		if (where.isInside(thumb))
		{
			draggingScrollbar = true;
			dragOffset = where.v - thumb.top;
		}
		else if (where.v < thumb.top)
			scrollTo(firstVisibleRow - visibleRows());
		else
			scrollTo(firstVisibleRow + visibleRows());
		return kMouseEventHandled;
	}
	int index = firstVisibleRow + (int)((where.v - size.top - 1) / rowHeight);
	if (index >= 0 && index < loader->count())
	{
		selected = index;
		invalid();
		if (listener)
			listener->valueChanged(this);
	}
	return kMouseEventHandled;
}

CMouseEventResult PresetList::onMouseMoved(CPoint& where, const long& buttons)
{
	if (draggingScrollbar && (buttons & kLButton))
		scrollThumbTo(where.v - dragOffset);
	return kMouseEventHandled;
}

CMouseEventResult PresetList::onMouseUp(CPoint& where, const long& buttons)
{
	draggingScrollbar = false;
	return kMouseEventHandled;
}

bool PresetList::onWheel(const CPoint& where, const CMouseWheelAxis& axis, const float& distance, const long& buttons)
{
	if (axis != kMouseWheelAxisY)
		return false;
	int rows = (int)(distance * wheelRows);
	if (rows == 0)
		rows = distance > 0 ? 1 : -1;
	scrollTo(firstVisibleRow - rows);
	return true;
}

//------------------------------------------------------------------------------------
// Dims the editor behind the browser and closes the browser when clicked
class PresetBackdrop : public CControl
{
public:
	PresetBackdrop(const CRect& size, CControlListener* listener, long tag)
		: CControl(size, listener, tag)
	{
	}
	void draw(CDrawContext* pContext) override
	{
		CColor dim = { 0, 0, 0, 120 };
		pContext->setFillColor(dim);
		pContext->drawRect(size, kDrawFilled);
		setDirty(false);
	}
	CMouseEventResult onMouseDown(CPoint& where, const long& buttons) override
	{
		return kMouseEventHandled;
	}
	CMouseEventResult onMouseUp(CPoint& where, const long& buttons) override
	{
		if (listener && where.isInside(size))
			listener->valueChanged(this);
		return kMouseEventHandled;
	}
	CLASS_METHODS(PresetBackdrop, CControl)
};

// Background of the browser panel. Swallows clicks so that they don't close the browser.
class PresetPanel : public CView
{
	CColor fg, bg;
public:
	PresetPanel(const CRect& size, CColor fg, CColor bg)
		: CView(size), fg(fg), bg(bg)
	{
	}
	void draw(CDrawContext* pContext) override
	{
		pContext->setFillColor(bg);
		pContext->setFrameColor(fg);
		pContext->setLineWidth(2);
		pContext->drawRect(size, kDrawFilledAndStroked);
		setDirty(false);
	}
	CMouseEventResult onMouseDown(CPoint& where, const long& buttons) override
	{
		return kMouseEventHandled;
	}
};

//------------------------------------------------------------------------------------
PresetBrowser::PresetBrowser(const CRect& size, CFrame* frame, EditorGui* editor, PresetLoader* loader, CColor fg, CColor bg)
	: CViewContainer(size, frame), editor(editor), loader(loader), confirmingDelete(false)
{
	setTransparency(true);
	addView(new PresetBackdrop(size, this, TAG_BACKDROP));

	const CCoord panelWidth = 460, margin = 12, gap = 8;
	CCoord panelHeight = size.getHeight() - 40;
	CRect panel(0, 0, panelWidth, panelHeight);
	panel.offset((size.getWidth() - panelWidth) / 2, (size.getHeight() - panelHeight) / 2);
	addView(new PresetPanel(panel, fg, bg));

	CCoord left = panel.left + margin, right = panel.right - margin, y = panel.top + margin;

	CTextLabel* title = new CTextLabel(CRect(left, y, right, y + 20), "Presets");
	title->setFont(kNormalFontBig);
	title->setFontColor(fg);
	title->setBackColor(bg);
	title->setFrameColor(bg);
	title->setHoriAlign(kLeftText);
	addView(title);
	y += 20 + gap;

	CTextLabel* nameLabel = new CTextLabel(CRect(left, y, left + 44, y + 22), "Name");
	nameLabel->setFontColor(fg);
	nameLabel->setBackColor(bg);
	nameLabel->setFrameColor(bg);
	nameLabel->setHoriAlign(kLeftText);
	addView(nameLabel);
	nameEdit = new CTextEdit(CRect(left + 48, y, right, y + 22), this, TAG_NAME, "");
	nameEdit->setFontColor(fg);
	nameEdit->setBackColor(bg);
	nameEdit->setFrameColor(fg);
	nameEdit->setHoriAlign(kLeftText);
	addView(nameEdit);
	y += 22 + gap;

	const int buttonCount = 4;
	CCoord buttonWidth = (right - left - gap * (buttonCount - 1)) / buttonCount;
	const char* titles[buttonCount] = { "Save", "Save new", "Delete", "Close" };
	const long tags[buttonCount] = { TAG_SAVE, TAG_SAVE_NEW, TAG_DELETE, TAG_CLOSE };
	for (int i = 0; i < buttonCount; i++)
	{
		CCoord x = left + i * (buttonWidth + gap);
		auto button = new PresetButton(CRect(x, y, x + buttonWidth, y + 24), this, tags[i], titles[i], fg, bg);
		if (tags[i] == TAG_DELETE)
			deleteButton = button;
		addView(button);
	}
	y += 24 + gap;

	CCoord statusTop = panel.bottom - margin - 18;
	list = new PresetList(CRect(left, y, right, statusTop - gap / 2), this, TAG_LIST, loader, fg, bg);
	addView(list);

	status = new CTextLabel(CRect(left, statusTop, right, statusTop + 18), "");
	status->setFont(kNormalFontSmall);
	status->setFontColor(fg);
	status->setBackColor(bg);
	status->setFrameColor(bg);
	status->setHoriAlign(kLeftText);
	addView(status);
}

void PresetBrowser::refresh(int currentPreset)
{
	endNameEdit();
	resetDeleteConfirmation();
	setStatus("Click a preset to load it. Save overwrites the selected preset.");
	list->setSelected(currentPreset);
	list->ensureVisible(currentPreset);
	nameEdit->setText(currentPreset >= 0 ? loader->getName(currentPreset).c_str() : "");
}

void PresetBrowser::setStatus(const char* text)
{
	status->setText(text);
}

void PresetBrowser::resetDeleteConfirmation()
{
	confirmingDelete = false;
	deleteButton->setTitle("Delete");
}

void PresetBrowser::selectAndLoad(int index)
{
	if (index < 0 || index >= loader->count())
		return;
	endNameEdit();
	list->setSelected(index);
	list->ensureVisible(index);
	nameEdit->setText(loader->getName(index).c_str());
	editor->loadPreset(index);
	setStatus(("Loaded \"" + loader->getName(index) + "\"").c_str());
}

// Ends a possibly active edit so that the text control has the typed text and its native window is gone.
// CTextEdit opens the native edit on mouse down without becoming the frame's focus view, and clicking
// elsewhere in the frame doesn't take the keyboard focus from it, so this has to be done explicitly.
void PresetBrowser::endNameEdit()
{
	if (nameEdit->platformControl)
		nameEdit->looseFocus();
}

std::string PresetBrowser::getEnteredName()
{
	endNameEdit();
	return nameEdit->getText();
}

void PresetBrowser::valueChanged(CControl* pControl)
{
	long tag = pControl->getTag();
	if (tag != TAG_DELETE && tag != TAG_NAME)
		resetDeleteConfirmation();

	switch (tag)
	{
	case TAG_LIST:
		selectAndLoad(list->getSelected());
		break;
	case TAG_SAVE:
	case TAG_SAVE_NEW:
	{
		auto name = getEnteredName();
		int wavePack = 0;
		auto params = editor->getCurrentParams(&wavePack);
		int index = list->getSelected();
		bool ok;
		if (tag == TAG_SAVE && index >= 0)
		{
			ok = loader->overwritePreset(index, name, params, wavePack);
		}
		else
		{
			index = loader->savePreset(name, params, wavePack);
			ok = index >= 0;
		}
		if (!ok)
		{
			setStatus("Could not write presets.dat");
			break;
		}
		list->setSelected(index);
		list->ensureVisible(index);
		nameEdit->setText(loader->getName(index).c_str());
		editor->setCurrentPreset(index);
		setStatus(("Saved \"" + loader->getName(index) + "\"").c_str());
		break;
	}
	case TAG_DELETE:
	{
		int index = list->getSelected();
		if (index < 0)
		{
			setStatus("Select a preset to delete");
			break;
		}
		if (!confirmingDelete)
		{
			confirmingDelete = true;
			deleteButton->setTitle("Really delete?");
			setStatus(("Click again to delete \"" + loader->getName(index) + "\"").c_str());
			break;
		}
		resetDeleteConfirmation();
		auto name = loader->getName(index);
		if (!loader->deletePreset(index))
		{
			setStatus("Could not write presets.dat");
			break;
		}
		// the sound stays as it is, it's just not stored as a preset anymore
		list->setSelected(-1);
		nameEdit->setText(name.c_str());
		editor->presetDeleted(index);
		setStatus(("Deleted \"" + name + "\". Save new would restore it.").c_str());
		break;
	}
	case TAG_CLOSE:
	case TAG_BACKDROP:
		editor->closePresetBrowser();
		break;
	}
}

long PresetBrowser::onKeyDown(VstKeyCode& keyCode)
{
	long result = CViewContainer::onKeyDown(keyCode);
	if (result != -1)
		return result;
	int index = list->getSelected();
	switch (keyCode.virt)
	{
	case VKEY_ESCAPE:
		editor->closePresetBrowser();
		return 1;
	case VKEY_UP:
		selectAndLoad(index > 0 ? index - 1 : 0);
		return 1;
	case VKEY_DOWN:
		selectAndLoad(index + 1);
		return 1;
	}
	return -1;
}
