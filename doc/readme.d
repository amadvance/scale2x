Name
	scale2x - Scale2x, Scale3x, Scale4x

	Scale2x, Scale3x and Scale4x are real-time graphics effects
	able to increase the size of small bitmaps guessing the
	missing pixels without blurring the images.

	They were originally developed for the AdvanceMAME project
	in the year 2001 to improve the quality of old games running
	at low video resolutions.

	The specification of the algorithm and more details are at :

		+http://scale2x.sourceforge.net

	This package contains some implementations of the effects
	in C and MMX Pentium assembler, and a command line tool to
	convert manually .PNG images.

Implementation
	The files scale2x.h and scale3x.h are fast C and MMX
	implementations of the effects.

	The file scale2x.c, scale3x.c and scale4x.c are simple command
	line processors of PNG files. They use reference implementations
	of the effects.

Tools
	The command line tools "scale2x", "scale3x" and "scale4x" read
	a .PNG file and write another .PNG file with the effect applied.
	The syntax of the programs is :

		:scale2x [-w] FROM.png TO.png

	The option -w can be used to scale textures with a wraparound effect.

	To compile the command line tool you need the libz and libpng
	libraries.

