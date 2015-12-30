#include "Theme.h"
#include "UIToolkit/UI.h"

// Globals

char* panelLeftMenu = "*Applications* Windows	> *";
char* panelRightMenu = "* <	> *Exit* >";

bool quit = false;
char markup[100];


// Functions

char* updateMarkup(char*);


// Main

int main(int numArgs, const char** argList)
{	
	uiWindow panel = UI.get(0, 0, UI.fullWidth(), UI.barHeight);
	
	UI.redraw(panel, updateMarkup(markup));
	
	while(!quit)
	{
		uiEvent event = UI.wait(panel, markup);
		switch(event.type)
		{
			case ButtonPress:
				if(UI.at(event, 1, 1, 1)) { system("xfce4-appfinder &"); }
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
