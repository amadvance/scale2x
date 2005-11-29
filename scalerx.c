/*
 * This file is part of the Scale2x project.
 *
 * Copyright (C) 2003 Andrea Mazzoleni
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details. 
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*
 * This is the source of a simple command line tool which uses the reference
 * implementation of the Scale effects.
 *
 * You can find an high level description of the effects at :
 *
 * http://scale2x.sourceforge.net/
 */

#include "file.h"
#include "pixel.h"
#include "portable.h"

#include <zlib.h>

#include <stdlib.h>
#include <stdio.h>

void scalex(unsigned char* dst_ptr, unsigned dst_slice, const unsigned char* src_ptr, unsigned src_slice, unsigned pixel, unsigned width, unsigned height, unsigned mx, unsigned my)
{
	int x;
	int y;

	for(y=0;y<height;++y) {
		for(x=0;x<width;++x) {
			pixel_t E;
			unsigned i,j;

			E = pixel_get(x, y, src_ptr, src_slice, pixel, width, height, 0);

			for(i=0;i<mx;++i)
				for(j=0;j<my;++j)
					pixel_put(x*mx+i, y*my+j, dst_ptr, dst_slice, pixel, width*mx, height*my, E);
		}
	}
}

#define SCALE2X_REVISION_MAX 3

void scale2x(unsigned char* dst_ptr, unsigned dst_slice, const unsigned char* src_ptr, unsigned src_slice, unsigned pixel, unsigned width, unsigned height, int opt_tes, int opt_ver)
{
	int x;
	int y;

	for(y=0;y<height;++y) {
		for(x=0;x<width;++x) {
			pixel_t E0, E1, E2, E3;
			pixel_t A, B, C, D, E, F, G, H, I;

			A = pixel_get(x-1, y-1, src_ptr, src_slice, pixel, width, height, opt_tes);
			B = pixel_get(x, y-1, src_ptr, src_slice, pixel, width, height, opt_tes);
			C = pixel_get(x+1, y-1, src_ptr, src_slice, pixel, width, height, opt_tes);
			D = pixel_get(x-1, y, src_ptr, src_slice, pixel, width, height, opt_tes);
			E = pixel_get(x, y, src_ptr, src_slice, pixel, width, height, opt_tes);
			F = pixel_get(x+1, y, src_ptr, src_slice, pixel, width, height, opt_tes);
			G = pixel_get(x-1, y+1, src_ptr, src_slice, pixel, width, height, opt_tes);
			H = pixel_get(x, y+1, src_ptr, src_slice, pixel, width, height, opt_tes);
			I = pixel_get(x+1, y+1, src_ptr, src_slice, pixel, width, height, opt_tes);

			/*
				ABC
				DEF
				GHI

				E0E1
				E2E3
			*/
			switch (opt_ver) {
			default:
			case 0 :
				/* version 0, normal scaling */
				E0 = E;
				E1 = E;
				E2 = E;
				E3 = E;
				break;
			case 1 :
				/* default */
				E0 = D == B && B != F && D != H ? D : E;
				E1 = B == F && B != D && F != H ? F : E;
				E2 = D == H && D != B && H != F ? D : E;
				E3 = H == F && D != H && B != F ? F : E;
				break;
			case 2 :
				/* rejected */
				E0 = D == B && ((B != F && D != H) || D == A) ? D : E;
				E1 = B == F && ((B != D && F != H) || B == C) ? F : E;
				E2 = D == H && ((D != B && H != F) || D == G) ? D : E;
				E3 = H == F && ((D != H && B != F) || H == I) ? F : E;
				break;
			case 3 :
				/* rejected, loses isolated pixels */
				E0 = D == B && A != E ? D : E;
				E1 = B == F && C != E ? F : E;
				E2 = D == H && G != E ? D : E;
				E3 = H == F && I != E ? F : E;
				break;
			}

			pixel_put(x*2, y*2, dst_ptr, dst_slice, pixel, width*2, height*2, E0);
			pixel_put(x*2+1, y*2, dst_ptr, dst_slice, pixel, width*2, height*2, E1);
			pixel_put(x*2, y*2+1, dst_ptr, dst_slice, pixel, width*2, height*2, E2);
			pixel_put(x*2+1, y*2+1, dst_ptr, dst_slice, pixel, width*2, height*2, E3);
		}
	}
}

