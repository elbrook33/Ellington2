gcc $1.c $2	\
	NanoVG/build/libnanovg.a	\
	-I. -INanoVG/src	\
	-lm -lX11 -lXcomposite -lGL -lGLEW	\
	-Wreturn-type	\
	-o $1
