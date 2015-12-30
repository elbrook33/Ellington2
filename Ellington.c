/*
 * Ellington Desktop
 * =================
 * 
 * Outline
 * -------
 * Main
 * 	→ WindowManager
 * 		1. Track windows opening and closing.
 * 		2. Lay out windows.
 * 		3. Listen for hot-keys
 * 		4. Switch desktops.
 * 			→ X11Interface (for tracking window attributes)
 * 			→ Xlib (for resizing)
 * 
 * 	→ Desktop
 * 		1. Draw background, titlebars and shadow.
 * 		2. Handle drag events.
 * 			→ UIToolkit (for drawing)
 * 			→ Xlib (for moving)
 * 
 * 	→ Panel
 * 		1. Draw panel with basic information, such as time.
 * 		2. Show pop-up menus (applets).
 * 		3. Draw status tray.
 * 			→ UIToolkit (for drawing)
 * 			→ WindowManager (for window lists)
 * 
 * To do
 * -----
 * - Panel applets: applications menu, windows list/workspace switcher, clock, status tray, log out.
 * 	Comms between panel and window manager:
 * 		→ No: Pointers. It'd be a shame to start now.
 * 		→ No: Messages. Via X11? A queue? Cumbersome.
 * 		→ No: Make panel like desktop: a child of window manager, and always with a copy of parent.
 * 		→ Yes: One-way. Window manager doesn't need panel enough. Just pass window manager to panel.
 * - Full-screen.
 * - Split up parser. Handle tags.
 * - Split up event handlers.
 * - Extra workspace actions (e.g. copy and paste).
 * - Problem cases: Firefox tooltips, xfce4-taskmanager...
 */

#include <poll.h>
#include <stdbool.h>

bool globalQuit = false;

#include "WindowManager/WindowManager.h"
#include "Applets/Panel.h"

int main(int numArgs, const char** argList)
{
	// Parse command-line arguments
	const char* displayID = numArgs > 1? argList[1] : ":0";
	
	// Start up window manager and panel
	wmSession session = wmGet(displayID);
	uiWindow panel = panelGet(session.root);
	
	// Set up a funnel to watch for the next event
	short discardResult;
	struct pollfd watchList[3] =
	{
		{XConnectionNumber(session.root.display), POLLIN, discardResult},
		{XConnectionNumber(session.desktop.ui.window.display), POLLIN, discardResult},
		{XConnectionNumber(panel.window.display), POLLIN, discardResult}
	};
	
	// Loop
	while(true)
	{
		session = wmEvents(session);
		panel = panelEvents(panel, session);
		
		if(globalQuit) { break; }

		poll(watchList, 3, -1);
	}
	
	return 0;
}
