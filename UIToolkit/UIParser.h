/*
 * To do:
 * Separate specific rules from parser structure.
 */

#ifndef UI_PARSER_H
#define UI_PARSER_H

#include "Theme.h"
#include "UIToolkit/UITypes.h"
#include "Misc/Helpers.h"

uiState uiParsePars(uiWindow, uiWordAction, uiParseContext* data,
	float* x, float* y, const char* pars);
uiState uiParseTabs(uiWindow, uiWordAction, uiParseContext* data,
	float* x, float* y, int parIndex, char* tabs);
uiState uiParseWords(uiWindow, uiWordAction, uiParseContext* data,
	float* x, float* y, uiAlign align, int parIndex, int tabIndex, char* words);
uiState uiParseTag(uiWindow, uiWordAction, uiParseContext* data,
	float* x, float* y, uiAlign align, int parIndex, int tabIndex, char* word, char** wordProgress);

#define forEachToken(T, Delimiter)	\
	char *T, *T##Progress;	\
	int T##Index = 1;	\
	for(T = strtok_r(T##s, Delimiter, &T##Progress);	\
		T != NULL;	\
		T = strtok_r(NULL, Delimiter, &T##Progress), T##Index++)


// Set up

uiState uiParse(uiWindow ui, uiWordAction action, uiParseContext* data)
{
	if(!ui.canvas.markup || !*ui.canvas.markup) { return uiNoStop; }
	
	// Set up drawing context
	float
		x = ui.canvas.left + ui.canvas.margin,
		y = ui.canvas.top + ui.canvas.margin + ui.canvas.scrollY;
	
	nvgFontSize(ui.canvas.nano, ui.canvas.fontSize);
	nvgFontFace(ui.canvas.nano, "normal");
	nvgFillColor(ui.canvas.nano, ui.canvas.fgColour);
	
	ui.canvas.spaceWidth =
		nvgTextBounds(ui.canvas.nano, 0, 0, "M M", NULL, NULL)
		- nvgTextBounds(ui.canvas.nano, 0, 0, "MM", NULL, NULL);
	
	return uiParsePars(ui, action, data, &x, &y, ui.canvas.markup);
}


// Pars

uiState uiParsePars(uiWindow ui, uiWordAction action, uiParseContext* data,
	float* x, float* y, const char* markup)
{
	char *pars = strdup(markup);
	
	forEachToken(par, "\n")
	{
		if(
			uiParseTabs(ui, action, data, x, y, parIndex, par) == uiStop
		)
			{ free(pars); return uiStop; }
		
		*y += ui.canvas.lineHeight;
	}

	free(pars);
	return uiNoStop;
}


// Tab

uiState uiParseTabs(uiWindow ui, uiWordAction action, uiParseContext* data,
	float* x, float* y, int parIndex, char* tabs)
{
	int tabCount = strcount(tabs, '\t') + 1;
	float tabWidth = (ui.canvas.right - ui.canvas.left - 2*themeMargin) / (float)tabCount;
	
	forEachToken(tab, "\t")
	{
		*x = (float)(tabIndex - 1)*tabWidth + ui.canvas.left + themeMargin;
		int lastIndex = strlen(tab) - 1;
			
		// Left
		uiAlign align = uiLeft;
		nvgTextAlign(ui.canvas.nano, NVG_ALIGN_LEFT|NVG_ALIGN_TOP);
		
		// Centre
		if(tab[0] == '>' && tab[lastIndex] == '<')
		{
			align = uiCentre;
			*x += tabWidth / 2.0;
			nvgTextAlign(ui.canvas.nano, NVG_ALIGN_CENTER|NVG_ALIGN_TOP);
			tab[lastIndex] = '\0';
			tab++;
		}
		
		// Right
		if(tab[0] == '>' && tab[lastIndex] == '>')
		{
			align = uiRight;
			*x += tabWidth;
			nvgTextAlign(ui.canvas.nano, NVG_ALIGN_RIGHT|NVG_ALIGN_TOP);
			tab[lastIndex] = '\0';
			tab++;
		}
		
		if(
			uiParseWords(ui, action, data, x, y, align, parIndex, tabIndex, tab) == uiStop
		)
			{ return uiStop; }
	}
	return uiNoStop;
}


// Word

uiState uiParseWords(uiWindow ui, uiWordAction action, uiParseContext* data,
	float* x, float* y, uiAlign align, int parIndex, int tabIndex, char* words)
{
	uiItemType itemType = uiText;
	
	forEachToken(word, " ")
	{
		// Handle bold and italics
		int lastIndex = strlen(word) - 1;
		
		while(*word && strchr("\\_*<|", *word))
		{
			switch(*word)
			{
				case '\\':
					break;
				case '_':
					nvgFontFace(ui.canvas.nano, "italic");
					break;
				case '*':
					nvgFontFace(ui.canvas.nano, "bold");
					break;
				case '<':
					uiParseTag(ui, action, data, x, y, align, parIndex, tabIndex, word, &wordProgress);
					continue;
				case '|':
					itemType = uiHighlighted;
					break;
			}
			word += 1;
			lastIndex -= 1;
		}
		
		// Handle trailing stylings
		char lastLetter = '\0';
		
		while(lastIndex > 0 && strchr("\\_*<|", word[lastIndex]))
		{
			switch(word[lastIndex])
			{
				case '\\':
					break;
				case '_':
				case '*':
					lastLetter = '_';
					break;
				case '|':
					lastLetter = '|';
					break;
			}
			word[lastIndex] = '\0';
			lastIndex -= 1;
		}
		
		// Send word to "action"
		float bounds[4];
		float textWidth = nvgTextBounds(ui.canvas.nano, *x, *y, word, NULL, bounds) + ui.canvas.spaceWidth;
		uiIndices indices = { parIndex, tabIndex, wordIndex };
		
		bounds[0] -= ui.canvas.spaceWidth/4.0;
		bounds[1] = *y - ui.canvas.margin;
		bounds[2] += ui.canvas.spaceWidth/4.0;
		bounds[3] = *y + ui.canvas.lineHeight;
		
		if(action &&
			action(ui, itemType, word, *x, *y, boxXYXY(bounds), indices, data) == uiStop
		)
			{ return uiStop; }
		
		// Advance text entry position
		if(*word != '\0')
		{
			switch(align)
			{
				case uiLeft: *x += textWidth; break;
				case uiRight: *x -= textWidth; break;
			}
		}
		
		// Post-word processing
		switch(lastLetter)
		{
			case '_':
			case '*':
				nvgFontFace(ui.canvas.nano, "normal");
				break;
			case '|':
				itemType = uiText;
				break;
		}
	}
	
	return uiNoStop;
}


// Tag

uiState uiParseTag(uiWindow ui, uiWordAction action, uiParseContext* data,
	float* x, float* y, uiAlign align, int parIndex, int tabIndex, char* word, char** wordProgress)
{
	while(word = strtok_r(NULL, " ", wordProgress))
	{
		int lastIndex = strlen(word) - 1;
		if(word[lastIndex] == '>')
			{ return uiStop; }
	}
	return uiNoStop;
}

#endif
