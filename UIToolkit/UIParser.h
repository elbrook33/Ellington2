/*
 * To do:
 * Split up in pieces.
 */

#ifndef UI_PARSER_H
#define UI_PARSER_H

#include "Theme.h"
#include "UIToolkit/UITypes.h"
#include "Misc/Helpers.h"

uiState uiParse(uiWindow ui, const char* markup, uiWordAction action, void* data)
{
	if(!markup || !*markup) { return uiNoStop; }
	
	// Set up drawing context
	float
		x = ui.canvas.left + themeMargin,
		y = ui.canvas.top + themeMargin;
	
	nvgFontSize(ui.canvas.nano, ui.canvas.fontSize);
	nvgFontFace(ui.canvas.nano, "normal");
	nvgFillColor(ui.canvas.nano, ui.canvas.fgColour);
	
	float spaceWidth =
		nvgTextBounds(ui.canvas.nano, 0, 0, "M M", NULL, NULL)
		- nvgTextBounds(ui.canvas.nano, 0, 0, "MM", NULL, NULL);
	
	// Traverse through pars, then tabs, then words
	char *pars = strdup(markup);
	char *par, *parProgress;
	
	int parIndex = 0;
	
	for(	par = strtok_r(pars, "\n", &parProgress);
		par != NULL;
		par = strtok_r(NULL, "\n", &parProgress))
	{
		parIndex += 1;
		
		char *tabs = par;
		char *tab, *tabProgress;
		
		// Tabs are effectively tables
		int tabIndex = 0;
		int tabCount = strcount(tabs, '\t') + 1;
		float tabWidth = (ui.canvas.right - ui.canvas.left - 2*themeMargin) / (float)tabCount;
		
		int tabAlignment;
		
		for(	tab = strtok_r(tabs, "\t", &tabProgress);
			tab != NULL;
			tab = strtok_r(NULL, "\t", &tabProgress))
		{
			x = (float)tabIndex * tabWidth + ui.canvas.left + themeMargin;
			tabIndex += 1;
			
			// Handle left, centre and right alignment
			switch(*tab)
			{
				case '>':
					tab += 1;
					int lastIndex = strlen(tab) - 1;					
					switch(tab[lastIndex])
					{
						case '>':
							tab[lastIndex] = '\0';
							tabAlignment = 1;
							nvgTextAlign(ui.canvas.nano, NVG_ALIGN_RIGHT|NVG_ALIGN_TOP);
							x += tabWidth;
							break;
						case '<':
							tab[lastIndex] = '\0';
							tabAlignment = 0;
							nvgTextAlign(ui.canvas.nano, NVG_ALIGN_CENTER|NVG_ALIGN_TOP);
							x += tabWidth / 2.0;
							break;
					}
					break;
				default:
					tabAlignment = -1;
					nvgTextAlign(ui.canvas.nano, NVG_ALIGN_LEFT|NVG_ALIGN_TOP);
			}
			
			// Words may need to eventually be traversed twice for centre and right alignments
			char *words = tab;
			char *word, *wordProgress;
			
			int wordIndex = 0;

			for(	word = strtok_r(words, " ", &wordProgress);
				word != NULL;
				word = strtok_r(NULL, " ", &wordProgress))
			{
				wordIndex += 1;
				
				// Handle bold and italics
				switch(*word)
				{
					case '\\':
						word += 1;
						break;
					case '_':
						nvgFontFace(ui.canvas.nano, "italic");
						word += 1;
						break;
					case '*':
						nvgFontFace(ui.canvas.nano, "bold");
						word += 1;
						break;
				}
				
				// Handle trailing stylings
				int lastIndex = strlen(word) - 1;
				char lastLetter = '\0';
				switch(word[lastIndex])
				{
					case '\\':
						word[lastIndex] = '\0';
						break;
					case '_':
					case '*':
						word[lastIndex] = '\0';
						lastLetter = '_';
						break;
				}
				
				// Send word to "action"
				float bounds[4];
				float textWidth = nvgTextBounds(ui.canvas.nano, x, y, word, NULL, bounds)
					+ spaceWidth;
				
				uiIndices indices = { parIndex, tabIndex, wordIndex };

				if(action(ui, uiText, word, x, y, boxXYXY(bounds), indices, data) == uiStop)
					{ free(pars); return uiStop; }
				
				// Advance text entry position
				switch(tabAlignment)
				{
					case -1: x += textWidth; break;
					case 1: x -= textWidth; break;
				}
				
				// Post-word processing
				switch(lastLetter)
				{
					case '_':
					case '*':
						nvgFontFace(ui.canvas.nano, "normal");
						break;
				}
			}
		}
	}
	free(pars);
	return uiNoStop;
}

#endif
