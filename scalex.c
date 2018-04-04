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
 */

/*
 * This is the source of a simple command line tool which uses the fast
 * implementation of the Scale effects.
 *
 * You can find an high level description of the effects at :
 *
 * http://www.scale2x.it/
 */

#include "scalebit.h"
#include "file.h"
#include "pixel.h"
#include "scale2x.h"
#include "portable.h"

#include <zlib.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int file_process(const char* file0, const char* file1, int opt_scale_x, int opt_scale_y, int opt_crc, int opt_imp)
{
	unsigned pixel;
	unsigned width;
	unsigned height;
	void* src_alloc = 0;
	unsigned char* src_ptr;
	unsigned src_slice;
	void* dst_alloc = 0;
	unsigned char* dst_ptr;
	unsigned dst_slice;
	int type;
	int channel;
	png_color* palette = 0;
	unsigned palette_size;

	if (file_read(file0, &src_alloc, &src_ptr, &src_slice, &pixel, &width, &height, &type, &channel, &palette, &palette_size, 1) != 0) {
		goto bail;
	}

	if (scale_precondition(opt_scale_x * 100 + opt_scale_y, pixel, width, height) != 0) {
		fprintf(stderr, "Error in the size of the source bitmap. Generally this happen\n");
		fprintf(stderr, "when the bitmap is too small or when the width is not an exact\n");
		fprintf(stderr, "multiplier of 8 bytes.\n");
		goto bail;
	}

	dst_slice = scale2x_align_size(width * pixel * opt_scale_x);
	dst_alloc = malloc(dst_slice * height * opt_scale_y + SCALE2X_ALIGN_ALLOC);
	if (!dst_alloc) {
		fprintf(stderr, "Low memory.\n");
		goto bail;
	}
	dst_ptr = scale2x_align_ptr(dst_alloc);

	scale(opt_scale_x * 100 + opt_scale_y, dst_ptr, dst_slice, src_ptr, src_slice, pixel, width, height, opt_imp);

	if (file_write(file1, dst_ptr, dst_slice, pixel, width * opt_scale_x, height * opt_scale_y, type, channel, palette, palette_size) != 0) {
		goto bail;
	}

	if (opt_crc) {
		unsigned dst_width = width * opt_scale_x;
		unsigned dst_height = height * opt_scale_y;
		unsigned y;
		unsigned crc = 0;
		for (y = 0; y < dst_height; ++y)
			crc = crc32(crc, dst_ptr + y * dst_slice, dst_width * pixel);
		printf("%08x\n", crc);
	}

	free(dst_alloc);
	free(src_alloc);
	free(palette);

	return 0;

bail:
	free(dst_alloc);
	free(src_alloc);
	free(palette);
	return -1;
}

/**
 * Differential us of two timeval.
 */
static long long diffgettimeofday(struct timeval *start, struct timeval *stop)
{
	long long d;

	d = 1000000LL * (stop->tv_sec - start->tv_sec);
	d += stop->tv_usec - start->tv_usec;

	return d;
}

int image_speed(const unsigned char* src_ptr, unsigned src_slice, unsigned pixel, unsigned width, unsigned height, int opt_scale_x, int opt_scale_y, int opt_imp)
{
	void* dst_alloc = 0;
	unsigned char* dst_ptr = 0;
	unsigned dst_slice;
	struct timeval start;
	struct timeval stop;
	long long amount;
	long long elapsed;

	dst_slice = scale2x_align_size(width * pixel * opt_scale_x);
	dst_alloc = malloc(dst_slice * height * opt_scale_y + SCALE2X_ALIGN_ALLOC);
	if (!dst_alloc) {
		fprintf(stderr, "Low memory.\n");
		goto bail;
	}
	dst_ptr = scale2x_align_ptr(dst_alloc);

	if (gettimeofday(&start, 0) != 0) {
		fprintf(stderr, "Time error.\n");
		goto bail;
	}

	printf("Speed test with %d bits per pixel\n", pixel * 8);

	amount = 0;
	while (1) {
		unsigned i;

		for (i = 0; i < 1000; ++i)
			scale(opt_scale_x * 100 + opt_scale_y, dst_ptr, dst_slice, src_ptr, src_slice, pixel, width, height, opt_imp);

		amount += i * width * height * pixel;

		printf(".");
		fflush(stdout);

		if (gettimeofday(&stop, 0) != 0) {
			fprintf(stderr, "Time error.\n");
			goto bail;
		}

		elapsed = diffgettimeofday(&start, &stop);

		if (elapsed > 2000000)
			break;
	}

	printf("\nInput data processed at %g MB/s\n", amount / (double)elapsed);

	free(dst_alloc);

	return 0;

bail:
	free(dst_alloc);
	return -1;
}

/**
 * Integer reversible hash function for 32 bits.
 * Implementation of the Robert Jenkins "4-byte Integer Hashing",
 * from http://burtleburtle.net/bob/hash/integer.html
 */
unsigned inthash(unsigned key)
{
	key -= key << 6;
	key ^= key >> 17;
	key -= key << 9;
	key ^= key << 4;
	key -= key << 3;
	key ^= key << 10;
	key ^= key >> 15;

	return key;
}

