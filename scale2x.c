/*
 * This file is part of the Scale2x project.
 *
 * Copyright (C) 2001-2002 Andrea Mazzoleni
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

#include <png.h>
#include <stdlib.h>
#include <stdio.h>

#include "portable.h"

int file_write(const char* file, unsigned width, unsigned height, png_byte** row, int color_type, 	png_color* palette, int palette_size)
{
	png_struct* png_ptr;
	png_info* info_ptr;
	FILE* fp = fopen(file, "wb");
	if (!fp) {
		fprintf(stderr,"Error opening file %s\n", file);
		return -1;
	}

	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
	if (!png_ptr) {
		fclose(fp);
		return -1;
	}

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		png_destroy_write_struct(&png_ptr, 0);
		fclose(fp);
		return -1;
	}

	if (setjmp(png_jmpbuf(png_ptr))) {
		png_destroy_write_struct(&png_ptr, &info_ptr);
		fclose(fp);
		return -1;
	}

	png_init_io(png_ptr, fp);

	png_set_IHDR(png_ptr, info_ptr, width, height, 8, color_type, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	if (color_type == PNG_COLOR_TYPE_PALETTE) {
		png_set_PLTE(png_ptr, info_ptr, palette, palette_size);
	}

	png_write_info(png_ptr, info_ptr);

	png_write_image(png_ptr, row);

	png_write_end(png_ptr, info_ptr);

	png_destroy_write_struct(&png_ptr, &info_ptr);

	fclose(fp);

	return 0;
}

unsigned get(int x, int y, png_byte** pix, unsigned dx, unsigned dy, unsigned dp, int opt_tes)
{
	png_byte* p;

	if (opt_tes) {
		if (x < 0)
			x += dx;
		if (x >= dx)
			x -= dx;
		if (y < 0)
			y += dy;
		if (y >= dy)
			y -= dy;
	} else {
		if (x < 0)
			x = 0;
		if (x >= dx)
			x = dx - 1;
		if (y < 0)
			y = 0;
		if (y >= dy)
			y = dy - 1;
	}

	p = &pix[y][x*dp];

	switch (dp) {
	case 1 :
		return p[0];
	case 2 :
		return p[0] | p[1] << 8;
	case 3 :
		return p[0] | p[1] << 8 | p[2] << 16;
	case 4 :
		return p[0] | p[1] << 8 | p[2] << 16 | p[3] << 24;
	default:
		return 0;
	}
}

void put(int x, int y, png_byte** pix, unsigned dx, unsigned dy, unsigned dp, unsigned v)
{
	png_byte* p;

	p = &pix[y][x*dp];

	switch (dp) {
	case 1 :
		p[0] = v;
		break;
	case 2 :
		p[0] = v;
		p[1] = v >> 8;
		break;
	case 3 :
		p[0] = v;
		p[1] = v >> 8;
		p[2] = v >> 16;
		break;
	case 4 :
		p[0] = v;
		p[1] = v >> 8;
		p[2] = v >> 16;
		p[3] = v >> 24;
		break;
	}
}

#define REVISION_MAX 3

void scale2x(png_byte** dst, png_byte** src, unsigned dx, unsigned dy, unsigned dp, int opt_tes, int opt_ver) {
	int x;
	int y;

	for(y=0;y<dy;++y) {
		for(x=0;x<dx;++x) {
			unsigned E0,E1,E2,E3;
			unsigned A,B,C,D,E,F,G,H,I;

			A = get(x-1,y-1,src,dx,dy,dp,opt_tes);
			B = get(x,y-1,src,dx,dy,dp,opt_tes);
			C = get(x+1,y-1,src,dx,dy,dp,opt_tes);
			D = get(x-1,y,src,dx,dy,dp,opt_tes);
			E = get(x,y,src,dx,dy,dp,opt_tes);
			F = get(x+1,y,src,dx,dy,dp,opt_tes);
			G = get(x-1,y+1,src,dx,dy,dp,opt_tes);
			H = get(x,y+1,src,dx,dy,dp,opt_tes);
			I = get(x+1,y+1,src,dx,dy,dp,opt_tes);

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
				/* version 1 */
				E0 = D == B && B != F && D != H ? D : E;
				E1 = B == F && B != D && F != H ? F : E;
				E2 = D == H && D != B && H != F ? D : E;
				E3 = H == F && D != H && B != F ? F : E;
				break;
			case 2 :
				/* version 2 */
				E0 = D == B && ((B != F && D != H) || D == A) ? D : E;
				E1 = B == F && ((B != D && F != H) || B == C) ? F : E;
				E2 = D == H && ((D != B && H != F) || D == G) ? D : E;
				E3 = H == F && ((D != H && B != F) || H == I) ? F : E;
				break;
			case 3 :
				/* version 3 */
				E0 = D == B && A != E ? D : E;
				E1 = B == F && C != E ? F : E;
				E2 = D == H && G != E ? D : E;
				E3 = H == F && I != E ? F : E;
				break;
			}

			put(x*2,y*2,dst,dx*2,dy*2,dp,E0);
			put(x*2+1,y*2,dst,dx*2,dy*2,dp,E1);
			put(x*2,y*2+1,dst,dx*2,dy*2,dp,E2);
			put(x*2+1,y*2+1,dst,dx*2,dy*2,dp,E3);
		}
	}
}

