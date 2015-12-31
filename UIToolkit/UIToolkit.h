#ifndef UI_TOOLKIT_H
#define UI_TOOLKIT_H

#include "UIToolkit/UITypes.h"
#include "UIToolkit/UIParser.h"
#include "UIToolkit/X11-Interface.h"
#include "Theme.h"


// Initialising

uiWindow uiGetOn(xWindow root, uiBox box)
{
	uiWindow ui = {0};
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
	ui.canvas.fontSize = themeFontSize;
	ui.canvas.lineHeight = themeLineHeight;
	ui.canvas.fgColour = nvgHSL(themePanelFg);
	ui.canvas.bgColour = nvgHSL(themePanelBg);
	ui.canvas.highlight = nvgHSL(themeHighlightBg);
	ui.canvas.highlightStyle = uiCentre;
	
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
		PointerMotionMask|ButtonPressMask|ButtonReleaseMask|EnterWindowMask|LeaveWindowMask);
	
	XMapWindow(ui.window.display, ui.window.id);
	return ui;
}

uiWindow uiGetDefault(int x, int y, int width, int height)
{
	return uiGet(NULL, uiGetBox(x, y, width, height));
}


// Closing

void uiClose(uiWindow ui)
{
	nvgDeleteImage(ui.canvas.nano, ui.canvas.bgImage);	
	nvgDeleteGL2(ui.canvas.nano);
	XDestroyWindow(ui.window.display, ui.window.id);
	XCloseDisplay(ui.window.display);
}


// Modify

uiWindow uiSetColours(uiWindow ui, NVGcolor fg, NVGcolor bg)
{
	ui.canvas.fgColour = fg;
	ui.canvas.bgColour = bg;
	return ui;
}

uiWindow uiSetHSL(uiWindow ui,
	float fgH, float fgS, float fgL,
	float bgH, float bgS, float bgL)
{
	return uiSetColours(ui, nvgHSL(fgH, fgS, fgL), nvgHSL(bgH, bgS, bgL));
}

uiWindow uiSetImage(uiWindow ui, const char* image)
{
	ui.canvas.bgImage = nvgCreateImage(ui.canvas.nano, image, 0);
	ui.canvas.bgImagePattern = nvgImagePattern(ui.canvas.nano,
		0, 0, ui.window.attributes.width, ui.window.attributes.height, 0,
		ui.canvas.bgImage, 1
	);
	return ui;
}

uiWindow uiSetHighlight(uiWindow ui, uiAlign side, NVGcolor highlight)
{
	ui.canvas.highlightStyle = side;
	ui.canvas.highlight = highlight;
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


uiState uiDrawAction(uiWindow ui, uiItemType itemType, uiWordData word, uiParseContext* passedData)
{
	switch(itemType)
	{
		case uiText:
			nvgFillColor(ui.canvas.nano, ui.canvas.fgColour);
			nvgText(ui.canvas.nano, startX, startY, word, NULL);
			break;
		case uiHighlighted:
			nvgFillColor(ui.canvas.nano, ui.canvas.highlight);
			switch(ui.canvas.highlightStyle)
			{
				case uiCentre:
					nvgBeginPath(ui.canvas.nano);
					nvgRect(ui.canvas.nano, box.x, box.y + box.height - ui.canvas.margin, box.width, ui.canvas.margin);
					nvgFill(ui.canvas.nano);
					break;
				case uiLeft:
					nvgBeginPath(ui.canvas.nano);
					nvgRect(ui.canvas.nano, ui.canvas.left, box.y, ui.canvas.margin, box.height);
					nvgFill(ui.canvas.nano);
					
					//~ nvgBeginPath(ui.canvas.nano);
					//~ nvgRect(ui.canvas.nano, ui.canvas.right - ui.canvas.margin, box.y, ui.canvas.margin, box.height);
					//~ nvgFill(ui.canvas.nano);
					break;
			}
			nvgFillColor(ui.canvas.nano, ui.canvas.fgColour);
			nvgText(ui.canvas.nano, startX, startY, word, NULL);
			break;
	}
	return uiNoStop;	// Keep going
}

void uiDraw(uiWindow ui)
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
	uiParse(ui, uiDrawAction, NULL);
}