#define SCALE3X_REVISION_MAX 7

void scale3x(unsigned char* dst_ptr, unsigned dst_slice, const unsigned char* src_ptr, unsigned src_slice, unsigned pixel, unsigned width, unsigned height, int opt_tes, int opt_ver)
{
	int x;
	int y;

	for(y=0;y<height;++y) {
		for(x=0;x<width;++x) {
			pixel_t E0, E1, E2, E3, E4, E5, E6, E7, E8;
			pixel_t A, B, C, D, E, F, G, H, I;
			int k0, k1, k2, k3;
			int c0, c1, c2, c3;

			A = pixel_get(x-1, y-1, src_ptr, src_slice, pixel, width, height, opt_tes);
			B = pixel_get(x, y-1, src_ptr, src_slice, pixel, width, height, opt_tes);
			C = pixel_get(x+1, y-1, src_ptr, src_slice, pixel, width, height, opt_tes);
			D = pixel_get(x-1, y, src_ptr, src_slice, pixel, width, height, opt_tes);
			E = pixel_get(x, y, src_ptr, src_slice, pixel, width, height, opt_tes);
			F = pixel_get(x+1, y, src_ptr, src_slice, pixel, width, height, opt_tes);
			G = pixel_get(x-1, y+1, src_ptr, src_slice, pixel, width, height, opt_tes);
			H = pixel_get(x, y+1, src_ptr, src_slice, pixel, width, height, opt_tes);
			I = pixel_get(x+1, y+1, src_ptr, src_slice, pixel, width, height, opt_tes);

			/*
				ABC
				DEF
				GHI

				k0k1
				k2k3

				E0E1E2
				E3E4E5
				E6E7E8
			*/
			switch (opt_ver) {
			default:
			case 0 :
				/* version 0, normal scaling */
				E0 = E;
				E1 = E;
				E2 = E;
				E3 = E;
				E4 = E;
				E5 = E;
				E6 = E;
				E7 = E;
				E8 = E;
				break;
			case 1 :
				/* second version, default */
				k0 = D == B && B != F && D != H;
				k1 = B == F && B != D && F != H;
				k2 = D == H && D != B && H != F;
				k3 = H == F && D != H && B != F;
				E0 = k0 ? D : E;
				E1 = (k0 && E != C) || (k1 && E != A) ? B : E;
				E2 = k1 ? F : E;
				E3 = (k0 && E != G) || (k2 && E != A) ? D : E;
				E4 = E;
				E5 = (k1 && E != I) || (k3 && E != C) ? F : E;
				E6 = k2 ? D : E;
				E7 = (k2 && E != I) || (k3 && E != G) ? H : E;
				E8 = k3 ? F : E;
				break;
			case 2 :
				/* first version */
				E0 = D == B && B != F && D != H ? D : E;
				E1 = E;
				E2 = B == F && B != D && F != H ? F : E;
				E3 = E;
				E4 = E;
				E5 = E;
				E6 = D == H && D != B && H != F ? D : E;
				E7 = E;
				E8 = H == F && D != H && B != F ? F : E;
				break;
			case 3 :
				/* rejected */
				E0 = D == B && ((B != F && D != H) || D == A) ? D : E;
				E1 = E;
				E2 = B == F && ((B != D && F != H) || B == C) ? F : E;
				E3 = E;
				E4 = E;
				E5 = E;
				E6 = D == H && ((D != B && H != F) || D == G) ? D : E;
				E7 = E;
				E8 = H == F && ((D != H && B != F) || H == I) ? F : E;
				break;
			case 4 :
				/* rejected */
				E0 = D == B && A != E ? D : E;
				E1 = E;
				E2 = B == F && C != E ? F : E;
				E3 = E;
				E4 = E;
				E5 = E;
				E6 = D == H && G != E ? D : E;
				E7 = E;
				E8 = H == F && I != E ? F : E;
				break;
			case 5 :
				/* rejected */
				k0 = D == B && B != F && D != H;
				k1 = B == F && B != D && F != H;
				k2 = D == H && D != B && H != F;
				k3 = H == F && D != H && B != F;
				c0 = k0 && E != C;
				c1 = k1 && E != B;
				c2 = k2 && E != H;
				c3 = k3 && E != H;
				E0 = k0 ? D : E;
				E1 = c0 || c1 ? B : E;
				E2 = k1 ? F : E;
				E3 = c0 || c2 ? D : E;
				E4 = E;
				E5 = c1 || c3 ? F : E;
				E6 = k2 ? D : E;
				E7 = c2 || c3 ? H : E;
				E8 = k3 ? F : E;
				break;
			case 6 :
				/* rejected */
				k0 = D == B && B != F && D != H;
				k1 = B == F && B != D && F != H;
				k2 = D == H && D != B && H != F;
				k3 = H == F && D != H && B != F;
				E0 = k0 ? D : E;
				E1 = k0 != k1 ? B : E;
				E2 = k1 ? F : E;
				E3 = k0 != k2 ? D : E;
				E4 = E;
				E5 = k1 != k3 ? F : E;
				E6 = k2 ? D : E;
				E7 = k2 != k3 ? H : E;
				E8 = k3 ? F : E;
				break;
			case 7 :
				/* rejected */
				k0 = D == B && B != F && D != H;
				k1 = B == F && B != D && F != H;
				k2 = D == H && D != B && H != F;
				k3 = H == F && D != H && B != F;
				E0 = k0 ? D : E;
				E1 = k0 || k1 ? B : E;
				E2 = k1 ? F : E;
				E3 = k0 || k2 ? D : E;
				E4 = E;
				E5 = k1 || k3 ? F : E;
				E6 = k2 ? D : E;
				E7 = k2 || k3 ? H : E;
				E8 = k3 ? F : E;
				break;
			}

			pixel_put(x*3, y*3, dst_ptr, dst_slice, pixel, width*3, height*3, E0);
			pixel_put(x*3+1, y*3, dst_ptr, dst_slice, pixel, width*3, height*3, E1);
			pixel_put(x*3+2, y*3, dst_ptr, dst_slice, pixel, width*3, height*3, E2);
			pixel_put(x*3, y*3+1, dst_ptr, dst_slice, pixel, width*3, height*3, E3);
			pixel_put(x*3+1, y*3+1, dst_ptr, dst_slice, pixel, width*3, height*3, E4);
			pixel_put(x*3+2, y*3+1, dst_ptr, dst_slice, pixel, width*3, height*3, E5);
			pixel_put(x*3, y*3+2, dst_ptr, dst_slice, pixel, width*3, height*3, E6);
			pixel_put(x*3+1, y*3+2, dst_ptr, dst_slice, pixel, width*3, height*3, E7);
			pixel_put(x*3+2, y*3+2, dst_ptr, dst_slice, pixel, width*3, height*3, E8);
		}
	}
}

