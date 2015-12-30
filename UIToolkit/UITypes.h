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

typedef struct uiCanvas
{
	GLXContext glx;
	NVGcontext* nano;
	
	float left, right, top, bottom, margin,
		fontSize, lineHeight;
	
	NVGcolor fgColour, bgColour;
	int bgImage;
	NVGpaint bgImagePattern;
} uiCanvas;

typedef struct uiWindow
{
	xWindow window;
	uiCanvas canvas;
} uiWindow;

typedef enum { uiNoStop, uiStop } uiState;
typedef enum { uiText, uiImage } uiItemType;

typedef struct uiBox { float x, y, width, height; } uiBox;
typedef struct uiIndices { int par, tab, word; } uiIndices;

typedef uiState (*uiWordAction)(uiWindow, uiItemType, const char*, float, float, uiBox, uiIndices, void*);


// Helper functions

bool inBox(uiBox box, float x, float y)
{
	return x >= box.x
		&& x <= box.x + box.width
		&& y >= box.y
		&& y <= box.y + box.height;
}
uiBox boxXYWH(float x, float y, float width, float height)
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

#endif
