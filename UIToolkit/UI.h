#ifndef UI_SHORTHAND_H
#define UI_SHORTHAND_H

#include "UIToolkit/UIToolkit.h"
#include "Theme.h"

struct UI
{
	uiWindow (*get)(int, int, int, int);
	void (*redraw)(uiWindow, const char* markup);
	uiEvent (*wait)(uiWindow, const char* markup);
	bool (*at)(uiEvent, int, int, int);
	int (*fullWidth)();
	int (*fullHeight)();
	int (*width)(uiWindow);
	int (*height)(uiWindow);
	
	float barHeight;
};

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

const struct UI UI = {
	uiGetDefault, uiRedraw, uiGetEvent, uiEventTarget,
	uiRootWidth, uiRootHeight, uiWidth, uiHeight,
	themeBarHeight
};

#endif