#define SCALE4X_REVISION_MAX SCALE2X_REVISION_MAX

int scale4x(unsigned char* dst_ptr, unsigned dst_slice, const unsigned char* src_ptr, unsigned src_slice, unsigned pixel, unsigned width, unsigned height, int opt_tes, int opt_ver)
{
	unsigned char* mid_ptr;
	unsigned mid_slice;

	mid_slice = width * pixel * 2;
	mid_ptr = malloc(mid_slice * height * 2);
	if (!mid_ptr) {
		fprintf(stderr, "Low memory.\n");
		return -1;
	}

	scale2x(mid_ptr, mid_slice, src_ptr, src_slice, pixel, width, height, opt_tes, opt_ver);
	scale2x(dst_ptr, dst_slice, mid_ptr, mid_slice, pixel, width * 2, height * 2, opt_tes, opt_ver);

	free(mid_ptr);

	return 0;
}

#define SCALE2X3_REVISION_MAX 1

void scale2x3(unsigned char* dst_ptr, unsigned dst_slice, const unsigned char* src_ptr, unsigned src_slice, unsigned pixel, unsigned width, unsigned height, int opt_tes, int opt_ver)
{
	int x;
	int y;

	for(y=0;y<height;++y) {
		for(x=0;x<width;++x) {
			pixel_t E0, E1, E2, E3, E4, E5;
			pixel_t A, B, C, D, E, F, G, H, I;
			int k0, k1, k2, k3;

			A = pixel_get(x-1, y-1, src_ptr, src_slice, pixel, width, height, opt_tes);
			B = pixel_get(x, y-1, src_ptr, src_slice, pixel, width, height, opt_tes);
			C = pixel_get(x+1, y-1, src_ptr, src_slice, pixel, width, height, opt_tes);
			D = pixel_get(x-1, y, src_ptr, src_slice, pixel, width, height, opt_tes);
			E = pixel_get(x, y, src_ptr, src_slice, pixel, width, height, opt_tes);
			F = pixel_get(x+1, y, src_ptr, src_slice, pixel, width, height, opt_tes);
			G = pixel_get(x-1, y+1, src_ptr, src_slice, pixel, width, height, opt_tes);
			H = pixel_get(x, y+1, src_ptr, src_slice, pixel, width, height, opt_tes);
			I = pixel_get(x+1, y+1, src_ptr, src_slice, pixel, width, height, opt_tes);

			/*
				ABC
				DEF
				GHI

				E0E1
				E2E3
				E4E5
			*/
			switch (opt_ver) {
			default:
			case 0 :
				/* version 0, normal scaling */
				E0 = E;
				E1 = E;
				E2 = E;
				E3 = E;
				E4 = E;
				E5 = E;
				break;
			case 1 :
				/* default */
				k0 = D == B && B != F && D != H;
				k1 = B == F && B != D && F != H;
				k2 = D == H && D != B && H != F;
				k3 = H == F && D != H && B != F;
				E0 = k0 ? D : E;
				E1 = k1 ? F : E;
				E2 = (k0 && E != G) || (k2 && E != A) ? D : E;
				E3 = (k1 && E != I) || (k3 && E != C) ? F : E;
				E4 = k2 ? D : E;
				E5 = k3 ? F : E;
				break;
			}

			pixel_put(x*2, y*3, dst_ptr, dst_slice, pixel, width*2, height*3, E0);
			pixel_put(x*2+1, y*3, dst_ptr, dst_slice, pixel, width*2, height*3, E1);
			pixel_put(x*2, y*3+1, dst_ptr, dst_slice, pixel, width*2, height*3, E2);
			pixel_put(x*2+1, y*3+1, dst_ptr, dst_slice, pixel, width*2, height*3, E3);
			pixel_put(x*2, y*3+2, dst_ptr, dst_slice, pixel, width*2, height*3, E4);
			pixel_put(x*2+1, y*3+2, dst_ptr, dst_slice, pixel, width*2, height*3, E5);
		}
	}
}

