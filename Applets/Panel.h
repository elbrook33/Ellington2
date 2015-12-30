#ifndef PANEL_H
#define PANEL_H

#include "Theme.h"
#include "UIToolkit/UIToolkit.h"

char* panelLeftMenu = "*Applications* Windows	> *";
char* panelRightMenu = "* <	> *Exit* >";

uiWindow panelGet(xWindow root)
{
	uiWindow panel = uiGet(root, boxXYWH(0, 0, root.attributes.width, themeBarHeight));
	
	panel = uiSetColours(panel, themePanelFg, themePanelBg, NULL);
	
	XSelectInput(panel.window.display, panel.window.id, ButtonPressMask);
	XMapWindow(panel.window.display, panel.window.id);
	
	return panel;
}

char* panelMarkup(char* storage)
{
	char panelDate[20];

	strcat(storage, panelLeftMenu);
	strcat(storage, dateString(panelDate, 20));
	strcat(storage, panelRightMenu);

	return storage;
}

void panelRedraw(uiWindow panel)
{
	char markup[100] = {0};
	
	// Draw
	uiBeginFrame(panel);
	uiDraw(panel, panelMarkup(markup));
	uiEndFrame(panel);
}

uiWindow panelEvents(uiWindow panel, wmSession session)
{
	if(XPending(panel.window.display) == 0)
	{
		panelRedraw(panel);
		return panel;
	}
	
	XEvent event;
	XNextEvent(panel.window.display, &event);
	
	char markup[100] = {0};
	char dataStorage[max(2*sizeof(float), 3*sizeof(int))];
	int par, tab, word;
	
	switch(event.type)
	{
		case ButtonPress:
			if(uiIndexAt(panel, panelMarkup(markup), event.xbutton.x, event.xbutton.y, dataStorage))
			{
				par = ((int*)dataStorage)[0];
				tab = ((int*)dataStorage)[1];
				word = ((int*)dataStorage)[2];
				
				if(par == 1 && tab == 1 && word == 1) { system("xfce4-appfinder &"); }
				if(par == 1 && tab == 3 && word == 1) { globalQuit = true; }
			}
			break;
	}
	
	return panel;
}

#endif
