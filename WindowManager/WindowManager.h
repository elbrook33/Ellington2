//
// Window manager
//
// This is the main starting point for the app.
// It starts following new windows and sets up hot keys.
// It also launches the desktop wallpaper (which handles shadows and window movements) and panel.
//

#ifndef WINDOW_MANAGER_H
#define WINDOW_MANAGER_H


// Requirements

#include "UIToolkit/X11-Interface.h"
#include "Misc/Helpers.h"


// Public types and functions

typedef struct desktopDragState
{
	bool active;
	xWindow window;
	int index;
	int offsetX, offsetY;
} desktopDragState;

typedef struct desktopSession
{
	uiWindow ui;
	desktopDragState dragEvent;
} desktopSession;

typedef struct wmSession
{
	xWindow root;
	desktopSession desktop;
	xWindow workspaces[5][4];
	int workspaceLengths[5];
	int activeWorkspace, activeWindow;
} wmSession;

#define wmWorkspace(session) session.workspaces[session.activeWorkspace]
#define wmLength(session) session.workspaceLengths[session.activeWorkspace]

wmSession wmSwapPlaces(wmSession, int indexA, int indexB);
wmSession wmUpdateWindowAttributes(wmSession);

#include "WindowManager/Desktop.h"


// Functions

int wmErrorHandler(Display*, XErrorEvent*);

wmSession wmGet(const char* displayID)
{
	xWindow root = xGetRoot(displayID);
	
//	XCompositeRedirectSubwindows(root.display, root.id, CompositeRedirectAutomatic);
	XSelectInput(root.display, root.id,
		SubstructureRedirectMask|SubstructureNotifyMask
	);
	
	// 1-5
	XGrabKey(root.display, 10, Mod4Mask, root.id, true, GrabModeAsync, GrabModeAsync);
	XGrabKey(root.display, 11, Mod4Mask, root.id, true, GrabModeAsync, GrabModeAsync);
	XGrabKey(root.display, 12, Mod4Mask, root.id, true, GrabModeAsync, GrabModeAsync);
	XGrabKey(root.display, 13, Mod4Mask, root.id, true, GrabModeAsync, GrabModeAsync);
	XGrabKey(root.display, 14, Mod4Mask, root.id, true, GrabModeAsync, GrabModeAsync);
	
	XGrabKey(root.display, 27, Mod4Mask, root.id, true, GrabModeAsync, GrabModeAsync);	// R
	XGrabKey(root.display, 28, Mod4Mask, root.id, true, GrabModeAsync, GrabModeAsync);	// T
	
	// Create components
	wmSession session;
	session.root = root;
	
	session.desktop = desktopGet(session);
	desktopRedraw(session);
	
	XSetErrorHandler(wmErrorHandler);
	
	return session;
}

wmSession wmUpdateWindowAttributes(wmSession session)
{
	for(int window = 0; window < wmLength(session); window++)
	{
		wmWorkspace(session)[window] = xUpdateAttributes(wmWorkspace(session)[window]);
	}
	return session;
}


// Layout