#define SCALE2X4_REVISION_MAX 2

void scale2x4(unsigned char* dst_ptr, unsigned dst_slice, const unsigned char* src_ptr, unsigned src_slice, unsigned pixel, unsigned width, unsigned height, int opt_tes, int opt_ver)
{
	int x;
	int y;

	for(y=0;y<height;++y) {
		for(x=0;x<width;++x) {
			pixel_t E0, E1, E2, E3, E4, E5, E6, E7;
			pixel_t A, B, C, D, E, F, G, H, I;
			int k0, k1, k2, k3;

			A = pixel_get(x-1, y-1, src_ptr, src_slice, pixel, width, height, opt_tes);
			B = pixel_get(x, y-1, src_ptr, src_slice, pixel, width, height, opt_tes);
			C = pixel_get(x+1, y-1, src_ptr, src_slice, pixel, width, height, opt_tes);
			D = pixel_get(x-1, y, src_ptr, src_slice, pixel, width, height, opt_tes);
			E = pixel_get(x, y, src_ptr, src_slice, pixel, width, height, opt_tes);
			F = pixel_get(x+1, y, src_ptr, src_slice, pixel, width, height, opt_tes);
			G = pixel_get(x-1, y+1, src_ptr, src_slice, pixel, width, height, opt_tes);
			H = pixel_get(x, y+1, src_ptr, src_slice, pixel, width, height, opt_tes);
			I = pixel_get(x+1, y+1, src_ptr, src_slice, pixel, width, height, opt_tes);

			/*
				ABC
				DEF
				GHI

				E0E1
				E2E3
				E4E5
				E6E7
			*/
			switch (opt_ver) {
			default:
			case 0 :
				/* version 0, normal scaling */
				E0 = E;
				E1 = E;
				E2 = E;
				E3 = E;
				E4 = E;
				E5 = E;
				E6 = E;
				E7 = E;
				break;
			case 1 :
				/* default */
				k0 = D == B && B != F && D != H;
				k1 = B == F && B != D && F != H;
				k2 = D == H && D != B && H != F;
				k3 = H == F && D != H && B != F;
				E0 = k0 ? D : E;
				E1 = k1 ? F : E;
				E2 = (k0 && E != G) || (k2 && E != A) ? D : E;
				E3 = (k1 && E != I) || (k3 && E != C) ? F : E;
				E4 = (k0 && E != G) || (k2 && E != A) ? D : E;
				E5 = (k1 && E != I) || (k3 && E != C) ? F : E;
				E6 = k2 ? D : E;
				E7 = k3 ? F : E;
				break;
			case 2 :
				/* Hans de Goede variation, rejected */
				k0 = D == B && B != F && D != H;
				k1 = B == F && B != D && F != H;
				k2 = D == H && D != B && H != F;
				k3 = H == F && D != H && B != F;
				E0 = k0 ? D : E;
				E1 = k1 ? F : E;
				E2 = k0 ? D : E;
				E3 = k1 ? F : E;
				E4 = k2 ? D : E;
				E5 = k3 ? F : E;
				E6 = k2 ? D : E;
				E7 = k3 ? F : E;
				break;
			}

			pixel_put(x*2, y*4, dst_ptr, dst_slice, pixel, width*2, height*4, E0);
			pixel_put(x*2+1, y*4, dst_ptr, dst_slice, pixel, width*2, height*4, E1);
			pixel_put(x*2, y*4+1, dst_ptr, dst_slice, pixel, width*2, height*4, E2);
			pixel_put(x*2+1, y*4+1, dst_ptr, dst_slice, pixel, width*2, height*4, E3);
			pixel_put(x*2, y*4+2, dst_ptr, dst_slice, pixel, width*2, height*4, E4);
			pixel_put(x*2+1, y*4+2, dst_ptr, dst_slice, pixel, width*2, height*4, E5);
			pixel_put(x*2, y*4+3, dst_ptr, dst_slice, pixel, width*2, height*4, E6);
			pixel_put(x*2+1, y*4+3, dst_ptr, dst_slice, pixel, width*2, height*4, E7);
		}
	}
}

