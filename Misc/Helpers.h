#ifndef HELPERS_H
#define HELPERS_H

#include "Theme.h"
#include <stdio.h>
#include <time.h>

#define min(a,b)	\
({	\
	typeof (a) _a = (a);	\
	typeof (b) _b = (b);	\
	_a < _b ? _a : _b;	\
})

#define max(a,b)	\
({	\
	typeof (a) _a = (a);	\
	typeof (b) _b = (b);	\
	_a > _b ? _a : _b;	\
})

int strcount(const char* text, char c)
{
	int count;
	for(count = 0; *text; text++)
		{ count += (*text == c); }
	return count;
}

char* dateString(char* storage, int storageSize)
{
	struct tm* tm;
	time_t t;
	
	t = time(NULL);
	tm = localtime(&t);
	
	strftime(storage, storageSize, themeDateFormat, tm);
	return storage;
}

#endif