uiWindow uiDrawInBox(uiWindow ui, uiBox box, char* markup)
{
	ui.canvas.left = box.x;
	ui.canvas.right = box.x + box.width;
	ui.canvas.top = box.y;
	ui.canvas.bottom = box.y + box.height;
	ui.canvas.markup = markup;
	
	uiDraw(ui);
	
	return ui;
}

void uiRedraw(uiWindow ui)
{
	uiBeginFrame(ui);
	uiDraw(ui);
	uiEndFrame(ui);
}

uiWindow uiAutoDraw(uiWindow ui, char* markup)
{
	ui.canvas.markup = markup;
	uiRedraw(ui);
	return ui;
}


// Reverse index lookup (from x, y to par, tab, word)

uiState uiIndexAction(uiWindow ui,
	uiItemType itemType,
	uiWordData data,
	uiParseContext* context)
{
	if(uiInBox(box, event->x, event->y))
	{
		event->startX = startX;
		event->startY = startY;
		event->box = box;
		event->target = position;
		return uiStop; 	// Finish
	}
	else
	{
		return uiNoStop; // Keep going
	}
}

uiEvent* uiIndexAt(uiWindow ui, float x, float y, uiEvent* event)
{
	event->x = x;
	event->y = y;

	uiBeginFrame(ui);
	if(uiParse(ui, uiIndexAction, event) == uiNoStop)
		{ event = NULL; }
	uiEndFrame(ui);
	
	return event;
}

uiEvent* uiIndexInBox(uiWindow ui, uiBox box, float x, float y, uiEvent* event)
{
	ui.canvas.left = box.x;
	ui.canvas.right = box.x + box.width;
	ui.canvas.top = box.y;
	ui.canvas.bottom = box.y + box.height;
	
	return uiIndexAt(ui, x, y, event);
}


// Events

uiEvent uiGetEvent(uiWindow ui)
{
	uiEvent event = {0};
	XNextEvent(ui.window.display, &event.xEvent);
	event.type = event.xEvent.type;
	
	switch(event.type)
	{
		case MotionNotify:
			uiIndexAt(ui, event.xEvent.xmotion.x, event.xEvent.xmotion.y, &event);
			break;
	}
	
	return event;
}


// Scrolling

uiWindow uiScroll(uiWindow ui, int amount)
{
	uiEvent event;
	uiParse(ui, NULL, &event);
	ui.canvas.scrollHeight = -ui.canvas.scrollY + event.finalY + ui.canvas.lineHeight;
	ui.canvas.scrollY += amount * themeScrollBy;
	ui.canvas.scrollY = min(0, ui.canvas.scrollY);
	ui.canvas.scrollY = max(ui.window.attributes.height - ui.canvas.scrollHeight, ui.canvas.scrollY);
	uiRedraw(ui);
	return ui;
}


// Modifying markup

void uiHighlight(uiWindow ui, uiEvent event)
{
	if(event.target.par <= 0||event.target.tab <= 0 || event.target.word <= 0) { return; }
	printf("%i.%i.%i\n", event.target.par, event.target.tab, event.target.word);
	
	char head[strlen(ui.canvas.markup) + 1];
	strcpy(head, ui.canvas.markup);
	
	int target = 0;
	
	for(int i = 1; i < event.target.par; i++)
		{ target += strcspn(head+target, "\n"); target += strspn(head+target, "\n"); }
	for(int i = 1; i < event.target.tab; i++)
		{ target += strcspn(head+target, "\t"); target += strspn(head+target, "\t"); }
	for(int i = 1; i < event.target.word; i++)
		{ target += strcspn(head+target, " "); target += strspn(head+target, " "); }
	
	char markup[strlen(ui.canvas.markup) + 3];
	memset(markup, 0, strlen(ui.canvas.markup) + 3);
	
	strncpy(markup, head, target);
	strcat(markup, "|");
	
	int wordLength = strcspn(head+target, " \t\n");	
	strncat(markup, head+target, wordLength);
	
	strcat(markup, "|");
	strcat(markup, head+target+wordLength);

	uiAutoDraw(ui, markup);
}

#endif
