Name
	scale2x - Scale2x

	Scale2x is real-time graphics effect able to increase
	the size of small bitmaps guessing the missing pixels
	without blurring the images.

	It was originally developed for the AdvanceMAME project
	in the year 2001 to improve the quality of old games running
	at low video resolutions.

	The specification of the algorithm and more details are at :

		+http://scale2x.sourceforge.net

	This package contains some implementations of the Scale2x
	effect in C and MMX Pentium assembler, and a command
	line tool to convert manually .PNG images.

Implementation
	The fast C and MMX implementations are in the file scale2x.h.
	To use these implementations in your program you need simply
	to include the file in a C source.

	In the file scale2x.c there is the reference implementation
	used by the command line tool.

Tool
	The command line tools "scale2x" and "scale3x" read a .PNG file
	and write another .PNG file with the Scale2x effect applied.
	The syntax of the program is :

		:scale2x [-w] FROM.png TO.png

	The option -w can be used to scale textures with a wraparound effect.

	To compile the command line tool you need the libpng library.