wmSession wmLayoutWorkspace(wmSession session)
{
	printf("Layout %i x %i\n", session.activeWorkspace, wmLength(session));
	
	const int
		topY = themeBarHeight + themeOuterSpace + themeBarHeight,
		leftX = themeOuterSpace,
		
		fullW = session.root.attributes.width - leftX - themeOuterSpace,
		halfW = fullW / 2 - themeInnerSpace,
		
		fullH = session.root.attributes.height - topY - themeOuterSpace,
		halfH = (themeBarHeight + fullH) / 2 - themeBarHeight - themeInnerSpace,
		
		rightX = leftX + halfW + 2*themeInnerSpace,
		bottomY = topY + halfH + 2*themeInnerSpace + themeBarHeight;
	
	switch(wmLength(session))
	{
		case 1:
			XMoveResizeWindow(session.root.display, wmWorkspace(session)[0].id,
				leftX, topY, fullW, fullH
			);
			break;
		case 2:
			XMoveResizeWindow(session.root.display, wmWorkspace(session)[0].id,
				leftX, topY, halfW, fullH
			);
			XMoveResizeWindow(session.root.display, wmWorkspace(session)[1].id,
				rightX, topY, halfW, fullH
			);
			break;
		case 3:
			XMoveResizeWindow(session.root.display, wmWorkspace(session)[0].id,
				leftX, topY, halfW, fullH
			);
			XMoveResizeWindow(session.root.display, wmWorkspace(session)[1].id,
				rightX, topY, halfW, halfH
			);
			XMoveResizeWindow(session.root.display, wmWorkspace(session)[2].id,
				rightX, bottomY, halfW, halfH
			);
			break;
		case 4:
			XMoveResizeWindow(session.root.display, wmWorkspace(session)[0].id,
				leftX, topY, halfW, halfH
			);
			XMoveResizeWindow(session.root.display, wmWorkspace(session)[1].id,
				rightX, topY, halfW, halfH
			);
			XMoveResizeWindow(session.root.display, wmWorkspace(session)[2].id,
				rightX, bottomY, halfW, halfH
			);
			XMoveResizeWindow(session.root.display, wmWorkspace(session)[3].id,
				leftX, bottomY, halfW, halfH
			);
			break;
	}
	
	session = wmUpdateWindowAttributes(session);
	desktopRedraw(session);
	
	return session;
}


// Modify

wmSession wmSwitchToWorkspace(wmSession session, int workspace)
{
	if(workspace == session.activeWorkspace) { return session; }

	for(int window = 0; window < wmLength(session); window++)
	{
		XUnmapWindow(session.root.display, wmWorkspace(session)[window].id);
	}
	
	session.activeWorkspace = workspace;
	session = wmLayoutWorkspace(session);
	
	for(int window = 0; window < wmLength(session); window++)
	{
		XMapWindow(session.root.display, wmWorkspace(session)[window].id);
	}
	return session;
}

wmSession wmSwapPlaces(wmSession session, int indexA, int indexB)
{
	xWindow placeholder = wmWorkspace(session)[indexA];
	wmWorkspace(session)[indexA] = wmWorkspace(session)[indexB];
	wmWorkspace(session)[indexB] = placeholder;
	
	session = wmLayoutWorkspace(session);
	return session;
}

wmSession wmRemoveByID(wmSession session, int workspace, Window id)
{
	for(int window = 0; window < session.workspaceLengths[workspace]; window++)
	{
		// Find matching window in each workspace
		if(session.workspaces[workspace][window].id == id)
		{
			// Shift other windows into its place
			for(int shift = window + 1; shift < session.workspaceLengths[workspace]; shift++)
			{
				session.workspaces[workspace][shift - 1] = session.workspaces[workspace][shift];
			}
			session.workspaceLengths[workspace] -= 1;
			break;
		}
	}
	return session;
}

wmSession wmRemoveEverywhereByID(wmSession session, Window id)
{
	for(int workspace = 0; workspace < 5; workspace++)
	{
		session = wmRemoveByID(session, workspace, id);
	}
	return session;
}


// Events

