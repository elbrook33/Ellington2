#ifndef WALLPAPER_H
#define WALLPAPER_H

#include "UIToolkit/UIToolkit.h"


// Set up

desktopSession desktopGet(wmSession session)
{
	desktopSession desktop;
	desktop.ui = uiGetOn(session.root, boxWindow(session.root));
	desktop.ui = uiSetColours(desktop.ui, themeTitlebarFg, themeTitlebarInactive, themeWallpaper);
	
	XSelectInput(desktop.ui.window.display, desktop.ui.window.id,
		PointerMotionMask|ButtonPressMask|ButtonReleaseMask);
	XMapWindow(desktop.ui.window.display, desktop.ui.window.id);
	
	return desktop;
}


// Drawing

void desktopRedraw(wmSession session)
{
	// Copy as a shorthand
	uiWindow ui = session.desktop.ui;
	
	// Start drawing
	uiBeginFrame(ui);
	
	// Background image
	uiDraw(ui, NULL);
	ui.canvas.bgImage = false;
	
	for(int i = 0; i < wmLength(session); i++)
	{
		xWindow window = wmWorkspace(session)[i];
		
		uiBox box = boxXYWH(
			window.attributes.x,
			window.attributes.y - themeBarHeight,
			window.attributes.width,
			themeBarHeight
		);
		
		if(i == session.activeWindow)
		{
			if(!session.desktop.dragEvent.active)
			{
				// Shadow
				int shadowSize = 20, shadowOffset = 10;
				nvgBeginPath(ui.canvas.nano);
				nvgFillPaint(ui.canvas.nano,
					nvgBoxGradient(ui.canvas.nano,
						window.attributes.x,
						window.attributes.y - themeLineHeight + shadowOffset,
						window.attributes.width,
						window.attributes.height + themeLineHeight,
						shadowSize / 2,
						2 * shadowSize,
						nvgHSLA(0, 0, 0, 191),
						nvgHSLA(0, 0, 0, 0)
					)
				);
				nvgRect(ui.canvas.nano,
					window.attributes.x - shadowSize,
					window.attributes.y - themeLineHeight - shadowSize + shadowOffset,
					window.attributes.width + 2 * shadowSize,
					window.attributes.height + themeLineHeight + 2 * shadowSize
				);
				nvgFill(ui.canvas.nano);
			}
			
			// Darker titlebar
			ui.canvas.bgColour = nvgHSL(themeTitlebarActive);
			uiDrawInBox(ui, box, "> *×* >");
		}
		else
		{
			// Lighter titlebar
			ui.canvas.bgColour = nvgHSL(themeTitlebarInactive);
			uiDrawInBox(ui, box, NULL);
		}
	}
	
	uiEndFrame(ui);
}


// Drag events

int desktopWindowAt(wmSession session, int x, int y)
{
	int xMid = session.desktop.ui.window.attributes.width / 2,
		yMid = (session.desktop.ui.window.attributes.height - themeBarHeight) / 2 + themeBarHeight;

	if(x > xMid && wmLength(session) > 1)
	{
		if(y > yMid && wmLength(session) > 2)
			{ return 2; }
		else
			{ return 1; }
	}
	else
	{
		if(y > yMid && wmLength(session) > 3)
			{ return 3; }
		else
			{ return 0; }
	}
}

int desktopTitlebarAt(wmSession session, int x, int y)
{
	for(int window = 0; window < wmLength(session); window++)
	{
		if(
			inBox(boxXYWH(
				wmWorkspace(session)[window].attributes.x,
				wmWorkspace(session)[window].attributes.y - themeBarHeight,
				wmWorkspace(session)[window].attributes.width,
				themeBarHeight),
			x, y)
		)
			{ return window; }
	}
	return -1;
}