int file_process(const char* file0, const char* file1, int opt_scale_x, int opt_scale_y, int opt_tes, int opt_ver, int opt_crc, int opt_only124)
{
	unsigned pixel;
	unsigned width;
	unsigned height;
	unsigned char* src_ptr;
	unsigned src_slice;
	unsigned char* dst_ptr;
	unsigned dst_slice;
	int type;
	int channel;
	png_color* palette;
	unsigned palette_size;

	if (file_read(file0, &src_ptr, &src_slice, &pixel, &width, &height, &type, &channel, &palette, &palette_size, opt_only124 ? 1 : 0) != 0) {
		goto err;
	}

	dst_slice = width * pixel * opt_scale_x;
	dst_ptr = malloc(dst_slice * height * opt_scale_y);
	if (!dst_ptr) {
		fprintf(stderr, "Low memory.\n");
		goto err_src;
	}

	switch (opt_scale_x * 100 + opt_scale_y) {
	case 202 :
		scale2x(dst_ptr, dst_slice, src_ptr, src_slice, pixel, width, height, opt_tes, opt_ver);
		break;
	case 203 :
		scale2x3(dst_ptr, dst_slice, src_ptr, src_slice, pixel, width, height, opt_tes, opt_ver);
		break;
	case 204 :
		scale2x4(dst_ptr, dst_slice, src_ptr, src_slice, pixel, width, height, opt_tes, opt_ver);
		break;
	case 303 :
		scale3x(dst_ptr, dst_slice, src_ptr, src_slice, pixel, width, height, opt_tes, opt_ver);
		break;
	case 404 :
		if (scale4x(dst_ptr, dst_slice, src_ptr, src_slice, pixel, width, height, opt_tes, opt_ver) != 0)
			goto err_dst;
		break;
	default:
		scalex(dst_ptr, dst_slice, src_ptr, src_slice, pixel, width, height, opt_scale_x, opt_scale_y);
		break;
	}

	if (file_write(file1, dst_ptr, dst_slice, pixel, width * opt_scale_x, height * opt_scale_y, type, channel, palette, palette_size) != 0) {
		goto err_dst;
	}

	if (opt_crc) {
		unsigned crc = crc32(0, dst_ptr, dst_slice * height * opt_scale_y);
		printf("%08x\n", crc);
	}

	free(dst_ptr);
	free(src_ptr);
	free(palette);

	return 0;

err_dst:
	free(dst_ptr);
err_src:
	free(src_ptr);
	free(palette);
err:
	return -1;
}