wmSession wmGlobalEvents(wmSession session)
{
	if(XPending(session.root.display) == 0)
		return session;
	
	XEvent event;
	XNextEvent(session.root.display, &event);

	xWindow targetedWindow;
	switch(event.type)
	{
		case MapRequest:
			printf("wmGlobalEvent MapRequest %lui\n", event.xmaprequest.window);
			
			targetedWindow = xWrap(session.root, event.xmaprequest.window);
			if(xIsNormal(targetedWindow))
			{
				// Listen for mouse entering the new window (in desktop.h)
				XSelectInput(session.desktop.ui.window.display, targetedWindow.id, EnterWindowMask);
				
				// New top-level application windows are placed at the end of the workspace
				if(wmLength(session) < 4)
					wmLength(session) += 1;
				else
					XUnmapWindow(session.root.display, wmWorkspace(session)[3].id);

				wmWorkspace(session)[wmLength(session) - 1]
					= targetedWindow;
				
				XSetWindowBorderWidth(session.root.display, targetedWindow.id, 0);
				session = wmLayoutWorkspace(session);
			}
			XMapWindow(session.root.display, targetedWindow.id);
			break;
		
		case ConfigureRequest:
			printf("wmGlobalEvent ConfigureRequest %li\n", event.xconfigurerequest.window);
			
			// Sign-off on all changes to dialogs, popups, etc.
			XWindowChanges changes;
			changes.x = event.xconfigurerequest.x;
			changes.y = event.xconfigurerequest.y;
			changes.width = event.xconfigurerequest.width;
			changes.height = event.xconfigurerequest.height;
			changes.border_width = event.xconfigurerequest.border_width;
			changes.sibling = event.xconfigurerequest.above;
			changes.stack_mode = event.xconfigurerequest.detail;
			XConfigureWindow(session.root.display, event.xconfigurerequest.window,
				event.xconfigurerequest.value_mask, &changes);
			
			// Fix layout for top-level application windows
			targetedWindow = xWrap(session.root, event.xconfigurerequest.window);			
			if(xIsNormal(targetedWindow))
			{
				session = wmLayoutWorkspace(session);
			}
			break;
		
		case UnmapNotify:
			printf("wmGlobalEvent UnmapNotify %lui\n", event.xunmap.window);
			
			session = wmRemoveByID(session, session.activeWorkspace, event.xunmap.window);
			session = wmLayoutWorkspace(session);
			break;
		
		case DestroyNotify:
			printf("wmGlobalEvent DestroyNotify\n");

			session = wmRemoveEverywhereByID(session, event.xdestroywindow.window);
			session = wmLayoutWorkspace(session);
			break;
		
		case KeyPress:
			switch(event.xkey.keycode)
			{
				// 0-9
				case 10: session = wmSwitchToWorkspace(session, 0); break;
				case 11: session = wmSwitchToWorkspace(session, 1); break;
				case 12: session = wmSwitchToWorkspace(session, 2); break;
				case 13: session = wmSwitchToWorkspace(session, 3); break;
				case 14: session = wmSwitchToWorkspace(session, 4); break;
				
				// Launchers
				case 27:	system("xfce4-appfinder --disable-server &"); break;	// R
				case 28:	system("x-terminal-emulator &"); break;	// T
				
				// Cmd-X
				// Cmd-C
				// Cmd-V
				// Cmd-D
				// Cmd-(Left|Right)
			}
			break;
	}
	
	// Go again
	return wmGlobalEvents(session);
}

// Keep track of BadWindows (errors for destroyed windows), to remove them from layouts later
struct wmRemovals
{
	void* sessionID;
	Window queue[40];
	int length;
};
struct wmRemovals wmRemovals = {0};

wmSession wmEvents(wmSession session)
{
	// Handle window manager and desktop events
	session = wmGlobalEvents(session);
	session = desktopEvents(session);
	
	// Remove failed windows
	for(int i = 0; i < wmRemovals.length; i++)
	{
		session = wmRemoveEverywhereByID(session, wmRemovals.queue[i]);
	}
	wmRemovals.length = 0;
	
	// Set up a funnel to watch for the next event
	short discardResult;
	struct pollfd watchList[2] =
	{
		{XConnectionNumber(session.root.display), POLLIN, discardResult},
		{XConnectionNumber(session.desktop.ui.window.display), POLLIN, discardResult},
	};
	poll(watchList, 2, -1);
	
	return session;
}

// Replacement X11 error handler - note BadWindows
int wmErrorHandler(Display* display, XErrorEvent* error) {
	printf("X11 error. Request %i, code %i.%i, resource %li\n",
		error->request_code, error->error_code, error->minor_code, error->resourceid);
	
	char errorText[100];
	XGetErrorText(display, error->error_code, errorText, 100);
	printf("Error code: %s\n", errorText);
	
	XGetErrorText(display, error->minor_code, errorText, 100);
	printf("Minor code: %s\n", errorText);
	
	if(error->error_code == BadWindow)
	{
		wmRemovals.queue[wmRemovals.length] = error->resourceid;
		wmRemovals.length += 1;
	}
	
	return error->error_code;
}

#endif