wmSession desktopEvents(wmSession session)
{
	// Shorthands
	Display* display = session.desktop.ui.window.display;
	desktopDragState drag = session.desktop.dragEvent;		
	
	// Check if there are any events
	if(XPending(display) == 0)
		{ return session; }
	
	XEvent event;
	XNextEvent(display, &event);

	// Set up data before switch
	int newX, newY;
	int targetIndex;

	char indexData[max(2*sizeof(float), 3*sizeof(int))];
	
	switch(event.type)
	{
		case EnterNotify: // From mapped windows (assigned on MapRequest)
			if(session.desktop.dragEvent.active)
				{ break; }
			
			for(int window = 0; window < wmLength(session); window++)
			{
				if(wmWorkspace(session)[window].id == event.xcrossing.window)
				{
					session.activeWindow = window;
					desktopRedraw(session);
					break;
				}
			}
			break;
		
		case ButtonPress: // On desktop itself
			targetIndex = desktopTitlebarAt(session, event.xbutton.x_root, event.xbutton.y_root);
			
			// Check if clicked on close button	
			if(targetIndex > -1 &&
				uiIndexInBox(session.desktop.ui,
					boxXYWH(
						wmWorkspace(session)[targetIndex].attributes.x,
						wmWorkspace(session)[targetIndex].attributes.y - themeBarHeight,
						wmWorkspace(session)[targetIndex].attributes.width,
						themeBarHeight
					),
					"> *×* >",
					event.xbutton.x_root, event.xbutton.y_root,
					indexData
				)
			) { 
				printf("Killing %lui\n", wmWorkspace(session)[targetIndex].id);
				XKillClient(display, wmWorkspace(session)[targetIndex].id);
			}
			else
			{
				// Start drag
				drag.active = true;
				drag.index = desktopWindowAt(session, event.xbutton.x_root, event.xbutton.y_root);
				drag.window = wmWorkspace(session)[drag.index];
				drag.offsetX = event.xbutton.x_root - drag.window.attributes.x;
				drag.offsetY = event.xbutton.y_root - drag.window.attributes.y;
				
				session.desktop.dragEvent = drag;
			}
			break;
		
		case MotionNotify:
		
			// Handle non-drag (focus follows mouse)
			
			if(!session.desktop.dragEvent.active)
			{
				targetIndex = desktopWindowAt(session, event.xmotion.x_root, event.xmotion.y_root);
				if(targetIndex != session.activeWindow)
				{
					session.activeWindow = targetIndex;
					desktopRedraw(session);
				}
				break;
			}
			
			
			//
			// Handle drag
			//
			
			newX = event.xmotion.x_root - drag.offsetX;
			newY = max(event.xmotion.y_root - drag.offsetY, 2*themeBarHeight);
			
			
			// Limit movements
			
			if(drag.index == 0 || drag.index == 3)
			{
				if(wmLength(session) > 1)
				{
					// Limit left-hand windows from moving right too far
					newX = min(newX,
						themeOuterSpace + 2*themeInnerSpace
					);
				}
			}
			else
			{
				// Limit right-hand windows from moving left too far
				newX = max(newX,
					session.root.attributes.width/2 - themeInnerSpace
				);
			}

			if(drag.index <= 1)
			{
				if(drag.index == 0 && wmLength(session) > 3
					|| drag.index == 1 && wmLength(session) > 2)
				{
					// Limit upper windows from moving down too far
					newY = min(newY,
						themeBarHeight + themeOuterSpace + themeBarHeight + 2*themeInnerSpace
					);
				}
			}
			else
			{
				// Limit lower windows from moving up too far
				newY = max(newY,
					(session.root.attributes.height - themeBarHeight)/2 + themeBarHeight
					- themeInnerSpace + themeBarHeight
				);
			}
			
			
			// Move windows and redraw
			
			XMoveWindow(display, drag.window.id, newX, newY);
			session = wmUpdateWindowAttributes(session);
			desktopRedraw(session);
			
			
			// Check if overlapping
			
			// Top-left corner
			targetIndex = desktopWindowAt(session,
				newX,
				newY - themeBarHeight
			);
			if(targetIndex != drag.index)
				{ session.activeWindow = targetIndex; break; }
			
			// Top-right corner
			targetIndex = desktopWindowAt(session,
				newX + drag.window.attributes.width,
				newY - themeBarHeight
			);
			if(targetIndex != drag.index)
				{ session.activeWindow = targetIndex; break; }
			
			// Bottom-right corner
			targetIndex = desktopWindowAt(session,
				newX + drag.window.attributes.width,
				newY + drag.window.attributes.height
			);
			if(targetIndex != drag.index)
				{ session.activeWindow = targetIndex; break; }
			
			// Bottom-left corner
			targetIndex = desktopWindowAt(session,
				newX,
				newY + drag.window.attributes.height
			);
			if(targetIndex != drag.index)
				{ session.activeWindow = targetIndex; break; }

			break;
		
		case ButtonRelease:
			// End drag
			if(session.desktop.dragEvent.active)
			{
				session.desktop.dragEvent.active = false;
				session = wmSwapPlaces(session, session.activeWindow, session.desktop.dragEvent.index);
			}
			break;
	}
	
	return desktopEvents(session);
}

#endif
