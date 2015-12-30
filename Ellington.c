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
 * - Split up parser. Handle tags, especially images.
 * - Panel applets: applications menu, windows list/workspace switcher, clock, status tray, log out.
 * 	Comms between panel and window manager:
 * 		→ No: Pointers. It'd be a shame to start now.
 * 		→ No: Messages. Via X11? A queue? Cumbersome.
 * 		→ No: Make panel like desktop: a child of window manager, and always with a copy of parent.
 * 		→ No: One-way. Window manager doesn't need panel enough. Just pass window manager to panel.
 * 		→ Yes: None. Just grab what you need globally.
 * - Full-screen.
 * - Extra workspace actions (e.g. copy and paste).
 * - Split up event handlers.
 * - Problem cases: Firefox tooltips, xfce4-taskmanager...
 */

#include <poll.h>
#include <stdbool.h>

bool globalQuit = false;

#include "WindowManager/WindowManager.h"

int main(int numArgs, const char** argList)
{
	// Parse command-line arguments
	const char* displayID = numArgs > 1? argList[1] : ":0";
	
	// Start up window manager
	wmSession session = wmGet(displayID);
	while(!globalQuit)
	{
		session = wmEvents(session);
	}
	
	return 0;
}
