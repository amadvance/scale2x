/*
 * This file is part of the Scale2x project.
 *
 * Copyright (C) 2001-2003 Andrea Mazzoleni
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

#include "file.h"

#include <stdio.h>
#include <stdlib.h>

int file_write(const char* file, unsigned char* ptr, unsigned slice, unsigned pixel, unsigned width, unsigned height, int type, int channel, png_color* palette, unsigned palette_size)
{
	png_struct* png_ptr;
	png_info* info_ptr;
	png_byte** row;
	unsigned i;
	int bit_depth;
	FILE* fp;

	row = malloc(sizeof(void*) * height);
	if (!row) {
		fprintf(stderr,"Low memory.\n");
		goto err;
	}
	for(i=0;i<height;++i) {
		row[i] = ptr + i * slice;
	}

	fp = fopen(file, "wb");
	if (!fp) {
		fprintf(stderr,"Error creating file %s.\n", file);
		goto err_free;
	}

	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
	if (!png_ptr) {
		goto err_close;
	}

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		png_destroy_write_struct(&png_ptr, 0);
		goto err_close;
	}

	if (setjmp(png_jmpbuf(png_ptr))) {
		goto err_destroy;
	}

	png_init_io(png_ptr, fp);

	bit_depth = pixel * 8 / channel;

	png_set_IHDR(png_ptr, info_ptr, width, height, bit_depth, type, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	if (type == PNG_COLOR_TYPE_PALETTE) {
		png_set_PLTE(png_ptr, info_ptr, palette, palette_size);
	}

	png_write_info(png_ptr, info_ptr);

	png_write_image(png_ptr, row);

	png_write_end(png_ptr, info_ptr);

	png_destroy_write_struct(&png_ptr, &info_ptr);
	fclose(fp);
	free(row);

	return 0;

err_destroy:
	png_destroy_write_struct(&png_ptr, &info_ptr);
err_close:
	fclose(fp);
err_free:
	free(row);
err:
	return -1;
}

int file_read(const char* file, unsigned char** ptr, unsigned* slice, unsigned* pixel, unsigned* width, unsigned* height, int* type, int* channel, png_color** palette, unsigned* palette_size, int allow_only124)
{
	png_struct* png_ptr;
	png_info* info_ptr;
	png_byte** row;
	png_color* pal;
	int size;
	png_color_8p sig_bit;
	int bit_depth;
	int pre_type;
	int pre_channel;
	unsigned i;
	FILE* fp;

	/* always freed on error */
	row = 0;
	*ptr = 0;
	*palette = 0;

	fp = fopen(file, "rb");
	if (!fp) {
		fprintf(stderr,"Error opening file %s.\n", file);
		goto err;
	}

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
	if (!png_ptr) {
		goto err_close;
	}

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		png_destroy_read_struct(&png_ptr, 0, 0);
		goto err_close;
	}

	if (setjmp(png_jmpbuf(png_ptr))) {
		goto err_destroy;
	}

	png_init_io(png_ptr, fp);

	png_set_sig_bytes(png_ptr, 0);

	png_read_info(png_ptr, info_ptr);

	bit_depth = png_get_bit_depth(png_ptr, info_ptr);
	pre_channel = png_get_channels(png_ptr, info_ptr);
	pre_type = png_get_color_type(png_ptr, info_ptr);

	/* expand grey images */
	if (pre_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
		png_set_gray_1_2_4_to_8(png_ptr);

	/* convert RNS to ALPHA channel */
	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
		png_set_tRNS_to_alpha(png_ptr);

	/* reduce to 8 bit if the pixel size doesn't fit in 4 byte */
	if (allow_only124 && pre_channel * ((bit_depth + 7) / 8) > 4)
		png_set_strip_16(png_ptr);

	/* normalize the pixel value after an expand */
	if (pre_type != PNG_COLOR_TYPE_PALETTE) {
		if (png_get_sBIT(png_ptr, info_ptr, &sig_bit))
			png_set_shift(png_ptr, sig_bit);
	}

	/* expand to 8 bit */
	if (bit_depth < 8)
		png_set_packing(png_ptr);

	/* add alpha channel */
	if (allow_only124 && pre_type == PNG_COLOR_TYPE_RGB)
		png_set_filler(png_ptr, 0xFF, PNG_FILLER_AFTER);

	png_read_update_info(png_ptr, info_ptr);

	bit_depth = png_get_bit_depth(png_ptr, info_ptr);
	*channel = png_get_channels(png_ptr, info_ptr);
	*width = png_get_image_width(png_ptr, info_ptr);
	*height = png_get_image_height(png_ptr, info_ptr);
	*type = png_get_color_type(png_ptr, info_ptr);

	if (*type == PNG_COLOR_TYPE_PALETTE) {
		png_get_PLTE(png_ptr, info_ptr, &pal, &size);
		*palette = malloc(sizeof(png_color) * size);
		if (!*palette) {
			fprintf(stderr,"Low memory.\n");
			return -1;
		}
		*palette_size = size;
		for(i=0;i<*palette_size;++i)
			(*palette)[i] = pal[i];
	} else {
		*palette = 0;
		*palette_size = 0;
	}

	*pixel = *channel * ((bit_depth + 7) / 8);
	if (*type == PNG_COLOR_TYPE_RGB && *channel == 4)
		*type = PNG_COLOR_TYPE_RGB_ALPHA;

	*slice = *width * *pixel;
	*ptr = malloc(*height * *slice);
	if (!*ptr) {
		fprintf(stderr,"Low memory.\n");
		goto err_destroy;
	}

	row = malloc(sizeof(void*) * *height);
	if (!row) {
		fprintf(stderr,"Low memory.\n");
		goto err_destroy;
	}
	for(i=0;i<*height;++i) {
		row[i] = *ptr + i * *slice;
	}

	png_read_image(png_ptr, row);

	png_read_end(png_ptr, 0);

	free(row);
	png_destroy_read_struct(&png_ptr, &info_ptr, 0);
	fclose(fp);

	return 0;

err_destroy:
	free(*palette);
	free(*ptr);
	free(row);
	png_destroy_read_struct(&png_ptr, &info_ptr, 0);
err_close:
	fclose(fp);
err:
	return -1;
}

