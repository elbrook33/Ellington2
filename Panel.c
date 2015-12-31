#include "UIToolkit/UI.h"
#include "Applets/WindowList.h"

// Globals

char* panelLeftMenu = "*Applications* Windows	> *";
char* panelRightMenu = "* <	> *Exit* >";

bool quit = false;


// Functions

char* updateMarkup(char*);
void openWindowList(uiWindow, uiEvent, char* markup);


// Main

int main(int numArgs, const char** argList)
{	
	uiWindow panel = UI.get(0, 0, UI.fullWidth(), UI.barHeight);
	
	char markup[100];
	panel = UI.draw(panel, updateMarkup(markup));
	
	while(!quit)
	{
		uiEvent event = UI.wait(panel);
		switch(event.type)
		{
			case ButtonPress:
				if(UI.button(event) != 1) { break; }
				if(UI.at(event, 1, 1, 1)) { system("xfce4-appfinder &"); }
				if(UI.at(event, 1, 1, 2)) { openWindowList(panel, event, markup); }
				if(UI.at(event, 1, 3, 1)) { quit = true; }
				break;
		}
	}
	
	return 0;
}


// Implementation

char* updateMarkup(char* storage)
{
	char date[20];
	memset(storage, 0, 100);
	
	strcat(storage, panelLeftMenu);
	strcat(storage, dateString(date, 20));
	strcat(storage, panelRightMenu);
	
	return storage;
}

void openWindowList(uiWindow panel, uiEvent event, char* markup)
{
	panelLeftMenu = "*Applications* |Windows|	> *";
	updateMarkup(markup);
	UI.redraw(panel);
	
	windowList(UI.popup(panel, event));
	
	panelLeftMenu = "*Applications* Windows	> *";
	updateMarkup(markup);
	UI.redraw(panel);
}