int file_speed(const char* file0, int opt_scale_x, int opt_scale_y, int opt_imp)
{
	unsigned pixel;
	unsigned width;
	unsigned height;
	void* src_alloc = 0;
	unsigned char* src_ptr = 0;
	unsigned src_slice;
	int type;
	int channel;
	png_color* palette = 0;
	unsigned palette_size;
	unsigned pack;

	if (file_read(file0, &src_alloc, &src_ptr, &src_slice, &pixel, &width, &height, &type, &channel, &palette, &palette_size, 1) != 0) {
		goto bail;
	}

	if (scale_precondition(opt_scale_x * 100 + opt_scale_y, pixel, width, height) != 0) {
		fprintf(stderr, "Error in the size of the source bitmap. Generally this happen\n");
		fprintf(stderr, "when the bitmap is too small or when the width is not an exact\n");
		fprintf(stderr, "multiplier of 8 bytes.\n");
		goto bail;
	}

	/* repeat the test reducing the bit depth */
	for (pack = 1; pixel / pack != 0; pack *= 2) {
		void* tmp_alloc;
		unsigned char* tmp_ptr;
		unsigned tmp_slice;
		unsigned tmp_pixel = pixel / pack;
		unsigned x, y;

		tmp_slice = scale2x_align_size(width * tmp_pixel);
		tmp_alloc = malloc(tmp_slice * height + SCALE2X_ALIGN_ALLOC);
		if (!tmp_alloc) {
			fprintf(stderr, "Low memory.\n");
			goto bail;
		}
		tmp_ptr = scale2x_align_ptr(tmp_alloc);

		/* dummy packing of the image to reduce bit depth */
		for (y = 0; y < height; ++y) {
			for (x = 0; x < width; ++x) {
				unsigned v = pixel_get(x, y, src_ptr, src_slice, pixel, width, height, 0);

				/* dummy transformation to diffuse the pixel value */
				v = inthash(v);

				pixel_put(x, y, tmp_ptr, tmp_slice, tmp_pixel, width, height, v);
			}
		}

		image_speed(tmp_ptr, tmp_slice, tmp_pixel, width, height, opt_scale_x, opt_scale_y, opt_imp);

		free(tmp_alloc);
	}

	free(src_alloc);
	free(palette);

	return 0;

bail:
	free(src_alloc);
	free(palette);
	return -1;
}

void version(void)
{
	printf(PACKAGE " v" VERSION " by Andrea Mazzoleni, " PACKAGE_URL "\n");
}

void usage(void)
{
	version();
	printf("Fast implementation of the Scale2/3/4x effects\n");
#ifdef USE_SCALE2X_SSE2
	printf("(using SSE2 vector implementation)\n");
#endif
#ifdef USE_SCALE2X_NEON
	printf("(using NEON vector implementation)\n");
#endif
	printf("\nSyntax: scalex [-k N] FROM.png TO.png\n");
	printf("\nOptions:\n");
	printf("\t-k N\tSelect the scale factor. 2, 2x3, 2x4, 3 or 4. (default 2).\n");
#if defined(USE_SCALE2X_SSE2) || defined(USE_SCALE2X_NEON)
	printf("\t-o novect\tDisable vector implementation.\n");
#endif
	printf("\t-o nomem\tDisable random memory access implementation.\n");
	printf("\nMore info at " PACKAGE_URL "\n");
	exit(EXIT_FAILURE);
}

#ifdef HAVE_GETOPT_LONG
struct option long_options[] = {
	{ "scale", 1, 0, 'k' },
	{ "opt", 1, 0, 'o' },
	{ "speed", 0, 0, 'T' },
	{ "crc", 0, 0, 'c' },
	{ "help", 0, 0, 'h' },
	{ "version", 0, 0, 'v' },
	{ 0, 0, 0, 0 }
};
#endif

#define OPTIONS "k:o:Tchv"

int main(int argc, char* argv[])
{
	int opt_scale_x = 2;
	int opt_scale_y = 2;
	int opt_crc = 0;
	int opt_speed = 0;
	int opt_imp = 0;
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
		case 'o' :
			if (strcmp(optarg, "novect") == 0) {
				opt_imp |= SCALE2X_OPT_NOVECT;
			} else if (strcmp(optarg, "nomem") == 0) {
				opt_imp |= SCALE2X_OPT_NOMEM;
			} else {
				printf("Invalid -o option. Valid values are novect, nomem.\n");
				exit(EXIT_FAILURE);
			}
			break;
		case 'c' :
			opt_crc = 1;
			break;
		case 'T' :
			opt_speed = 1;
			break;
		default :
			printf("Unknown option `%c'.\n", (char)optopt);
			exit(EXIT_FAILURE);
		}
	}

	if (opt_speed != 0) {
		if (optind + 1 != argc) {
			usage();
			exit(EXIT_FAILURE);
		}

		if (file_speed(argv[optind], opt_scale_x, opt_scale_y, opt_imp) != 0) {
			exit(EXIT_FAILURE);
		}
	} else {
		if (optind + 2 != argc) {
			usage();
			exit(EXIT_FAILURE);
		}

		if (file_process(argv[optind], argv[optind + 1], opt_scale_x, opt_scale_y, opt_crc, opt_imp) != 0) {
			exit(EXIT_FAILURE);
		}
	}

	return EXIT_SUCCESS;
}