int file_transform(const char* file, png_struct* png_ptr, png_info* info_ptr, png_byte** row, int opt_tes, int opt_ver) {
	png_uint_32 width;
	png_uint_32 height;
	int bit_depth;
	int color_type;
	int interlace_type;
	int compression_type;
	int filter_type;
	png_color* palette;
	int palette_size;
	png_byte** row_tra;
	unsigned scan;
	unsigned i;
	unsigned dp;

	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, &compression_type, &filter_type);

	switch (color_type) {
	case PNG_COLOR_TYPE_GRAY :
		palette = 0;
		palette_size = 0;
		dp = 1;
	break;
	case PNG_COLOR_TYPE_GRAY_ALPHA :
		palette = 0;
		palette_size = 0;
		dp = 2;
	break;
	case PNG_COLOR_TYPE_PALETTE :
		png_get_PLTE(png_ptr, info_ptr, &palette, &palette_size);
		dp = 1;
	break;
	case PNG_COLOR_TYPE_RGB :
		palette = 0;
		palette_size = 0;
		dp = 3;
	break;
	case PNG_COLOR_TYPE_RGB_ALPHA :
		palette = 0;
		palette_size = 0;
		dp = 4;
	break;
	default:
		return -1;
	}

	scan = width * 2 * dp;

	row_tra = malloc(sizeof(void*) * height * 2);
	if (!row_tra) {
		fprintf(stderr,"Low memory\n");
		return -1;
	}
	for(i=0;i<height * 2;++i) {
		row_tra[i] = malloc(scan);
		if (!row_tra[i]) {
			fprintf(stderr,"Low memory\n");
			return -1;
		}
	}

	scale2x(row_tra, row, width, height, dp, opt_tes, opt_ver);

	if (file_write(file, width * 2, height * 2, row_tra, color_type, palette, palette_size) != 0)
		return -1;

	return 0;
}

int file_process(const char* file0, const char* file1, int opt_tes, int opt_ver) {
	png_struct* png_ptr;
	png_info* info_ptr;
	png_byte** row;

	FILE* fp = fopen(file0, "rb");
	if (!fp) {
		fprintf(stderr,"Error opening file %s\n", file0);
		return -1;
	}

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
	if (!png_ptr) {
		fclose(fp);
		return -1;
	}

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
		fclose(fp);
		return -1;
	}

	if (setjmp(png_jmpbuf(png_ptr))) {
		png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
		fclose(fp);
		return -1;
	}

	png_init_io(png_ptr, fp);

	png_set_sig_bytes(png_ptr, 0);

	png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_PACKING | PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_SHIFT | PNG_TRANSFORM_EXPAND, NULL);

	row = png_get_rows(png_ptr, info_ptr);

	if (file_transform(file1, png_ptr, info_ptr, row, opt_tes, opt_ver) != 0) {
		png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
		fclose(fp);
		return -1;
	}

	png_destroy_read_struct(&png_ptr, &info_ptr, 0);

	fclose(fp);

	return 0;
}

void version(void) {
	printf(PACKAGE " v" VERSION " by Andrea Mazzoleni\n");
}

void usage(void) {
	version();
	printf("Syntax: scale2x [-t] [-r N] FROM.png TO.png\n");
	printf("Options:\n");
	printf("-w\tWrap around on the borders.\n");
	printf("-r N\tSelect the revision of the algorithm 0-%d (default 1).\n", REVISION_MAX);
	exit(EXIT_FAILURE);
}

#ifdef HAVE_GETOPT_LONG
struct option long_options[] = {
	{"wrap", 0, 0, 'w'},
	{"revision", 1, 0, 'r'},
	{"help", 0, 0, 'h'},
	{"version", 0, 0, 'v'},
	{0, 0, 0, 0}
};
#endif

#define OPTIONS "whvr:"

int main(int argc, char* argv[]) {
	int opt_tes = 0;
	int opt_ver = 1;
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
			case 'w' :
				opt_tes = 1;
				break;
			case 'r' :
				opt_ver = atoi(optarg);
				if (opt_ver < 0 || opt_ver > REVISION_MAX) {
					printf("Invalid -r option. Valid values are 0 - %d\n", REVISION_MAX);
					exit(EXIT_FAILURE);
				}
				break;
			default:
				printf("Unknown option `%c'\n", (char)optopt);
				exit(EXIT_FAILURE);
		} 
	}

	if (optind + 2 != argc) {
		usage();
		exit(EXIT_FAILURE);
	}

	if (file_process(argv[optind], argv[optind+1], opt_tes, opt_ver) != 0) {
		exit(EXIT_FAILURE);
	}

	return EXIT_SUCCESS;
}
