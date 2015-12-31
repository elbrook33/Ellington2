// ui-types.h
// Define data types and load libraries here

#ifndef UI_TYPES_H
#define UI_TYPES_H


// Requirements

#include <GL/glew.h>
#include <GL/glx.h>
#include <GL/gl.h>
#define NANOVG_GLEW
#define NANOVG_GL2_IMPLEMENTATION
#include "nanovg.h"
#include "nanovg_gl.h"
#include <X11/Xlib.h>

#include <X11/Xlib.h>
#include <X11/extensions/Xcomposite.h>
#include <X11/Xatom.h>	// For WINDOW_TYPE
#include <X11/Xutil.h>	// For titles and app names

#include <stdbool.h>
#include <stdio.h>
#include <string.h>


// Types

typedef struct xWindow
{
	Display* display;
	Window id;
	XWindowAttributes attributes;
	XClassHint name;
} xWindow;

typedef enum { uiNoStop, uiStop } uiState;
typedef enum { uiText, uiHighlighted, uiImage } uiItemType;
typedef enum { uiLeft, uiCentre, uiRight } uiAlign;

typedef struct uiBox { float x, y, width, height; } uiBox;
typedef struct uiIndices { int par, tab, word; } uiIndices;

typedef struct uiCanvas
{
	GLXContext glx;
	NVGcontext* nano;
	char* markup;
	
	float left, right, top, bottom, margin,
		fontSize, lineHeight, spaceWidth,
		scrollHeight, scrollY;
	
	NVGcolor fgColour, bgColour, highlight;
	int bgImage;
	NVGpaint bgImagePattern;
	uiAlign highlightStyle;
} uiCanvas;

typedef struct uiWindow
{
	xWindow window;
	uiCanvas canvas;
} uiWindow;

typedef struct uiWordData
{
	const char* text;
	float wordX, wordY;
	uiIndices position, lengths;
	uiBox box;
} uiWordData;

typedef struct uiParseContext
{
	struct {
		float eventX, eventY;
	} in;
	uiWordData out;
} uiParseContext;

typedef struct uiEvent
{
	int type;
	XEvent xEvent;
	float x, y;
	uiIndices target;
} uiEvent;

typedef uiState (*uiWordAction)(uiWindow, uiItemType, uiWordData, uiParseContext*);


// Helper functions

bool uiInBox(uiBox box, float x, float y)
{
	return x >= box.x
		&& x <= box.x + box.width
		&& y >= box.y
		&& y <= box.y + box.height;
}
uiBox uiGetBox(float x, float y, float width, float height)
{
	uiBox box;
	box.x = x;
	box.y = y;
	box.width = width;
	box.height = height;
	return box;
}
uiBox boxXYXY(float* bounds)
{
	uiBox box;
	box.x = bounds[0];
	box.y = bounds[1];
	box.width = bounds[2] - bounds[0];
	box.height = bounds[3] - bounds[1];
	return box;
}
uiBox boxWindow(xWindow window)
{
	uiBox box;
	box.x = window.attributes.x;
	box.y = window.attributes.y;
	box.width = window.attributes.width;
	box.height = window.attributes.height;
	return box;
}
bool uiEventTargetIs(uiEvent event, int par, int tab, int word)
{
	return event.target.par == par && event.target.tab == tab && event.target.word == word;
}

#endif
