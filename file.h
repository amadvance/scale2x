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

#ifndef __FILE_H
#define __FILE_H

#include <png.h>

int file_read(const char* file, unsigned char** ptr, unsigned* slice, unsigned* pixel, unsigned* width, unsigned* height, int* type, int* channel, png_color** palette, unsigned* palette_size, int allow_only124);
int file_write(const char* file, unsigned char* ptr, unsigned slice, unsigned pixel, unsigned width, unsigned height, int type, int channel, png_color* palette, unsigned palette_size);

#endif

