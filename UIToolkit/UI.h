#ifndef UI_SHORTHAND_H
#define UI_SHORTHAND_H

#include "UIToolkit/UIToolkit.h"
#include "Theme.h"

struct UI
{
	uiWindow (*get)(int, int, int, int);
	void (*close)(uiWindow);

	uiWindow (*draw)(uiWindow, char* markup);
	void (*redraw)(uiWindow);
	
	uiWindow (*scroll)(uiWindow, int amount);
	uiEvent (*wait)(uiWindow);
	
	bool (*at)(uiEvent, int, int, int);
	int (*button)(uiEvent);
	void (*highlight)(uiWindow, uiEvent);
	//~ void (*highlightAt)(uiWindow, int, int, int);
	
	int (*fullWidth)();
	int (*fullHeight)();
	int (*width)(uiWindow);
	int (*height)(uiWindow);
	
	uiWindow (*colour)(uiWindow, NVGcolor fg, NVGcolor bg);
	uiWindow (*highlightStyle)(uiWindow, uiAlign side, NVGcolor highlight);
	NVGcolor (*fg)();
	NVGcolor (*bg)();
	NVGcolor (*highlightFg)();
	NVGcolor (*highlightBg)();
	
	uiWindow (*popup)(uiWindow, uiEvent);
	
	float barHeight;
	uiAlign left;
};

int uiButton(uiEvent event) { return event.xEvent.xbutton.button; }

int uiRootWidth()
{
	xWindow root = xGetRoot(NULL);
	return root.attributes.width;
}
int uiRootHeight()
{
	xWindow root = xGetRoot(NULL);
	return root.attributes.width;
}

int uiWidth(uiWindow ui) { return ui.window.attributes.width; }
int uiHeight(uiWindow ui) { return ui.window.attributes.height; }

NVGcolor uiFg() { return nvgHSL(themePanelFg); }
NVGcolor uiBg() { return nvgHSL(themePanelBg); }
NVGcolor uiHighlightFg() { return nvgHSL(themeHighlightFg); }
NVGcolor uiHighlightBg() { return nvgHSL(themeHighlightBg); }

uiWindow uiPopup(uiWindow ui, uiEvent event)
{
	return uiGetDefault(
		event.box.x,
		event.box.y + event.box.height,
		200, 400
	);
}

const struct UI UI = {
	uiGetDefault, uiClose,
	uiAutoDraw, uiRedraw, uiScroll, uiGetEvent,
	uiEventTargetIs, uiButton, uiHighlight,
	uiRootWidth, uiRootHeight, uiWidth, uiHeight,
	uiSetColours, uiSetHighlight, uiFg, uiBg, uiHighlightFg, uiHighlightBg,
	uiPopup,
	themeBarHeight, uiLeft
};

#endif
