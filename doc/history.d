Name
	scale2x - History For Scale2x

Scale2x v3.0 2013/05
	) Now in git repository.
	) Preliminary scale2k support in the 'scalerx' tool. Just select the algo revision 'k'.
	) Removed libpng.dll and zlib.dll files. They are not needed anymore.

Scale2x v2.5 2011/9
	) Fixed compilation with newer libpng.
	) New EPX and "Depixelizing Pixel Art" documents in contrib/

Scale2x v2.4 2009/7
	) Added MMX support for x86_64 [Per Øyvind Karlsen]

Scale2x v2.3 2005/11
	) Some additions in the contrib/ directory.

Scale2x v2.2 2005/03
	) Fixed a typo in the algorithm documentation of scale3x.
		Specifically the definition of the E3 point in
		the algorithm.txt file was wrong.

Scale2x v2.1 2004/12
	) Added the `scale2x3' and `scale2x4' effects.
	) Fixed the computation of the border pixels for scale3x.
	) Fixed the scalebit.c implementation to be C++
		compatible [Max Horn]

Scale2x v2.0 2004/02
	) Added a new revision of the `scale3x' effect. It now looks good as
		`scale2x'.
	) Added a new optimized C version of the `scale2x' effect.
	) Added a detailed algorithm specification in the `algorithm.txt' file.
	) Minor fixes at the scalebit.c file.

Scale2x v1.6 2003/07
	) Added an example of the fast implementation of the effects
		applied to a generic bitmap. There is also an
		example for the not obvious Scale4x effect.
	) Revised the command line tools. Now they support all the
		formats of .PNG images. Also images with 16 bits per
		channel.
	) Fixed a linking problem with some libpng installations.

Scale2x v1.5 2003/05
	) Added the scale3x and scale4x command line tools.

Scale2x v1.4 2003/01
	) Added in the contrib dir a VisualC MMX assembler implementation
		by VisualBoy from the VisualBoyAdvance emulator.

Scale2x v1.3 2002/12
	) Added the contrib dir with some example implementations.
		Java by Randy Power and C SDL by Pete Shinners.

Scale2x v1.2 2002/11
	) The new Scale2x site is now http://scale2x.sourceforge.net
	) The MMX implementation now correctly computes the pixel on the
		left and right border.
	) Renamed the -t option in -w.

Scale2x v1.1 2002/11
	) Added the -t option at the command line converter.

Scale2x v1.0 2002/11
	) First version of the library

