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
 * 		2. Show pop-up menus.
 * 		3. Draw status tray.
 * 			→ UIToolkit (for drawing)
 * 			→ WindowManager (for window lists)
 * 
 * To do
 * -----
 * - Panel applets: applications menu, windows list/workspace switcher, clock, status tray, log out.
 * - Full-screen.
 * - Split up parser. Handle tags.
 * - Split up event handlers.
 * - Extra workspace actions (e.g. copy and paste).
 * - Problem cases: Firefox tooltips, xfce4-taskmanager...
 */

#include "WindowManager/WindowManager.h"
#include "Applets/Panel.h"
#include "poll.h"

int main(int numArgs, const char** argList)
{
	// Parse command-line arguments
	const char* displayID = numArgs > 1? argList[1] : ":0";
	
	// Start up window manager and panel
	wmSession session = wmGet(displayID);
	uiWindow panel = panelGet(session.root);
	
	// Set up a funnel to watch for the next event
	short watchResult;
	struct pollfd watchList[3] =
	{
		{XConnectionNumber(session.root.display), POLLIN, watchResult},
		{XConnectionNumber(session.desktop.ui.window.display), POLLIN, watchResult},
		{XConnectionNumber(panel.window.display), POLLIN, watchResult}
	};
	
	// Loop
	while(true)
	{
		session = wmEvents(session);
		panel = panelEvents(panel, session);

		poll(watchList, 3, -1);
	}
	
	return 0;
}
