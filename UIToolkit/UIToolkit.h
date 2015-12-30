#ifndef UI_TOOLKIT_H
#define UI_TOOLKIT_H

#include "UIToolkit/UITypes.h"
#include "UIToolkit/UIParser.h"
#include "UIToolkit/X11-Interface.h"
#include "Theme.h"


// Initialising

uiWindow uiGetOn(xWindow root, uiBox box)
{
	uiWindow ui;
	ui.window = xCreate(root, box.x, box.y, box.width, box.height);
	
	int glxCount;
	int requests[] = {
		GLX_RENDER_TYPE, GLX_RGBA_BIT,
		GLX_DOUBLEBUFFER, false,
		0
	};
	
	GLXFBConfig* glxOptions = glXChooseFBConfig(
		ui.window.display,
		XDefaultScreen(ui.window.display),
		requests, &glxCount
	);
	
	ui.canvas.glx = glXCreateNewContext(ui.window.display, glxOptions[0], GLX_RGBA_TYPE, NULL, true);	
	glXMakeCurrent(ui.window.display, ui.window.id, ui.canvas.glx);
	
	if(glewInit() != GLEW_OK) { printf("Failed to start glew\n"); }
	
	ui.canvas.nano = nvgCreateGL2(NVG_ANTIALIAS);
	if (ui.canvas.nano == NULL) { printf("Failed to start NanoVG\n"); }
	
	// Default settings
	ui.canvas.left = 0;
	ui.canvas.right = ui.window.attributes.width;
	ui.canvas.top = 0;
	ui.canvas.bottom = ui.window.attributes.height;
	ui.canvas.margin = themeMargin;
	ui.canvas.fontSize = 13;
	ui.canvas.lineHeight = 22;
	ui.canvas.fgColour = nvgHSL(themePanelFg);
	ui.canvas.bgColour = nvgHSL(themePanelBg);
	
	// Default fonts
	if (nvgCreateFont(ui.canvas.nano, "normal", themeFont) == -1)
		{ printf("Failed to create font normal.\n"); }
	if (nvgCreateFont(ui.canvas.nano, "bold", themeFontBold) == -1)
		{ printf("Failed to create font bold.\n"); }
	if (nvgCreateFont(ui.canvas.nano, "italic", themeFontItalic) == -1)
		{ printf("Failed to create font italic.\n"); }
	
	return ui;
}

uiWindow uiGet(const char* displayName, uiBox box)
{
	xWindow root = xGetRoot(displayName);
	uiWindow ui = uiGetOn(root, box);
	
	XSelectInput(ui.window.display, ui.window.id,
		PointerMotionMask|ButtonPressMask|ButtonReleaseMask);
	
	XMapWindow(ui.window.display, ui.window.id);
	return ui;
}

uiWindow uiGetDefault(int x, int y, int width, int height)
{
	return uiGet(NULL, getBox(x, y, width, height));
}

uiWindow uiSetColours(uiWindow ui,
	float fgH, float fgS, float fgL,
	float bgH, float bgS, float bgL,
	const char* bgImage)
{
	ui.canvas.fgColour = nvgHSL(fgH, fgS, fgL);
	ui.canvas.bgColour = nvgHSL(bgH, bgS, bgL);
	ui.canvas.bgImage = nvgCreateImage(ui.canvas.nano, bgImage, 0);
	ui.canvas.bgImagePattern = nvgImagePattern(ui.canvas.nano,
		0, 0, ui.window.attributes.width, ui.window.attributes.height, 0,
		ui.canvas.bgImage, 1
	);
	return ui;
}


// Drawing

void uiBeginFrame(uiWindow ui)
{
	glXMakeCurrent(ui.window.display, ui.window.id, ui.canvas.glx);
	nvgBeginFrame(ui.canvas.nano, ui.window.attributes.width, ui.window.attributes.height, 1/1);
}
void uiEndFrame(uiWindow ui)
{
	nvgEndFrame(ui.canvas.nano);
	glFlush();
}


uiState uiDrawAction(uiWindow ui,
	uiItemType type,
	const char* drawData,
	float startX, float startY,
	uiBox box,
	uiIndices position,
	void* passedData)
{
	nvgText(ui.canvas.nano, startX, startY, drawData, NULL);
	return uiNoStop;	// Keep going
}

void uiDraw(uiWindow ui, const char* markup)
{	
	// Background
	nvgBeginPath(ui.canvas.nano);
	
	if(ui.canvas.bgImage)
		{ nvgFillPaint(ui.canvas.nano, ui.canvas.bgImagePattern); }
	else
		{ nvgFillColor(ui.canvas.nano, ui.canvas.bgColour); }

	nvgRect(ui.canvas.nano, ui.canvas.left, ui.canvas.top, ui.canvas.right - ui.canvas.left, ui.canvas.bottom - ui.canvas.top);
	nvgFill(ui.canvas.nano);
	
	// Parse markup
	uiParse(ui, markup, uiDrawAction, NULL);
}

void uiDrawInBox(uiWindow ui, uiBox box, const char* markup)
{
	ui.canvas.left = box.x;
	ui.canvas.right = box.x + box.width;
	ui.canvas.top = box.y;
	ui.canvas.bottom = box.y + box.height;
	
	uiDraw(ui, markup);
	
	// Bounds will revert automatically since window is not returned
}

void uiRedraw(uiWindow ui, const char* markup)
{
	uiBeginFrame(ui);
	uiDraw(ui, markup);
	uiEndFrame(ui);
}


// Reverse index lookup (from x, y to par, tab, word)

uiState uiIndexAction(uiWindow ui,
	uiItemType type,
	const char* drawData,
	float startX, float startY,
	uiBox box,
	uiIndices position,
	void* passedData)
{	
	float x = ((float*)passedData)[0];
	float y = ((float*)passedData)[1];
	if(inBox(box, x, y))
	{
		((int*)passedData)[0] = position.par;
		((int*)passedData)[1] = position.tab;
		((int*)passedData)[2] = position.word;
		return uiStop;	// Finish
	}
	else
		{ return uiNoStop; }	// Keep going
}

int* uiIndexAt(uiWindow ui, const char* markup, float x, float y, void* dataStorage)
{
	((float*)dataStorage)[0] = x;
	((float*)dataStorage)[1] = y;

	uiBeginFrame(ui);
	if(uiParse(ui, markup, uiIndexAction, dataStorage) == uiNoStop)
		{ dataStorage = NULL; }
	uiEndFrame(ui);
	
	return dataStorage;
}

int* uiIndexInBox(uiWindow ui, uiBox box, const char* markup, float x, float y, void* dataStorage)
{
	ui.canvas.left = box.x;
	ui.canvas.right = box.x + box.width;
	ui.canvas.top = box.y;
	ui.canvas.bottom = box.y + box.height;
	
	return uiIndexAt(ui, markup, x, y, dataStorage);
}


// Events

uiEvent uiGetEvent(uiWindow panel, const char* markup)
{
	uiEvent event;
	XNextEvent(panel.window.display, &event.xEvent);
	event.type = event.xEvent.type;
	
	char dataStorage[max(2*sizeof(float), 3*sizeof(int))];
	int* target;
	
	switch(event.type)
	{
		case ButtonPress:
			target = uiIndexAt(panel, markup, event.xEvent.xbutton.x, event.xEvent.xbutton.y, dataStorage);
			event.target.par = target[0];
			event.target.tab = target[1];
			event.target.word = target[2];
			break;
	}
	
	return event;
}

#endif