void version(void) {
	printf(PACKAGE " v" VERSION " by Andrea Mazzoleni\n");
}

void usage(void) {
	version();
	printf("Reference implementation of the Scale2/3/4x effect\n");
	printf("\nSyntax: scalerx [-k N] [-w] [-r N] FROM.png TO.png\n");
	printf("\nOptions:\n");
	printf("\t-k N\tSelect the scale factor. 2, 2x3, 2x4, 3, 4 (default 2).\n");
	printf("\t-w\tWrap around on the borders.\n");
	printf("\t-r N\tSelect the revision of the algorithm 0-N (default 1).\n");
	printf("\nMore info at http://scale2x.sourceforge.net/\n");
	exit(EXIT_FAILURE);
}

#ifdef HAVE_GETOPT_LONG
struct option long_options[] = {
	{"scale", 1, 0, 'k'},
	{"crc", 0, 0, 'c'},
	{"only124", 0, 0, 'o'},
	{"wrap", 0, 0, 'w'},
	{"revision", 1, 0, 'r'},
	{"help", 0, 0, 'h'},
	{"version", 0, 0, 'v'},
	{0, 0, 0, 0}
};
#endif

#define OPTIONS "k:cowr:hv"

int main(int argc, char* argv[]) {
	int opt_scale_x = 2;
	int opt_scale_y = 2;
	int opt_crc = 0;
	int opt_tes = 0;
	int opt_ver = 1;
	int opt_only124 = 0;
	int max_ver;
	int c;

	opterr = 0;

	while ((c =
#ifdef HAVE_GETOPT_LONG
		getopt_long(argc, argv, OPTIONS, long_options, 0))
#else
		getopt(argc, argv, OPTIONS))
#endif
	!= EOF) {
		switch (c) {
			case 'h' :
				usage();
				exit(EXIT_SUCCESS);
			case 'v' :
				version();
				exit(EXIT_SUCCESS);
			case 'k' :
				if (strcmp(optarg, "2") == 0) {
					opt_scale_x = 2;
					opt_scale_y = 2;
				} else if (strcmp(optarg, "3") == 0) {
					opt_scale_x = 3;
					opt_scale_y = 3;
				} else if (strcmp(optarg, "4") == 0) {
					opt_scale_x = 4;
					opt_scale_y = 4;
				} else {
					if (sscanf(optarg, "%dx%d", &opt_scale_x, &opt_scale_y) != 2
						|| opt_scale_x < 1
						|| opt_scale_y < 1
					) {
						printf("Invalid -k option. Valid values are 2, 2x3, 2x4, 3 and 4.\n");
						exit(EXIT_FAILURE);
					}
				}
				break;
			case 'c' :
				opt_crc = 1;
				break;
			case 'o' :
				opt_only124 = 1;
				break;
			case 'w' :
				opt_tes = 1;
				break;
			case 'r' :
				opt_ver = atoi(optarg);
				break;
			default:
				printf("Unknown option `%c'.\n", (char)optopt);
				exit(EXIT_FAILURE);
		} 
	}

	switch (opt_scale_x * 100 + opt_scale_y) {
	case 202 : max_ver = SCALE2X_REVISION_MAX; break;
	case 203 : max_ver = SCALE2X3_REVISION_MAX; break;
	case 204 : max_ver = SCALE2X4_REVISION_MAX; break;
	case 303 : max_ver = SCALE3X_REVISION_MAX; break;
	case 404 : max_ver = SCALE4X_REVISION_MAX; break;
	default : max_ver = 0; break;
	}

	if (opt_ver < 0 || opt_ver > max_ver) {
		printf("Invalid -r option. Valid values are 0 - %d.\n", max_ver);
		exit(EXIT_FAILURE);
	}

	if (optind + 2 != argc) {
		usage();
		exit(EXIT_FAILURE);
	}

	if (file_process(argv[optind], argv[optind+1], opt_scale_x, opt_scale_y, opt_tes, opt_ver, opt_crc, opt_only124) != 0) {
		exit(EXIT_FAILURE);
	}

	return EXIT_SUCCESS;
}

