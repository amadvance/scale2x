Name
	scalex - Scale a .PNG image using the fast implementation of the Scale effects

Synopsis
	:scalex [-k N] input.png output.png

Description
	Scale2x, Scale3x and Scale4x are real-time graphics effects
	able to increase the size of small bitmaps guessing the
	missing pixels without blurring the images.

	They were originally developed for the AdvanceMAME project
	in the year 2001 to improve the quality of old games running
	at low video resolutions.

	The specification of the algorithm and more details are at:

		+http://www.scale2x.it

	These command line tools read a .PNG file and write another
	image with the effects applied.

	The fast implementation of the Scale effects is used. It imposes
	some limitations of the type of the image usable. Specifically :

	* Only the pixel sizes of 1, 2 and 4 bytes are supported. If the
		image has another pixel size it's automatically converted.

Options
	-k N, --scale N
		Select the scale factor. Available values are 2, 2x3,
		2x4, 3, and 4.

Copyright
	This file is Copyright (C) 2003, 2004 Andrea Mazzoleni

