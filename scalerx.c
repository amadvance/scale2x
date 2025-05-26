/*
 * This file is part of the Scale2x project.
 *
 * Copyright (C) 2003, 2013 Andrea Mazzoleni
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
 * This is the source of a simple command line tool which uses the reference
 * implementation of the Scale effects.
 *
 * You can find an high level description of the effects at :
 *
 * http://www.scale2x.it/
 */

#include "file.h"
#include "pixel.h"
#include "portable.h"

#include <zlib.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void scalex(unsigned char* dst_ptr, unsigned dst_slice, const unsigned char* src_ptr, unsigned src_slice, unsigned pixel, int width, int height, unsigned mx, unsigned my)
{
	int x;
	int y;

	for (y = 0; y < height; ++y) {
		for (x = 0; x < width; ++x) {
			pixel_t E;
			unsigned i, j;

			E = pixel_get(x, y, src_ptr, src_slice, pixel, width, height, 0);

			for (i = 0; i < mx; ++i)
				for (j = 0; j < my; ++j)
					pixel_put(x * mx + i, y * my + j, dst_ptr, dst_slice, pixel, width * mx, height * my, E);
		}
	}
}

#define SCALE2X_REVISION_MAX 7

pixel_t lerp(pixel_t v0, pixel_t v1, double f)
{
	unsigned r0, g0, b0;
	unsigned r1, g1, b1;

	r0 = v0 & 0xFF;
	g0 = (v0 >> 8) & 0xFF;
	b0 = (v0 >> 16) & 0xFF;

	r1 = v1 & 0xFF;
	g1 = (v1 >> 8) & 0xFF;
	b1 = (v1 >> 16) & 0xFF;

	r0 = r0 * f + r1 * (1.0 - f);
	g0 = g0 * f + g1 * (1.0 - f);
	b0 = b0 * f + b1 * (1.0 - f);

	if (r0 > 255) r0 = 255;
	if (g0 > 255) g0 = 255;
	if (b0 > 255) b0 = 255;

	v0 = r0 + (g0 << 8) + (b0 << 16);

	return v0 | 0xFF000000;
}

unsigned dist(pixel_t v0, pixel_t v1)
{
	int r0, g0, b0;
	int r1, g1, b1;

	r0 = v0 & 0xFF;
	g0 = (v0 >> 8) & 0xFF;
	b0 = (v0 >> 16) & 0xFF;

	r1 = v1 & 0xFF;
	g1 = (v1 >> 8) & 0xFF;
	b1 = (v1 >> 16) & 0xFF;

	r0 -= r1;
	g0 -= g1;
	b0 -= b1;

	if (r0 < 0) r0 = -r0;
	if (g0 < 0) g0 = -g0;
	if (b0 < 0) b0 = -b0;

	return 3 * r0 + 6 * g0 + b0;
}

void scale2x(unsigned char* dst_ptr, unsigned dst_slice, const unsigned char* src_ptr, unsigned src_slice, unsigned pixel, int width, int height, int opt_tes, int opt_ver)
{
	int x;
	int y;

	for (y = 0; y < height; ++y) {
		for (x = 0; x < width; ++x) {
			pixel_t O[4];
			pixel_t A, B, C, D, E, F, G, H, I;
			pixel_t A1, B1, C1, G5, H5, I5, A0, D0, G0, C4, F4, I4;

/*
        ABC
        DEF
        GHI

        O0O1
        O2O3
 */

			A = pixel_get(x - 1, y - 1, src_ptr, src_slice, pixel, width, height, opt_tes);
			B = pixel_get(x, y - 1, src_ptr, src_slice, pixel, width, height, opt_tes);
			C = pixel_get(x + 1, y - 1, src_ptr, src_slice, pixel, width, height, opt_tes);
			D = pixel_get(x - 1, y, src_ptr, src_slice, pixel, width, height, opt_tes);
			E = pixel_get(x, y, src_ptr, src_slice, pixel, width, height, opt_tes);
			F = pixel_get(x + 1, y, src_ptr, src_slice, pixel, width, height, opt_tes);
			G = pixel_get(x - 1, y + 1, src_ptr, src_slice, pixel, width, height, opt_tes);
			H = pixel_get(x, y + 1, src_ptr, src_slice, pixel, width, height, opt_tes);
			I = pixel_get(x + 1, y + 1, src_ptr, src_slice, pixel, width, height, opt_tes);


/*
     0  1  2  3  4

   0       A1 B1 C1
   1    A0 PA PB PC C4
   2    D0 PD PE PF F4
   3    G0 PG PH PI I4
   4       G5 H5 I5
 */

			A1 = pixel_get(x - 1, y - 2, src_ptr, src_slice, pixel, width, height, opt_tes);
			B1 = pixel_get(x, y - 2, src_ptr, src_slice, pixel, width, height, opt_tes);
			C1 = pixel_get(x + 1, y - 2, src_ptr, src_slice, pixel, width, height, opt_tes);
			G5 = pixel_get(x - 1, y + 2, src_ptr, src_slice, pixel, width, height, opt_tes);
			H5 = pixel_get(x, y + 2, src_ptr, src_slice, pixel, width, height, opt_tes);
			I5 = pixel_get(x + 1, y + 2, src_ptr, src_slice, pixel, width, height, opt_tes);
			A0 = pixel_get(x - 2, y - 1, src_ptr, src_slice, pixel, width, height, opt_tes);
			D0 = pixel_get(x - 2, y, src_ptr, src_slice, pixel, width, height, opt_tes);
			G0 = pixel_get(x - 2, y + 1, src_ptr, src_slice, pixel, width, height, opt_tes);
			C4 = pixel_get(x + 2, y - 1, src_ptr, src_slice, pixel, width, height, opt_tes);
			F4 = pixel_get(x + 2, y, src_ptr, src_slice, pixel, width, height, opt_tes);
			I4 = pixel_get(x + 2, y + 1, src_ptr, src_slice, pixel, width, height, opt_tes);

			switch (opt_ver) {
			default :
			case 0 :
				/* version 0, normal scaling */
				O[0] = E;
				O[1] = E;
				O[2] = E;
				O[3] = E;
				break;
			case 1 :
				/* default */
				O[0] = D == B && B != F && D != H ? D : E;
				O[1] = B == F && B != D && F != H ? F : E;
				O[2] = D == H && D != B && H != F ? D : E;
				O[3] = H == F && D != H && B != F ? F : E;
				break;

#define SCALE2K_BASE(A, B, C, D, E, F, G, H, I, N0, N1, N2, N3) \
	if (D == B && B != E && D != E) { \
		/* diagonal */ \
		if (B == C && D == G) { \
			/* square block */ \
		} \
		else if (B == C) { \
			/* horizontal slope */ \
			O[N0] = lerp(D, O[N0], 0.75); \
			O[N1] = lerp(D, O[N1], 0.25); \
		} else if (D == G) { \
			/* vertical slope */ \
			O[N0] = lerp(D, O[N0], 0.75); \
			O[N2] = lerp(D, O[N2], 0.25); \
		} else { \
			/* pure diagonal */ \
			O[N0] = lerp(O[N0], D, 0.5); \
		} \
	}

/* Used by AdvanceMAME */
#define SCALE2K(A, B, C, D, E, F, G, H, I, N0, N1, N2, N3) \
	if (D == B && B != E && D != E) { \
		/* diagonal */ \
		if (B == C && D == G) { \
			/* square block */ \
			if (A != E) { \
				/* no star */ \
				O[N0] = lerp(D, O[N0], 0.75); \
				O[N1] = lerp(D, O[N1], 0.25); \
				O[N2] = lerp(D, O[N2], 0.25); \
			} \
		} else if (B == C && C != F) { \
			/* horizontal slope */ \
			O[N0] = lerp(D, O[N0], 0.75); \
			O[N1] = lerp(D, O[N1], 0.25); \
		} else if (D == G && G != H) { \
			/* vertical slope */ \
			O[N0] = lerp(D, O[N0], 0.75); \
			O[N2] = lerp(D, O[N2], 0.25); \
		} else { \
			/* pure diagonal */ \
			O[N0] = lerp(O[N0], D, 0.5); \
		} \
	}

#define SCALE2K_SPIKE(A, B, C, D, E, F, G, H, I, N0, N1, N2, N3) \
	if (B != E && D != E) { \
		if (D == B) { \
			/* diagonal */ \
			if (B == C && D == G) { \
				/* square block */ \
			} else if (B == C) { \
				/* horizontal slope */ \
				O[N0] = lerp(D, O[N0], 0.75); \
				O[N1] = lerp(D, O[N1], 0.25); \
			} else if (D == G) { \
				/* vertical slope */ \
				O[N0] = lerp(D, O[N0], 0.75); \
				O[N2] = lerp(D, O[N2], 0.25); \
			} else { \
				/* pure diagonal */ \
				O[N0] = lerp(O[N0], D, 0.5); \
			} \
		} else if (A == B && G == E) { \
			/* horizontal spike */ \
			O[N0] = lerp(D, O[N0], 0.5); \
		} else if (A == D && C == E) { \
			/* vertical spike */ \
			O[N0] = lerp(B, O[N0], 0.5); \
		} \
	}

#define SCALE2K_ALTERNATE(A, B, C, D, E, F, G, H, I, N0, N1, N2, N3) \
	if (D == B && B != F && D != H) { \
		/* diagonal */ \
		if (B == C && D == G) { \
			/* square block */ \
			if (A != E) { \
				O[N0] = lerp(D, O[N0], 0.75); \
				O[N1] = lerp(D, O[N1], 0.25); \
				O[N2] = lerp(D, O[N2], 0.25); \
			} \
		} else if (B == C && C != F) { \
			/* horizontal slope */ \
			O[N0] = lerp(D, O[N0], 0.75); \
			O[N1] = lerp(D, O[N1], 0.25); \
		} else if (D == G && G != H) { \
			/* vertical slope */ \
			O[N0] = lerp(D, O[N0], 0.75); \
			O[N2] = lerp(D, O[N2], 0.25); \
		} else { \
			/* pure diagonal */ \
			O[N0] = lerp(O[N0], D, 0.5); \
		} \
	}

			case 2 :
				/* scalek */
				O[0] = O[1] = O[2] = O[3] = E;

				SCALE2K(A, B, C, D, E, F, G, H, I, 0, 1, 2, 3);
				SCALE2K(G, D, A, H, E, B, I, F, C, 2, 0, 3, 1);
				SCALE2K(I, H, G, F, E, D, C, B, A, 3, 2, 1, 0);
				SCALE2K(C, F, I, B, E, H, A, D, G, 1, 3, 0, 2);

				break;


#define XBR_LV2_2X(PE, PI, PH, PF, PG, PC, PD, PB, PA, G5, C4, G0, D0, C1, B1, F4, I4, H5, I5, A0, A1, N0, N1, N2, N3) \
	if (PE != PH && PE != PF) { \
		unsigned e = dist(PC, PE) + dist(PE, PG) + dist(H5, PI) + dist (PI, F4) + 4 * dist(PH, PF); \
		unsigned i = dist(PD, PH) + dist(PH, I5) + dist(I4, PF) + dist(PF, PB) + 4 * dist(PE, PI); \
		if (e < i) { \
			int ex2 = PE != PC && PB != PC; \
			int ex3 = PE != PG && PD != PG; \
			unsigned ke = dist(PF, PG); \
			unsigned ki = dist(PH, PC); \
			pixel_t px = dist(PE, PF) <= dist(PE, PH) ? PF : PH; \
			if (ke == 0 && ki == 0 && ex3 && ex2) { \
				LEFT_UP_2_2X(N3, N2, N1, px); \
			} else if (2 * ke <= ki && ex3) { \
				LEFT_2_2X(N3, N2, px); \
			} else if (ke >= 2 * ki && ex2) { \
				UP_2_2X(N3, N1, px); \
			} else { \
				DIA_2X(N3, px); \
			} \
		} \
	}

// XBR Level 3 - Enhanced edge detection with better diagonal handling
#define XBR_LV3_2X(PE, PI, PH, PF, PG, PC, PD, PB, PA, G5, C4, G0, D0, C1, B1, F4, I4, H5, I5, A0, A1, N0, N1, N2, N3) \
	if (PE != PH && PE != PF) { \
		unsigned e = dist(PC, PE) + dist(PE, PG) + dist(H5, PI) + dist (PI, F4) + 4 * dist(PH, PF); \
		unsigned i = dist(PD, PH) + dist(PH, I5) + dist(I4, PF) + dist(PF, PB) + 4 * dist(PE, PI); \
		if (e < i) { \
			int ex2 = PE != PC && PB != PC; \
			int ex3 = PE != PG && PD != PG; \
			unsigned ke = dist(PF, PG); \
			unsigned ki = dist(PH, PC); \
			int left_up = ex3 && ex2; \
			int left_only = ex3 && !ex2; \
			int up_only = !ex3 && ex2; \
			pixel_t px = dist(PE, PF) <= dist(PE, PH) ? PF : PH; \
			\
			/* Enhanced blending conditions for Level 3 */ \
			if (left_up && (ke * 2 <= ki) && (ki * 2 <= ke)) { \
				LEFT_UP_2_2X_LV3(N3, N2, N1, px, PE); \
			} else if (left_only && (ke * 2 <= ki)) { \
				LEFT_2_2X_LV3(N3, N2, px, PE); \
			} else if (up_only && (ki * 2 <= ke)) { \
				UP_2_2X_LV3(N3, N1, px, PE); \
			} else if (left_up) { \
				LEFT_UP_2_2X(N3, N2, N1, px); \
			} else if (left_only) { \
				LEFT_2_2X(N3, N2, px); \
			} else if (up_only) { \
				UP_2_2X(N3, N1, px); \
			} else { \
				DIA_2X(N3, px); \
			} \
		} \
	}

/* XBR Level 4 - Most advanced with gradient-aware interpolation */
#define XBR_LV4_2X(PE, PI, PH, PF, PG, PC, PD, PB, PA, G5, C4, G0, D0, C1, B1, F4, I4, H5, I5, A0, A1, N0, N1, N2, N3) \
	if (PE != PH && PE != PF) { \
		unsigned e = dist(PC, PE) + dist(PE, PG) + dist(H5, PI) + dist (PI, F4) + 4 * dist(PH, PF); \
		unsigned i = dist(PD, PH) + dist(PH, I5) + dist(I4, PF) + dist(PF, PB) + 4 * dist(PE, PI); \
		if (e < i) { \
			int ex2 = PE != PC && PB != PC; \
			int ex3 = PE != PG && PD != PG; \
			unsigned ke = dist(PF, PG); \
			unsigned ki = dist(PH, PC); \
			\
			/* Enhanced gradient detection for Level 4 */ \
			unsigned grad_h = dist(PA, PB) + dist(PB, PC) + dist(PD, PE) + dist(PE, PF) + dist(PG, PH) + dist(PH, PI); \
			unsigned grad_v = dist(A1, PA) + dist(PA, PD) + dist(B1, PB) + dist(PB, PE) + dist(C1, PC) + dist(PC, PF); \
			unsigned grad_ratio = (grad_h > grad_v) ? (grad_h * 8) / (grad_v + 1) : (grad_v * 8) / (grad_h + 1); \
			\
			int left_up = ex3 && ex2; \
			int left_only = ex3 && !ex2; \
			int up_only = !ex3 && ex2; \
			pixel_t px = dist(PE, PF) <= dist(PE, PH) ? PF : PH; \
			\
			/* Gradient-aware blending for Level 4 */ \
			if (left_up && grad_ratio > 4) { \
				LEFT_UP_2_2X_LV4(N3, N2, N1, px, PE, grad_ratio); \
			} else if (left_only && (ke * 3 <= ki * 2) && grad_ratio > 2) { \
				LEFT_2_2X_LV4(N3, N2, px, PE, grad_ratio); \
			} else if (up_only && (ki * 3 <= ke * 2) && grad_ratio > 2) { \
				UP_2_2X_LV4(N3, N1, px, PE, grad_ratio); \
			} else if (left_up) { \
				LEFT_UP_2_2X_LV3(N3, N2, N1, px, PE); \
			} else if (left_only) { \
				LEFT_2_2X_LV3(N3, N2, px, PE); \
			} else if (up_only) { \
				UP_2_2X_LV3(N3, N1, px, PE); \
			} else { \
				DIA_2X_LV4(N3, px, PE); \
			} \
		} \
	}

/* Level 2 interpolation macros */
#define LEFT_UP_2_2X(N3, N2, N1, PIXEL) \
	O[N3] = lerp(PIXEL, O[N3], 7.0 / 8); \
	O[N1] = O[N2] = lerp(O[N2], PIXEL, 3.0 / 4);

#define LEFT_2_2X(N3, N2, PIXEL) \
	O[N3] = lerp(PIXEL, O[N3], 3.0 / 4); \
	O[N2] = lerp(O[N2], PIXEL, 3.0 / 4);

#define UP_2_2X(N3, N1, PIXEL) \
	O[N3] = lerp(PIXEL, O[N3], 3.0 / 4); \
	O[N1] = lerp(O[N1], PIXEL, 3.0 / 4);

#define DIA_2X(N3, PIXEL) \
	O[N3] = lerp(O[N3], PIXEL, 1.0 / 2);

/* Level 3 enhanced interpolation macros */
#define LEFT_UP_2_2X_LV3(N3, N2, N1, PIXEL, PE) \
	O[N3] = lerp(PIXEL, O[N3], 5.0 / 8); \
	O[N1] = lerp(O[N1], PIXEL, 5.0 / 8); \
	O[N2] = lerp(O[N2], PIXEL, 5.0 / 8);

#define LEFT_2_2X_LV3(N3, N2, PIXEL, PE) \
	O[N3] = lerp(PIXEL, O[N3], 5.0 / 8); \
	O[N2] = lerp(O[N2], PIXEL, 5.0 / 8);

#define UP_2_2X_LV3(N3, N1, PIXEL, PE) \
	O[N3] = lerp(PIXEL, O[N3], 5.0 / 8); \
	O[N1] = lerp(O[N1], PIXEL, 5.0 / 8);

/* Level 4 gradient-aware interpolation macros */
#define LEFT_UP_2_2X_LV4(N3, N2, N1, PIXEL, PE, GRAD) \
	{ \
		if (GRAD > 8) { \
			O[N3] = lerp(PIXEL, O[N3], 7.0 / 8); \
			O[N1] = lerp(O[N1], PIXEL, 7.0 / 8); \
			O[N2] = lerp(O[N2], PIXEL, 7.0 / 8); \
		} else { \
			O[N3] = lerp(PIXEL, O[N3], 5.0 / 8); \
			O[N1] = lerp(O[N1], PIXEL, 5.0 / 8); \
			O[N2] = lerp(O[N2], PIXEL, 5.0 / 8); \
		} \
	}

#define LEFT_2_2X_LV4(N3, N2, PIXEL, PE, GRAD) \
	{ \
		if (GRAD > 8) { \
			O[N3] = lerp(PIXEL, O[N3], 7.0 / 8); \
			O[N2] = lerp(O[N2], PIXEL, 7.0 / 8); \
		} else { \
			O[N3] = lerp(PIXEL, O[N3], 5.0 / 8); \
			O[N2] = lerp(O[N2], PIXEL, 5.0 / 8); \
		} \
	}

#define UP_2_2X_LV4(N3, N1, PIXEL, PE, GRAD) \
	{ \
		if (GRAD > 8) { \
			O[N3] = lerp(PIXEL, O[N3], 7.0 / 8); \
			O[N1] = lerp(O[N1], PIXEL, 7.0 / 8); \
		} else { \
			O[N3] = lerp(PIXEL, O[N3], 5.0 / 8); \
			O[N1] = lerp(O[N1], PIXEL, 5.0 / 8); \
		} \
	}

#define DIA_2X_LV4(N3, PIXEL, PE) \
	O[N3] = lerp(O[N3], PIXEL, 3.0 / 8);

			case 3 :
				/* xbr level 2 */
				O[0] = O[1] = O[2] = O[3] = E;

				XBR_LV2_2X(E, I, H, F, G, C, D, B, A, G5, C4, G0, D0, C1, B1, F4, I4, H5, I5, A0, A1, 0, 1, 2, 3);
				XBR_LV2_2X(E, C, F, B, I, A, H, D, G, I4, A1, I5, H5, A0, D0, B1, C1, F4, C4, G5, G0, 2, 0, 3, 1);
				XBR_LV2_2X(E, A, B, D, C, G, F, H, I, C1, G0, C4, F4, G5, H5, D0, A0, B1, A1, I4, I5, 3, 2, 1, 0);
				XBR_LV2_2X(E, G, D, H, A, I, B, F, C, A0, I5, A1, B1, I4, F4, H5, G5, D0, G0, C1, C4, 1, 3, 0, 2);
				break;
			case 4 :
				/* xbr level 3 */
				O[0] = O[1] = O[2] = O[3] = E;

				XBR_LV3_2X(E, I, H, F, G, C, D, B, A, G5, C4, G0, D0, C1, B1, F4, I4, H5, I5, A0, A1, 0, 1, 2, 3);
				XBR_LV3_2X(E, C, F, B, I, A, H, D, G, I4, A1, I5, H5, A0, D0, B1, C1, F4, C4, G5, G0, 2, 0, 3, 1);
				XBR_LV3_2X(E, A, B, D, C, G, F, H, I, C1, G0, C4, F4, G5, H5, D0, A0, B1, A1, I4, I5, 3, 2, 1, 0);
				XBR_LV3_2X(E, G, D, H, A, I, B, F, C, A0, I5, A1, B1, I4, F4, H5, G5, D0, G0, C1, C4, 1, 3, 0, 2);
				break;
			case 5 :
				/* xbr level 4 */
				O[0] = O[1] = O[2] = O[3] = E;

				XBR_LV4_2X(E, I, H, F, G, C, D, B, A, G5, C4, G0, D0, C1, B1, F4, I4, H5, I5, A0, A1, 0, 1, 2, 3);
				XBR_LV4_2X(E, C, F, B, I, A, H, D, G, I4, A1, I5, H5, A0, D0, B1, C1, F4, C4, G5, G0, 2, 0, 3, 1);
				XBR_LV4_2X(E, A, B, D, C, G, F, H, I, C1, G0, C4, F4, G5, H5, D0, A0, B1, A1, I4, I5, 3, 2, 1, 0);
				XBR_LV4_2X(E, G, D, H, A, I, B, F, C, A0, I5, A1, B1, I4, F4, H5, G5, D0, G0, C1, C4, 1, 3, 0, 2);
				break;
			case 6 :
				/* rejected */
				O[0] = D == B && ((B != F && D != H) || D == A) ? D : E;
				O[1] = B == F && ((B != D && F != H) || B == C) ? F : E;
				O[2] = D == H && ((D != B && H != F) || D == G) ? D : E;
				O[3] = H == F && ((D != H && B != F) || H == I) ? F : E;
				break;
			case 7 :
				/* rejected, loses isolated pixels */
				O[0] = D == B && A != E ? D : E;
				O[1] = B == F && C != E ? F : E;
				O[2] = D == H && G != E ? D : E;
				O[3] = H == F && I != E ? F : E;
				break;
			}

			pixel_put(x * 2, y * 2, dst_ptr, dst_slice, pixel, width * 2, height * 2, O[0]);
			pixel_put(x * 2 + 1, y * 2, dst_ptr, dst_slice, pixel, width * 2, height * 2, O[1]);
			pixel_put(x * 2, y * 2 + 1, dst_ptr, dst_slice, pixel, width * 2, height * 2, O[2]);
			pixel_put(x * 2 + 1, y * 2 + 1, dst_ptr, dst_slice, pixel, width * 2, height * 2, O[3]);
		}
	}
}

#define SCALE3X_REVISION_MAX 11

void scale3x(unsigned char* dst_ptr, unsigned dst_slice, const unsigned char* src_ptr, unsigned src_slice, unsigned pixel, int width, int height, int opt_tes, int opt_ver)
{
	int x;
	int y;

	for (y = 0; y < height; ++y) {
		for (x = 0; x < width; ++x) {
			pixel_t O[9];
			pixel_t A, B, C, D, E, F, G, H, I;
			pixel_t A1, B1, C1, G5, H5, I5, A0, D0, G0, C4, F4, I4;
			int k0, k1, k2, k3;
			int c0, c1, c2, c3;

/*
        ABC
        DEF
        GHI

        k0k1
        k2k3

        O0O1O2
        O3O4O5
        O6O7O8
 */

			A = pixel_get(x - 1, y - 1, src_ptr, src_slice, pixel, width, height, opt_tes);
			B = pixel_get(x, y - 1, src_ptr, src_slice, pixel, width, height, opt_tes);
			C = pixel_get(x + 1, y - 1, src_ptr, src_slice, pixel, width, height, opt_tes);
			D = pixel_get(x - 1, y, src_ptr, src_slice, pixel, width, height, opt_tes);
			E = pixel_get(x, y, src_ptr, src_slice, pixel, width, height, opt_tes);
			F = pixel_get(x + 1, y, src_ptr, src_slice, pixel, width, height, opt_tes);
			G = pixel_get(x - 1, y + 1, src_ptr, src_slice, pixel, width, height, opt_tes);
			H = pixel_get(x, y + 1, src_ptr, src_slice, pixel, width, height, opt_tes);
			I = pixel_get(x + 1, y + 1, src_ptr, src_slice, pixel, width, height, opt_tes);

/*
     0  1  2  3  4

   0       A1 B1 C1
   1    A0 PA PB PC C4
   2    D0 PD PE PF F4
   3    G0 PG PH PI I4
   4       G5 H5 I5
 */

			A1 = pixel_get(x - 1, y - 2, src_ptr, src_slice, pixel, width, height, opt_tes);
			B1 = pixel_get(x, y - 2, src_ptr, src_slice, pixel, width, height, opt_tes);
			C1 = pixel_get(x + 1, y - 2, src_ptr, src_slice, pixel, width, height, opt_tes);
			G5 = pixel_get(x - 1, y + 2, src_ptr, src_slice, pixel, width, height, opt_tes);
			H5 = pixel_get(x, y + 2, src_ptr, src_slice, pixel, width, height, opt_tes);
			I5 = pixel_get(x + 1, y + 2, src_ptr, src_slice, pixel, width, height, opt_tes);
			A0 = pixel_get(x - 2, y - 1, src_ptr, src_slice, pixel, width, height, opt_tes);
			D0 = pixel_get(x - 2, y, src_ptr, src_slice, pixel, width, height, opt_tes);
			G0 = pixel_get(x - 2, y + 1, src_ptr, src_slice, pixel, width, height, opt_tes);
			C4 = pixel_get(x + 2, y - 1, src_ptr, src_slice, pixel, width, height, opt_tes);
			F4 = pixel_get(x + 2, y, src_ptr, src_slice, pixel, width, height, opt_tes);
			I4 = pixel_get(x + 2, y + 1, src_ptr, src_slice, pixel, width, height, opt_tes);

			switch (opt_ver) {
			default :
			case 0 :
				/* version 0, normal scaling */
				O[0] = E;
				O[1] = E;
				O[2] = E;
				O[3] = E;
				O[4] = E;
				O[5] = E;
				O[6] = E;
				O[7] = E;
				O[8] = E;
				break;
			case 1 :
				/* default */
				k0 = D == B && B != F && D != H;
				k1 = B == F && B != D && F != H;
				k2 = D == H && D != B && H != F;
				k3 = H == F && D != H && B != F;
				O[0] = k0 ? D : E;
				O[1] = (k0 && E != C) || (k1 && E != A) ? B : E;
				O[2] = k1 ? F : E;
				O[3] = (k0 && E != G) || (k2 && E != A) ? D : E;
				O[4] = E;
				O[5] = (k1 && E != I) || (k3 && E != C) ? F : E;
				O[6] = k2 ? D : E;
				O[7] = (k2 && E != I) || (k3 && E != G) ? H : E;
				O[8] = k3 ? F : E;
				break;

#define SCALE3K_BASE(A, B, C, D, E, F, G, H, I, N0, N1, N2, N3, N4, N5, N6, N7, N8) \
	if (D == B && B != E && D != E) { \
		/* diagonal */ \
		if (B == C && D == G) { \
			/* square block */ \
		} \
		else if (B == C) { \
			/* horizontal slope */ \
			O[N0] = D; \
			O[N1] = lerp(D, O[N1], 0.75); \
			O[N2] = lerp(D, O[N2], 0.25); \
			O[N3] = lerp(D, O[N3], 0.25); \
		} else if (D == G) { \
			/* vertical slope */ \
			O[N0] = D; \
			O[N3] = lerp(D, O[N3], 0.75); \
			O[N6] = lerp(D, O[N6], 0.25); \
			O[N1] = lerp(D, O[N1], 0.25); \
		} else { \
			/* pure diagonal */ \
			O[N0] = lerp(D, O[N0], 0.875); \
			O[N1] = lerp(D, O[N1], 0.125); \
			O[N3] = lerp(D, O[N3], 0.125); \
		} \
	}

/* Used by AdvanceMAME */
#define SCALE3K(A, B, C, D, E, F, G, H, I, N0, N1, N2, N3, N4, N5, N6, N7, N8) \
	if (D == B && B != E && D != E) { \
		/* diagonal */ \
		if (B == C && D == G) { \
			/* square block */ \
			if (A != E) { \
				O[N0] = D; \
				O[N1] = lerp(D, O[N1], 0.75); \
				O[N2] = lerp(D, O[N2], 0.25); \
				O[N3] = lerp(D, O[N3], 0.75); \
				O[N6] = lerp(D, O[N6], 0.25); \
			} \
		} else if (B == C && C != F) { \
			/* horizontal slope */ \
			O[N0] = D; \
			O[N1] = lerp(D, O[N1], 0.75); \
			O[N2] = lerp(D, O[N2], 0.25); \
			O[N3] = lerp(D, O[N3], 0.25); \
		} else if (D == G && G != H) { \
			/* vertical slope */ \
			O[N0] = D; \
			O[N3] = lerp(D, O[N3], 0.75); \
			O[N6] = lerp(D, O[N6], 0.25); \
			O[N1] = lerp(D, O[N1], 0.25); \
		} else { \
			/* pure diagonal */ \
			O[N0] = lerp(D, O[N0], 0.875); \
			O[N1] = lerp(D, O[N1], 0.125); \
			O[N3] = lerp(D, O[N3], 0.125); \
		} \
	}

#define SCALE3K_SPIKE(A, B, C, D, E, F, G, H, I, N0, N1, N2, N3, N4, N5, N6, N7, N8) \
	if (B != E && D != E) { \
		if (D == B) { \
			/* diagonal */ \
			if (B == C && D == G) { \
				/* square block */ \
			} else if (B == C) { \
				/* horizontal slope */ \
				O[N0] = D; \
				O[N1] = lerp(D, O[N1], 0.75); \
				O[N2] = lerp(D, O[N2], 0.25); \
				O[N3] = lerp(D, O[N3], 0.25); \
			} else if (D == G) { \
				/* vertical slope */ \
				O[N0] = D; \
				O[N3] = lerp(D, O[N3], 0.75); \
				O[N6] = lerp(D, O[N6], 0.25); \
				O[N1] = lerp(D, O[N1], 0.25); \
			} else { \
				/* pure diagonal */ \
				O[N0] = lerp(D, O[N0], 0.875); \
				O[N1] = lerp(D, O[N1], 0.125); \
				O[N3] = lerp(D, O[N3], 0.125); \
			} \
		} else if (A == B && G == E) { \
			/* horizontal spike */ \
			O[N0] = lerp(D, O[N0], 0.875); \
			O[N1] = lerp(D, O[N1], 0.125); \
			O[N3] = lerp(D, O[N3], 0.125); \
		} else if (A == D && C == E) { \
			/* vertical spike */ \
			O[N0] = lerp(B, O[N0], 0.875); \
			O[N1] = lerp(B, O[N1], 0.125); \
			O[N3] = lerp(B, O[N3], 0.125); \
		} \
	}

#define SCALE3K_ALTERNATE(A, B, C, D, E, F, G, H, I, N0, N1, N2, N3, N4, N5, N6, N7, N8) \
	if (D == B && B != F && D != H) { \
		/* diagonal */ \
		if (B == C && D == G) { \
			/* square block */ \
			if (A != E) { \
				O[N0] = D; \
				O[N1] = lerp(D, O[N1], 0.75); \
				O[N2] = lerp(D, O[N2], 0.25); \
				O[N3] = lerp(D, O[N3], 0.75); \
				O[N6] = lerp(D, O[N6], 0.25); \
			} \
		} else if (B == C && C != F) { \
			/* horizontal slope */ \
			O[N0] = D; \
			O[N1] = lerp(D, O[N1], 0.75); \
			O[N2] = lerp(D, O[N2], 0.25); \
			O[N3] = lerp(D, O[N3], 0.25); \
		} else if (D == G && G != H) { \
			/* vertical slope */ \
			O[N0] = D; \
			O[N3] = lerp(D, O[N3], 0.75); \
			O[N6] = lerp(D, O[N6], 0.25); \
			O[N1] = lerp(D, O[N1], 0.25); \
		} else { \
			/* pure diagonal */ \
			O[N0] = lerp(D, O[N0], 0.875); \
			O[N1] = lerp(D, O[N1], 0.125); \
			O[N3] = lerp(D, O[N3], 0.125); \
		} \
	}

			case 2 :
				/* scalek */
				O[0] = O[1] = O[2] = O[3] = O[4] = O[5] = O[6] = O[7] = O[8] = E;

				SCALE3K(A, B, C, D, E, F, G, H, I, 0, 1, 2, 3, 4, 5, 6, 7, 8);
				SCALE3K(G, D, A, H, E, B, I, F, C, 6, 3, 0, 7, 4, 1, 8, 5, 2);
				SCALE3K(I, H, G, F, E, D, C, B, A, 8, 7, 6, 5, 4, 3, 2, 1, 0);
				SCALE3K(C, F, I, B, E, H, A, D, G, 2, 5, 8, 1, 4, 7, 0, 3, 6);
				break;

/* XBR Level 2 for 3x scaling */
#define XBR_LV2_3X(PE, PI, PH, PF, PG, PC, PD, PB, PA, G5, C4, G0, D0, C1, B1, F4, I4, H5, I5, A0, A1, N0, N1, N2, N3, N4, N5, N6, N7, N8) \
	if (PE != PH && PE != PF) { \
		unsigned e = dist(PC, PE) + dist(PE, PG) + dist(H5, PI) + dist(PI, F4) + 4 * dist(PH, PF); \
		unsigned i = dist(PD, PH) + dist(PH, I5) + dist(I4, PF) + dist(PF, PB) + 4 * dist(PE, PI); \
		if (e < i) { \
			int ex2 = PE != PC && PB != PC; \
			int ex3 = PE != PG && PD != PG; \
			unsigned ke = dist(PF, PG); \
			unsigned ki = dist(PH, PC); \
			pixel_t px = dist(PE, PF) <= dist(PE, PH) ? PF : PH; \
			if (ke == 0 && ki == 0 && ex3 && ex2) { \
				LEFT_UP_2_3X(N0, N1, N2, N3, N4, N5, N6, N7, N8, px); \
			} else if (2 * ke <= ki && ex3) { \
				LEFT_2_3X(N0, N1, N2, N3, N4, N5, N6, N7, N8, px); \
			} else if (ke >= 2 * ki && ex2) { \
				UP_2_3X(N0, N1, N2, N3, N4, N5, N6, N7, N8, px); \
			} else { \
				DIA_3X(N0, N1, N2, N3, N4, N5, N6, N7, N8, px); \
			} \
		} \
	}

/* XBR Level 3 for 3x scaling - Enhanced edge detection */
#define XBR_LV3_3X(PE, PI, PH, PF, PG, PC, PD, PB, PA, G5, C4, G0, D0, C1, B1, F4, I4, H5, I5, A0, A1, N0, N1, N2, N3, N4, N5, N6, N7, N8) \
	if (PE != PH && PE != PF) { \
		unsigned e = dist(PC, PE) + dist(PE, PG) + dist(H5, PI) + dist(PI, F4) + 4 * dist(PH, PF); \
		unsigned i = dist(PD, PH) + dist(PH, I5) + dist(I4, PF) + dist(PF, PB) + 4 * dist(PE, PI); \
		if (e < i) { \
			int ex2 = PE != PC && PB != PC; \
			int ex3 = PE != PG && PD != PG; \
			unsigned ke = dist(PF, PG); \
			unsigned ki = dist(PH, PC); \
			int left_up = ex3 && ex2; \
			int left_only = ex3 && !ex2; \
			int up_only = !ex3 && ex2; \
			pixel_t px = dist(PE, PF) <= dist(PE, PH) ? PF : PH; \
			\
			/* Enhanced blending conditions for Level 3 */ \
			if (left_up && (ke * 2 <= ki) && (ki * 2 <= ke)) { \
				LEFT_UP_2_3X_LV3(N0, N1, N2, N3, N4, N5, N6, N7, N8, px, PE); \
			} else if (left_only && (ke * 2 <= ki)) { \
				LEFT_2_3X_LV3(N0, N1, N2, N3, N4, N5, N6, N7, N8, px, PE); \
			} else if (up_only && (ki * 2 <= ke)) { \
				UP_2_3X_LV3(N0, N1, N2, N3, N4, N5, N6, N7, N8, px, PE); \
			} else if (left_up) { \
				LEFT_UP_2_3X(N0, N1, N2, N3, N4, N5, N6, N7, N8, px); \
			} else if (left_only) { \
				LEFT_2_3X(N0, N1, N2, N3, N4, N5, N6, N7, N8, px); \
			} else if (up_only) { \
				UP_2_3X(N0, N1, N2, N3, N4, N5, N6, N7, N8, px); \
			} else { \
				DIA_3X(N0, N1, N2, N3, N4, N5, N6, N7, N8, px); \
			} \
		} \
	}

/* XBR Level 4 for 3x scaling - Most advanced with gradient-aware interpolation */
#define XBR_LV4_3X(PE, PI, PH, PF, PG, PC, PD, PB, PA, G5, C4, G0, D0, C1, B1, F4, I4, H5, I5, A0, A1, N0, N1, N2, N3, N4, N5, N6, N7, N8) \
	if (PE != PH && PE != PF) { \
		unsigned e = dist(PC, PE) + dist(PE, PG) + dist(H5, PI) + dist(PI, F4) + 4 * dist(PH, PF); \
		unsigned i = dist(PD, PH) + dist(PH, I5) + dist(I4, PF) + dist(PF, PB) + 4 * dist(PE, PI); \
		if (e < i) { \
			int ex2 = PE != PC && PB != PC; \
			int ex3 = PE != PG && PD != PG; \
			unsigned ke = dist(PF, PG); \
			unsigned ki = dist(PH, PC); \
			\
			/* Enhanced gradient detection for Level 4 */ \
			unsigned grad_h = dist(PA, PB) + dist(PB, PC) + dist(PD, PE) + dist(PE, PF) + dist(PG, PH) + dist(PH, PI); \
			unsigned grad_v = dist(A1, PA) + dist(PA, PD) + dist(B1, PB) + dist(PB, PE) + dist(C1, PC) + dist(PC, PF); \
			unsigned grad_ratio = (grad_h > grad_v) ? (grad_h * 8) / (grad_v + 1) : (grad_v * 8) / (grad_h + 1); \
			\
			int left_up = ex3 && ex2; \
			int left_only = ex3 && !ex2; \
			int up_only = !ex3 && ex2; \
			pixel_t px = dist(PE, PF) <= dist(PE, PH) ? PF : PH; \
			\
			/* Gradient-aware blending for Level 4 */ \
			if (left_up && grad_ratio > 4) { \
				LEFT_UP_2_3X_LV4(N0, N1, N2, N3, N4, N5, N6, N7, N8, px, PE, grad_ratio); \
			} else if (left_only && (ke * 3 <= ki * 2) && grad_ratio > 2) { \
				LEFT_2_3X_LV4(N0, N1, N2, N3, N4, N5, N6, N7, N8, px, PE, grad_ratio); \
			} else if (up_only && (ki * 3 <= ke * 2) && grad_ratio > 2) { \
				UP_2_3X_LV4(N0, N1, N2, N3, N4, N5, N6, N7, N8, px, PE, grad_ratio); \
			} else if (left_up) { \
				LEFT_UP_2_3X_LV3(N0, N1, N2, N3, N4, N5, N6, N7, N8, px, PE); \
			} else if (left_only) { \
				LEFT_2_3X_LV3(N0, N1, N2, N3, N4, N5, N6, N7, N8, px, PE); \
			} else if (up_only) { \
				UP_2_3X_LV3(N0, N1, N2, N3, N4, N5, N6, N7, N8, px, PE); \
			} else { \
				DIA_3X_LV4(N0, N1, N2, N3, N4, N5, N6, N7, N8, px, PE); \
			} \
		} \
	}

/* Level 2 interpolation macros for 3x scaling */
#define LEFT_UP_2_3X(N0, N1, N2, N3, N4, N5, N6, N7, N8, PIXEL) \
	O[N0] = lerp(PIXEL, O[N0], 7.0 / 8); \
	O[N1] = lerp(O[N1], PIXEL, 3.0 / 4); \
	O[N3] = lerp(O[N3], PIXEL, 3.0 / 4); \
	O[N4] = lerp(O[N4], PIXEL, 1.0 / 2);

#define LEFT_2_3X(N0, N1, N2, N3, N4, N5, N6, N7, N8, PIXEL) \
	O[N0] = lerp(PIXEL, O[N0], 3.0 / 4); \
	O[N3] = lerp(O[N3], PIXEL, 3.0 / 4); \
	O[N6] = lerp(O[N6], PIXEL, 3.0 / 4);

#define UP_2_3X(N0, N1, N2, N3, N4, N5, N6, N7, N8, PIXEL) \
	O[N0] = lerp(PIXEL, O[N0], 3.0 / 4); \
	O[N1] = lerp(O[N1], PIXEL, 3.0 / 4); \
	O[N2] = lerp(O[N2], PIXEL, 3.0 / 4);

#define DIA_3X(N0, N1, N2, N3, N4, N5, N6, N7, N8, PIXEL) \
	O[N0] = lerp(O[N0], PIXEL, 1.0 / 2);

/* Level 3 enhanced interpolation macros for 3x scaling */
#define LEFT_UP_2_3X_LV3(N0, N1, N2, N3, N4, N5, N6, N7, N8, PIXEL, PE) \
	O[N0] = lerp(PIXEL, O[N0], 5.0 / 8); \
	O[N1] = lerp(O[N1], PIXEL, 5.0 / 8); \
	O[N3] = lerp(O[N3], PIXEL, 5.0 / 8); \
	O[N4] = lerp(O[N4], PIXEL, 3.0 / 8);

#define LEFT_2_3X_LV3(N0, N1, N2, N3, N4, N5, N6, N7, N8, PIXEL, PE) \
	O[N0] = lerp(PIXEL, O[N0], 5.0 / 8); \
	O[N3] = lerp(O[N3], PIXEL, 5.0 / 8); \
	O[N6] = lerp(O[N6], PIXEL, 5.0 / 8);

#define UP_2_3X_LV3(N0, N1, N2, N3, N4, N5, N6, N7, N8, PIXEL, PE) \
	O[N0] = lerp(PIXEL, O[N0], 5.0 / 8); \
	O[N1] = lerp(O[N1], PIXEL, 5.0 / 8); \
	O[N2] = lerp(O[N2], PIXEL, 5.0 / 8);

/* Level 4 gradient-aware interpolation macros for 3x scaling */
#define LEFT_UP_2_3X_LV4(N0, N1, N2, N3, N4, N5, N6, N7, N8, PIXEL, PE, GRAD) \
	{ \
		if (GRAD > 8) { \
			O[N0] = lerp(PIXEL, O[N0], 7.0 / 8); \
			O[N1] = lerp(O[N1], PIXEL, 7.0 / 8); \
			O[N3] = lerp(O[N3], PIXEL, 7.0 / 8); \
			O[N4] = lerp(O[N4], PIXEL, 5.0 / 8); \
		} else { \
			O[N0] = lerp(PIXEL, O[N0], 5.0 / 8); \
			O[N1] = lerp(O[N1], PIXEL, 5.0 / 8); \
			O[N3] = lerp(O[N3], PIXEL, 5.0 / 8); \
			O[N4] = lerp(O[N4], PIXEL, 3.0 / 8); \
		} \
	}

#define LEFT_2_3X_LV4(N0, N1, N2, N3, N4, N5, N6, N7, N8, PIXEL, PE, GRAD) \
	{ \
		if (GRAD > 8) { \
			O[N0] = lerp(PIXEL, O[N0], 7.0 / 8); \
			O[N3] = lerp(O[N3], PIXEL, 7.0 / 8); \
			O[N6] = lerp(O[N6], PIXEL, 7.0 / 8); \
		} else { \
			O[N0] = lerp(PIXEL, O[N0], 5.0 / 8); \
			O[N3] = lerp(O[N3], PIXEL, 5.0 / 8); \
			O[N6] = lerp(O[N6], PIXEL, 5.0 / 8); \
		} \
	}

#define UP_2_3X_LV4(N0, N1, N2, N3, N4, N5, N6, N7, N8, PIXEL, PE, GRAD) \
	{ \
		if (GRAD > 8) { \
			O[N0] = lerp(PIXEL, O[N0], 7.0 / 8); \
			O[N1] = lerp(O[N1], PIXEL, 7.0 / 8); \
			O[N2] = lerp(O[N2], PIXEL, 7.0 / 8); \
		} else { \
			O[N0] = lerp(PIXEL, O[N0], 5.0 / 8); \
			O[N1] = lerp(O[N1], PIXEL, 5.0 / 8); \
			O[N2] = lerp(O[N2], PIXEL, 5.0 / 8); \
		} \
	}

#define DIA_3X_LV4(N0, N1, N2, N3, N4, N5, N6, N7, N8, PIXEL, PE) \
	O[N0] = lerp(O[N0], PIXEL, 3.0 / 8);

			case 3:
				/* xbr level 2 */
				O[0] = O[1] = O[2] = O[3] = O[4] = O[5] = O[6] = O[7] = O[8] = E;

				/* Apply XBR algorithm for each quadrant of the 3x3 grid */
				XBR_LV2_3X(E, I, H, F, G, C, D, B, A, G5, C4, G0, D0, C1, B1, F4, I4, H5, I5, A0, A1, 0, 1, 2, 3, 4, 5, 6, 7, 8);
				XBR_LV2_3X(E, C, F, B, I, A, H, D, G, I4, A1, I5, H5, A0, D0, B1, C1, F4, C4, G5, G0, 2, 5, 8, 1, 4, 7, 0, 3, 6);
				XBR_LV2_3X(E, A, B, D, C, G, F, H, I, C1, G0, C4, F4, G5, H5, D0, A0, B1, A1, I4, I5, 8, 7, 6, 5, 4, 3, 2, 1, 0);
				XBR_LV2_3X(E, G, D, H, A, I, B, F, C, A0, I5, A1, B1, I4, F4, H5, G5, D0, G0, C1, C4, 6, 3, 0, 7, 4, 1, 8, 5, 2);
				break;
			case 4:
				/* xbr level 3 */
				O[0] = O[1] = O[2] = O[3] = O[4] = O[5] = O[6] = O[7] = O[8] = E;

				XBR_LV3_3X(E, I, H, F, G, C, D, B, A, G5, C4, G0, D0, C1, B1, F4, I4, H5, I5, A0, A1, 0, 1, 2, 3, 4, 5, 6, 7, 8);
				XBR_LV3_3X(E, C, F, B, I, A, H, D, G, I4, A1, I5, H5, A0, D0, B1, C1, F4, C4, G5, G0, 2, 5, 8, 1, 4, 7, 0, 3, 6);
				XBR_LV3_3X(E, A, B, D, C, G, F, H, I, C1, G0, C4, F4, G5, H5, D0, A0, B1, A1, I4, I5, 8, 7, 6, 5, 4, 3, 2, 1, 0);
				XBR_LV3_3X(E, G, D, H, A, I, B, F, C, A0, I5, A1, B1, I4, F4, H5, G5, D0, G0, C1, C4, 6, 3, 0, 7, 4, 1, 8, 5, 2);
				break;
			case 5:
				/* xbr level 4 */
				O[0] = O[1] = O[2] = O[3] = O[4] = O[5] = O[6] = O[7] = O[8] = E;

				XBR_LV4_3X(E, I, H, F, G, C, D, B, A, G5, C4, G0, D0, C1, B1, F4, I4, H5, I5, A0, A1, 0, 1, 2, 3, 4, 5, 6, 7, 8);
				XBR_LV4_3X(E, C, F, B, I, A, H, D, G, I4, A1, I5, H5, A0, D0, B1, C1, F4, C4, G5, G0, 2, 5, 8, 1, 4, 7, 0, 3, 6);
				XBR_LV4_3X(E, A, B, D, C, G, F, H, I, C1, G0, C4, F4, G5, H5, D0, A0, B1, A1, I4, I5, 8, 7, 6, 5, 4, 3, 2, 1, 0);
				XBR_LV4_3X(E, G, D, H, A, I, B, F, C, A0, I5, A1, B1, I4, F4, H5, G5, D0, G0, C1, C4, 6, 3, 0, 7, 4, 1, 8, 5, 2);
				break;
			case 6 :
				/* rejected */
				O[0] = D == B && B != F && D != H ? D : E;
				O[1] = E;
				O[2] = B == F && B != D && F != H ? F : E;
				O[3] = E;
				O[4] = E;
				O[5] = E;
				O[6] = D == H && D != B && H != F ? D : E;
				O[7] = E;
				O[8] = H == F && D != H && B != F ? F : E;
				break;
			case 7 :
				/* rejected */
				O[0] = D == B && ((B != F && D != H) || D == A) ? D : E;
				O[1] = E;
				O[2] = B == F && ((B != D && F != H) || B == C) ? F : E;
				O[3] = E;
				O[4] = E;
				O[5] = E;
				O[6] = D == H && ((D != B && H != F) || D == G) ? D : E;
				O[7] = E;
				O[8] = H == F && ((D != H && B != F) || H == I) ? F : E;
				break;
			case 8 :
				/* rejected */
				O[0] = D == B && A != E ? D : E;
				O[1] = E;
				O[2] = B == F && C != E ? F : E;
				O[3] = E;
				O[4] = E;
				O[5] = E;
				O[6] = D == H && G != E ? D : E;
				O[7] = E;
				O[8] = H == F && I != E ? F : E;
				break;
			case 9 :
				/* rejected */
				k0 = D == B && B != F && D != H;
				k1 = B == F && B != D && F != H;
				k2 = D == H && D != B && H != F;
				k3 = H == F && D != H && B != F;
				c0 = k0 && E != C;
				c1 = k1 && E != B;
				c2 = k2 && E != H;
				c3 = k3 && E != H;
				O[0] = k0 ? D : E;
				O[1] = c0 || c1 ? B : E;
				O[2] = k1 ? F : E;
				O[3] = c0 || c2 ? D : E;
				O[4] = E;
				O[5] = c1 || c3 ? F : E;
				O[6] = k2 ? D : E;
				O[7] = c2 || c3 ? H : E;
				O[8] = k3 ? F : E;
				break;
			case 10 :
				/* rejected */
				k0 = D == B && B != F && D != H;
				k1 = B == F && B != D && F != H;
				k2 = D == H && D != B && H != F;
				k3 = H == F && D != H && B != F;
				O[0] = k0 ? D : E;
				O[1] = k0 != k1 ? B : E;
				O[2] = k1 ? F : E;
				O[3] = k0 != k2 ? D : E;
				O[4] = E;
				O[5] = k1 != k3 ? F : E;
				O[6] = k2 ? D : E;
				O[7] = k2 != k3 ? H : E;
				O[8] = k3 ? F : E;
				break;
			case 11 :
				/* rejected */
				k0 = D == B && B != F && D != H;
				k1 = B == F && B != D && F != H;
				k2 = D == H && D != B && H != F;
				k3 = H == F && D != H && B != F;
				O[0] = k0 ? D : E;
				O[1] = k0 || k1 ? B : E;
				O[2] = k1 ? F : E;
				O[3] = k0 || k2 ? D : E;
				O[4] = E;
				O[5] = k1 || k3 ? F : E;
				O[6] = k2 ? D : E;
				O[7] = k2 || k3 ? H : E;
				O[8] = k3 ? F : E;
				break;
			}

			pixel_put(x * 3, y * 3, dst_ptr, dst_slice, pixel, width * 3, height * 3, O[0]);
			pixel_put(x * 3 + 1, y * 3, dst_ptr, dst_slice, pixel, width * 3, height * 3, O[1]);
			pixel_put(x * 3 + 2, y * 3, dst_ptr, dst_slice, pixel, width * 3, height * 3, O[2]);
			pixel_put(x * 3, y * 3 + 1, dst_ptr, dst_slice, pixel, width * 3, height * 3, O[3]);
			pixel_put(x * 3 + 1, y * 3 + 1, dst_ptr, dst_slice, pixel, width * 3, height * 3, O[4]);
			pixel_put(x * 3 + 2, y * 3 + 1, dst_ptr, dst_slice, pixel, width * 3, height * 3, O[5]);
			pixel_put(x * 3, y * 3 + 2, dst_ptr, dst_slice, pixel, width * 3, height * 3, O[6]);
			pixel_put(x * 3 + 1, y * 3 + 2, dst_ptr, dst_slice, pixel, width * 3, height * 3, O[7]);
			pixel_put(x * 3 + 2, y * 3 + 2, dst_ptr, dst_slice, pixel, width * 3, height * 3, O[8]);
		}
	}
}

#define SCALE4X_REVISION_MAX 3

int scale4x(unsigned char* dst_ptr, unsigned dst_slice, const unsigned char* src_ptr, unsigned src_slice, unsigned pixel, int width, int height, int opt_tes, int opt_ver)
{
	int x;
	int y;

	if (opt_ver != 0 && opt_ver != 2 && opt_ver != 3) {
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

	for (y = 0; y < height; ++y) {
		for (x = 0; x < width; ++x) {
			pixel_t O[16];
			pixel_t A, B, C, D, E, F, G, H, I;
			pixel_t A1, B1, C1, G5, H5, I5, A0, D0, G0, C4, F4, I4;

/*
        ABC
        DEF
        GHI

        O0O1O2O3
        O4O5O6O7
        O8O9OAOB
        OCODOOOF
 */

			A = pixel_get(x - 1, y - 1, src_ptr, src_slice, pixel, width, height, opt_tes);
			B = pixel_get(x, y - 1, src_ptr, src_slice, pixel, width, height, opt_tes);
			C = pixel_get(x + 1, y - 1, src_ptr, src_slice, pixel, width, height, opt_tes);
			D = pixel_get(x - 1, y, src_ptr, src_slice, pixel, width, height, opt_tes);
			E = pixel_get(x, y, src_ptr, src_slice, pixel, width, height, opt_tes);
			F = pixel_get(x + 1, y, src_ptr, src_slice, pixel, width, height, opt_tes);
			G = pixel_get(x - 1, y + 1, src_ptr, src_slice, pixel, width, height, opt_tes);
			H = pixel_get(x, y + 1, src_ptr, src_slice, pixel, width, height, opt_tes);
			I = pixel_get(x + 1, y + 1, src_ptr, src_slice, pixel, width, height, opt_tes);

/*
     0  1  2  3  4

   0       A1 B1 C1
   1    A0 PA PB PC C4
   2    D0 PD PE PF F4
   3    G0 PG PH PI I4
   4       G5 H5 I5
 */

			A1 = pixel_get(x - 1, y - 2, src_ptr, src_slice, pixel, width, height, opt_tes);
			B1 = pixel_get(x, y - 2, src_ptr, src_slice, pixel, width, height, opt_tes);
			C1 = pixel_get(x + 1, y - 2, src_ptr, src_slice, pixel, width, height, opt_tes);
			G5 = pixel_get(x - 1, y + 2, src_ptr, src_slice, pixel, width, height, opt_tes);
			H5 = pixel_get(x, y + 2, src_ptr, src_slice, pixel, width, height, opt_tes);
			I5 = pixel_get(x + 1, y + 2, src_ptr, src_slice, pixel, width, height, opt_tes);
			A0 = pixel_get(x - 2, y - 1, src_ptr, src_slice, pixel, width, height, opt_tes);
			D0 = pixel_get(x - 2, y, src_ptr, src_slice, pixel, width, height, opt_tes);
			G0 = pixel_get(x - 2, y + 1, src_ptr, src_slice, pixel, width, height, opt_tes);
			C4 = pixel_get(x + 2, y - 1, src_ptr, src_slice, pixel, width, height, opt_tes);
			F4 = pixel_get(x + 2, y, src_ptr, src_slice, pixel, width, height, opt_tes);
			I4 = pixel_get(x + 2, y + 1, src_ptr, src_slice, pixel, width, height, opt_tes);


			switch (opt_ver) {
			default :
			case 0 :
				/* version 0, normal scaling */
				O[0] = E;
				O[1] = E;
				O[2] = E;
				O[3] = E;
				O[4] = E;
				O[5] = E;
				O[6] = E;
				O[7] = E;
				O[8] = E;
				O[9] = E;
				O[10] = E;
				O[11] = E;
				O[12] = E;
				O[13] = E;
				O[14] = E;
				O[15] = E;
				break;

#define SCALE4K_BASE(A, B, C, D, E, F, G, H, I, N0, N1, N2, N3, N4, N5, N6, N7, N8, N9, N10, N11, N12, N13, N14, N15) \
	if (D == B && B != E && D != E) { \
		/* diagonal */ \
		if (B == C && D == G) { \
			/* square block */ \
		} \
		else if (B == C && C != F) { \
			/* horizontal slope */ \
			O[N0] = D; \
			O[N1] = D; \
			O[N2] = lerp(D, O[N2], 0.75); \
			O[N3] = lerp(D, O[N3], 0.25); \
			O[N4] = lerp(D, O[N4], 0.75); \
			O[N5] = lerp(D, O[N5], 0.25); \
		} else if (D == G && G != H) { \
			/* vertical slope */ \
			O[N0] = D; \
			O[N4] = D; \
			O[N8] = lerp(D, O[N8], 0.75); \
			O[N12] = lerp(D, O[N12], 0.25); \
			O[N1] = lerp(D, O[N1], 0.75); \
			O[N5] = lerp(D, O[N5], 0.25); \
		} else { \
			/* pure diagonal */ \
			O[N0] = D; \
			O[N1] = lerp(D, O[N1], 0.5); \
			O[N4] = lerp(D, O[N4], 0.5); \
		} \
	}

/* Used by AdvanceMAME */
#define SCALE4K(A, B, C, D, E, F, G, H, I, N0, N1, N2, N3, N4, N5, N6, N7, N8, N9, N10, N11, N12, N13, N14, N15) \
	if (D == B && B != E && D != E) { \
		/* diagonal */ \
		if (B == C && D == G) { \
			if (A != E) { \
				/* no star */ \
				O[N0] = D; \
				O[N1] = D; \
				O[N2] = lerp(D, O[N2], 0.75); \
				O[N3] = lerp(D, O[N3], 0.25); \
				O[N4] = D; \
				O[N8] = lerp(D, O[N8], 0.75); \
				O[N12] = lerp(D, O[N12], 0.25); \
				O[N5] = lerp(D, O[N5], 0.50); \
			} \
		} else if (B == C && C != F) { \
			/* horizontal slope */ \
			O[N0] = D; \
			O[N1] = D; \
			O[N2] = lerp(D, O[N2], 0.75); \
			O[N3] = lerp(D, O[N3], 0.25); \
			O[N4] = lerp(D, O[N4], 0.75); \
			O[N5] = lerp(D, O[N5], 0.25); \
		} else if (D == G && G != H) { \
			/* vertical slope */ \
			O[N0] = D; \
			O[N4] = D; \
			O[N8] = lerp(D, O[N8], 0.75); \
			O[N12] = lerp(D, O[N12], 0.25); \
			O[N1] = lerp(D, O[N1], 0.75); \
			O[N5] = lerp(D, O[N5], 0.25); \
		} else { \
			/* pure diagonal */ \
			O[N0] = D; \
			O[N1] = lerp(D, O[N1], 0.5); \
			O[N4] = lerp(D, O[N4], 0.5); \
		} \
	}

#define SCALE4K_ALTERNATE(A, B, C, D, E, F, G, H, I, N0, N1, N2, N3, N4, N5, N6, N7, N8, N9, N10, N11, N12, N13, N14, N15) \
	if (D == B && B != F && D != H) { \
		/* diagonal */ \
		if (B == C && D == G) { \
			/* avoid starts */ \
			if (A != E) { \
				O[N0] = D; \
				O[N1] = D; \
				O[N2] = lerp(D, O[N2], 0.75); \
				O[N3] = lerp(D, O[N3], 0.25); \
				O[N4] = D; \
				O[N8] = lerp(D, O[N8], 0.75); \
				O[N12] = lerp(D, O[N12], 0.25); \
				O[N5] = lerp(D, O[N5], 0.50); \
			} \
		} else if (B == C && C != F) { \
			/* horizontal slope */ \
			O[N0] = D; \
			O[N1] = D; \
			O[N2] = lerp(D, O[N2], 0.75); \
			O[N3] = lerp(D, O[N3], 0.25); \
			O[N4] = lerp(D, O[N4], 0.75); \
			O[N5] = lerp(D, O[N5], 0.25); \
		} else if (D == G && G != H) { \
			/* vertical slope */ \
			O[N0] = D; \
			O[N4] = D; \
			O[N8] = lerp(D, O[N8], 0.75); \
			O[N12] = lerp(D, O[N12], 0.25); \
			O[N1] = lerp(D, O[N1], 0.75); \
			O[N5] = lerp(D, O[N5], 0.25); \
		} else { \
			/* pure diagonal */ \
			O[N0] = D; \
			O[N1] = lerp(D, O[N1], 0.5); \
			O[N4] = lerp(D, O[N4], 0.5); \
		} \
	}

			case 2 :
				/* scalek */
				O[0] = O[1] = O[2] = O[3] = O[4] = O[5] = O[6] = O[7] = O[8] = O[9] = O[10] = O[11] = O[12] = O[13] = O[14] = O[15] = E;

				SCALE4K(A, B, C, D, E, F, G, H, I, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
				SCALE4K(G, D, A, H, E, B, I, F, C, 12, 8, 4, 0, 13, 9, 5, 1, 14, 10, 6, 2, 15, 11, 7, 3);
				SCALE4K(I, H, G, F, E, D, C, B, A, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0);
				SCALE4K(C, F, I, B, E, H, A, D, G, 3, 7, 11, 15, 2, 6, 10, 14, 1, 5, 9, 13, 0, 4, 8, 12);
				break;

/*
     0  1  2  3  4

   0       A1 B1 C1
   1    A0 PA PB PC C4
   2    D0 PD PE PF F4
   3    G0 PG PH PI I4
   4       G5 H5 I5
 */

#define XBR_LV2_4X(PE, PI, PH, PF, PG, PC, PD, PB, PA, G5, C4, G0, D0, C1, B1, F4, I4, H5, I5, A0, A1, N0, N1, N2, N3, N4, N5, N6, N7, N8, N9, N10, N11, N12, N13, N14, N15) \
	if (PE != PH && PE != PF) { \
		unsigned e = dist(PE, PC) + dist(PE, PG) + dist(PI, H5) + dist(PI, F4) + 4 * dist(PH, PF); \
		unsigned i = dist(PH, PD) + dist(PH, I5) + dist(PF, I4) + dist(PF, PB) + 4 * dist(PE, PI); \
		if (e < i) { \
			int ex2 = PE != PC && PB != PC; \
			int ex3 = PE != PG && PD != PG; \
			unsigned ke = dist(PF, PG); \
			unsigned ki = dist(PH, PC); \
			pixel_t px = dist(PE, PF) <= dist(PE, PH) ? PF : PH; \
			if (ke == 0 && ki == 0 && ex3 && ex2) { \
				LEFT_UP_2(N15, N14, N11, N13, N12, N10, N7, N3, px); \
			} else if (2 * ke <= ki && ex3) { \
				LEFT_2(N15, N14, N11, N13, N12, N10, px); \
			} else if (ke >= 2 * ki && ex2) { \
				UP_2(N15, N14, N11, N3, N7, N10, px); \
			} else { \
				DIA_2(N15, N14, N11, px); \
			} \
		} \
	}

#define LEFT_UP_2(N15, N14, N11, N13, N12, N10, N7, N3, PIXEL) \
	O[N7] = O[N13] = lerp(PIXEL, O[N13], 0.75); \
	O[N3] = O[N10] = O[N12] = lerp(O[N12], PIXEL, 0.75); \
	O[N11] = O[N14] = O[N15] = PIXEL;

#define LEFT_2(N15, N14, N11, N13, N12, N10, PIXEL) \
	O[N11] = lerp(PIXEL, O[N11], 0.75); \
	O[N13] = lerp(PIXEL, O[N13], 0.75); \
	O[N10] = lerp(O[N10], PIXEL, 0.75); \
	O[N12] = lerp(O[N12], PIXEL, 0.75); \
	O[N14] = O[N15] = PIXEL;

#define UP_2(N15, N14, N11, N3, N7, N10, PIXEL) \
	O[N14] = lerp(PIXEL, O[N14], 0.75); \
	O[N7] = lerp(PIXEL, O[N7], 0.75); \
	O[N10] = lerp(O[N10], PIXEL, 0.75); \
	O[N3] = lerp(O[N3], PIXEL, 0.75); \
	O[N11] = O[N15] = PIXEL;

#define DIA_2(N15, N14, N11, PIXEL) \
	O[N11] = lerp(O[N11], PIXEL, 0.5); \
	O[N14] = lerp(O[N14], PIXEL, 0.5); \
	O[N15] = PIXEL;


/* XBR Level 3 - 4x scale with enhanced edge detection */
#define XBR_LV3_4X(PE, PI, PH, PF, PG, PC, PD, PB, PA, G5, C4, G0, D0, C1, B1, F4, I4, H5, I5, A0, A1, N0, N1, N2, N3, N4, N5, N6, N7, N8, N9, N10, N11, N12, N13, N14, N15) \
	if (PE != PH && PE != PF) { \
		unsigned e = dist(PC, PE) + dist(PE, PG) + dist(H5, PI) + dist(PI, F4) + 4 * dist(PH, PF); \
		unsigned i = dist(PD, PH) + dist(PH, I5) + dist(I4, PF) + dist(PF, PB) + 4 * dist(PE, PI); \
		if (e < i) { \
			int ex2 = PE != PC && PB != PC; \
			int ex3 = PE != PG && PD != PG; \
			unsigned ke = dist(PF, PG); \
			unsigned ki = dist(PH, PC); \
			int left_up = ex3 && ex2; \
			int left_only = ex3 && !ex2; \
			int up_only = !ex3 && ex2; \
			pixel_t px = dist(PE, PF) <= dist(PE, PH) ? PF : PH; \
			\
			/* Enhanced blending conditions for Level 3 4x */ \
			if (left_up && (ke * 2 <= ki) && (ki * 2 <= ke)) { \
				LEFT_UP_2_4X_LV3(N0, N1, N2, N3, N4, N5, N6, N7, N8, N9, N10, N11, N12, N13, N14, N15, px, PE); \
			} else if (left_only && (ke * 2 <= ki)) { \
				LEFT_2_4X_LV3(N0, N1, N2, N3, N4, N5, N6, N7, N8, N9, N10, N11, N12, N13, N14, N15, px, PE); \
			} else if (up_only && (ki * 2 <= ke)) { \
				UP_2_4X_LV3(N0, N1, N2, N3, N4, N5, N6, N7, N8, N9, N10, N11, N12, N13, N14, N15, px, PE); \
			} else if (left_up) { \
				LEFT_UP_2_4X(N0, N1, N2, N3, N4, N5, N6, N7, N8, N9, N10, N11, N12, N13, N14, N15, px); \
			} else if (left_only) { \
				LEFT_2_4X(N0, N1, N2, N3, N4, N5, N6, N7, N8, N9, N10, N11, N12, N13, N14, N15, px); \
			} else if (up_only) { \
				UP_2_4X(N0, N1, N2, N3, N4, N5, N6, N7, N8, N9, N10, N11, N12, N13, N14, N15, px); \
			} else { \
				DIA_4X(N0, N1, N2, N3, N4, N5, N6, N7, N8, N9, N10, N11, N12, N13, N14, N15, px); \
			} \
		} \
	}

/* XBR Level 4 - 4x scale with gradient-aware interpolation */
#define XBR_LV4_4X(PE, PI, PH, PF, PG, PC, PD, PB, PA, G5, C4, G0, D0, C1, B1, F4, I4, H5, I5, A0, A1, N0, N1, N2, N3, N4, N5, N6, N7, N8, N9, N10, N11, N12, N13, N14, N15) \
	if (PE != PH && PE != PF) { \
		unsigned e = dist(PC, PE) + dist(PE, PG) + dist(H5, PI) + dist(PI, F4) + 4 * dist(PH, PF); \
		unsigned i = dist(PD, PH) + dist(PH, I5) + dist(I4, PF) + dist(PF, PB) + 4 * dist(PE, PI); \
		if (e < i) { \
			int ex2 = PE != PC && PB != PC; \
			int ex3 = PE != PG && PD != PG; \
			unsigned ke = dist(PF, PG); \
			unsigned ki = dist(PH, PC); \
			\
			/* Enhanced gradient detection for Level 4 4x */ \
			unsigned grad_h = dist(PA, PB) + dist(PB, PC) + dist(PD, PE) + dist(PE, PF) + dist(PG, PH) + dist(PH, PI); \
			unsigned grad_v = dist(A1, PA) + dist(PA, PD) + dist(B1, PB) + dist(PB, PE) + dist(C1, PC) + dist(PC, PF); \
			unsigned grad_ratio = (grad_h > grad_v) ? (grad_h * 8) / (grad_v + 1) : (grad_v * 8) / (grad_h + 1); \
			\
			int left_up = ex3 && ex2; \
			int left_only = ex3 && !ex2; \
			int up_only = !ex3 && ex2; \
			pixel_t px = dist(PE, PF) <= dist(PE, PH) ? PF : PH; \
			\
			/* Gradient-aware blending for Level 4 4x */ \
			if (left_up && grad_ratio > 6) { \
				LEFT_UP_2_4X_LV4(N0, N1, N2, N3, N4, N5, N6, N7, N8, N9, N10, N11, N12, N13, N14, N15, px, PE, grad_ratio); \
			} else if (left_only && (ke * 3 <= ki * 2) && grad_ratio > 4) { \
				LEFT_2_4X_LV4(N0, N1, N2, N3, N4, N5, N6, N7, N8, N9, N10, N11, N12, N13, N14, N15, px, PE, grad_ratio); \
			} else if (up_only && (ki * 3 <= ke * 2) && grad_ratio > 4) { \
				UP_2_4X_LV4(N0, N1, N2, N3, N4, N5, N6, N7, N8, N9, N10, N11, N12, N13, N14, N15, px, PE, grad_ratio); \
			} else if (left_up) { \
				LEFT_UP_2_4X_LV3(N0, N1, N2, N3, N4, N5, N6, N7, N8, N9, N10, N11, N12, N13, N14, N15, px, PE); \
			} else if (left_only) { \
				LEFT_2_4X_LV3(N0, N1, N2, N3, N4, N5, N6, N7, N8, N9, N10, N11, N12, N13, N14, N15, px, PE); \
			} else if (up_only) { \
				UP_2_4X_LV3(N0, N1, N2, N3, N4, N5, N6, N7, N8, N9, N10, N11, N12, N13, N14, N15, px, PE); \
			} else { \
				DIA_4X_LV4(N0, N1, N2, N3, N4, N5, N6, N7, N8, N9, N10, N11, N12, N13, N14, N15, px, PE); \
			} \
		} \
	}

#define LEFT_UP_2_4X(N0, N1, N2, N3, N4, N5, N6, N7, N8, N9, N10, N11, N12, N13, N14, N15, PIXEL) \
	O[N0] = lerp(PIXEL, O[N0], 7.0 / 8); \
	O[N1] = lerp(PIXEL, O[N1], 3.0 / 4); \
	O[N4] = lerp(PIXEL, O[N4], 3.0 / 4); \
	O[N5] = lerp(O[N5], PIXEL, 1.0 / 2); \
	O[N2] = lerp(O[N2], PIXEL, 1.0 / 4); \
	O[N3] = lerp(O[N3], PIXEL, 1.0 / 8); \
	O[N8] = lerp(O[N8], PIXEL, 1.0 / 4); \
	O[N12] = lerp(O[N12], PIXEL, 1.0 / 8);

#define LEFT_2_4X(N0, N1, N2, N3, N4, N5, N6, N7, N8, N9, N10, N11, N12, N13, N14, N15, PIXEL) \
	O[N0] = lerp(PIXEL, O[N0], 3.0 / 4); \
	O[N1] = lerp(O[N1], PIXEL, 1.0 / 2); \
	O[N4] = lerp(PIXEL, O[N4], 3.0 / 4); \
	O[N5] = lerp(O[N5], PIXEL, 1.0 / 2); \
	O[N8] = lerp(O[N8], PIXEL, 1.0 / 4); \
	O[N9] = lerp(O[N9], PIXEL, 1.0 / 8); \
	O[N12] = lerp(O[N12], PIXEL, 1.0 / 4); \
	O[N13] = lerp(O[N13], PIXEL, 1.0 / 8);

#define UP_2_4X(N0, N1, N2, N3, N4, N5, N6, N7, N8, N9, N10, N11, N12, N13, N14, N15, PIXEL) \
	O[N0] = lerp(PIXEL, O[N0], 3.0 / 4); \
	O[N1] = lerp(PIXEL, O[N1], 3.0 / 4); \
	O[N4] = lerp(O[N4], PIXEL, 1.0 / 2); \
	O[N5] = lerp(O[N5], PIXEL, 1.0 / 2); \
	O[N2] = lerp(O[N2], PIXEL, 1.0 / 4); \
	O[N6] = lerp(O[N6], PIXEL, 1.0 / 8); \
	O[N3] = lerp(O[N3], PIXEL, 1.0 / 4); \
	O[N7] = lerp(O[N7], PIXEL, 1.0 / 8);

#define DIA_4X(N0, N1, N2, N3, N4, N5, N6, N7, N8, N9, N10, N11, N12, N13, N14, N15, PIXEL) \
	O[N0] = lerp(O[N0], PIXEL, 1.0 / 2); \
	O[N1] = lerp(O[N1], PIXEL, 3.0 / 8); \
	O[N4] = lerp(O[N4], PIXEL, 3.0 / 8); \
	O[N5] = lerp(O[N5], PIXEL, 1.0 / 4);

/* Level 3 enhanced 4x interpolation macros */
#define LEFT_UP_2_4X_LV3(N0, N1, N2, N3, N4, N5, N6, N7, N8, N9, N10, N11, N12, N13, N14, N15, PIXEL, PE) \
	O[N0] = lerp(PIXEL, O[N0], 6.0 / 8); \
	O[N1] = lerp(PIXEL, O[N1], 5.0 / 8); \
	O[N4] = lerp(PIXEL, O[N4], 5.0 / 8); \
	O[N5] = lerp(O[N5], PIXEL, 3.0 / 8); \
	O[N2] = lerp(O[N2], PIXEL, 2.0 / 8); \
	O[N3] = lerp(O[N3], PIXEL, 1.0 / 8); \
	O[N8] = lerp(O[N8], PIXEL, 2.0 / 8); \
	O[N9] = lerp(O[N9], PIXEL, 1.0 / 8); \
	O[N6] = lerp(O[N6], PIXEL, 1.0 / 8); \
	O[N10] = lerp(O[N10], PIXEL, 1.0 / 16); \
	O[N12] = lerp(O[N12], PIXEL, 1.0 / 8); \
	O[N13] = lerp(O[N13], PIXEL, 1.0 / 16);

#define LEFT_2_4X_LV3(N0, N1, N2, N3, N4, N5, N6, N7, N8, N9, N10, N11, N12, N13, N14, N15, PIXEL, PE) \
	O[N0] = lerp(PIXEL, O[N0], 5.0 / 8); \
	O[N1] = lerp(O[N1], PIXEL, 3.0 / 8); \
	O[N4] = lerp(PIXEL, O[N4], 5.0 / 8); \
	O[N5] = lerp(O[N5], PIXEL, 3.0 / 8); \
	O[N8] = lerp(O[N8], PIXEL, 2.0 / 8); \
	O[N9] = lerp(O[N9], PIXEL, 1.0 / 8); \
	O[N12] = lerp(O[N12], PIXEL, 2.0 / 8); \
	O[N13] = lerp(O[N13], PIXEL, 1.0 / 8); \
	O[N2] = lerp(O[N2], PIXEL, 1.0 / 8); \
	O[N6] = lerp(O[N6], PIXEL, 1.0 / 16); \
	O[N10] = lerp(O[N10], PIXEL, 1.0 / 16); \
	O[N14] = lerp(O[N14], PIXEL, 1.0 / 32);

#define UP_2_4X_LV3(N0, N1, N2, N3, N4, N5, N6, N7, N8, N9, N10, N11, N12, N13, N14, N15, PIXEL, PE) \
	O[N0] = lerp(PIXEL, O[N0], 5.0 / 8); \
	O[N1] = lerp(PIXEL, O[N1], 5.0 / 8); \
	O[N4] = lerp(O[N4], PIXEL, 3.0 / 8); \
	O[N5] = lerp(O[N5], PIXEL, 3.0 / 8); \
	O[N2] = lerp(O[N2], PIXEL, 2.0 / 8); \
	O[N6] = lerp(O[N6], PIXEL, 1.0 / 8); \
	O[N3] = lerp(O[N3], PIXEL, 2.0 / 8); \
	O[N7] = lerp(O[N7], PIXEL, 1.0 / 8); \
	O[N8] = lerp(O[N8], PIXEL, 1.0 / 8); \
	O[N9] = lerp(O[N9], PIXEL, 1.0 / 16); \
	O[N10] = lerp(O[N10], PIXEL, 1.0 / 16); \
	O[N11] = lerp(O[N11], PIXEL, 1.0 / 32);

/* Level 4 gradient-aware 4x interpolation macros */
#define LEFT_UP_2_4X_LV4(N0, N1, N2, N3, N4, N5, N6, N7, N8, N9, N10, N11, N12, N13, N14, N15, PIXEL, PE, GRAD) \
	{ \
		if (GRAD > 12) { \
			O[N0] = lerp(PIXEL, O[N0], 7.0 / 8); \
			O[N1] = lerp(PIXEL, O[N1], 6.0 / 8); \
			O[N4] = lerp(PIXEL, O[N4], 6.0 / 8); \
			O[N5] = lerp(O[N5], PIXEL, 2.0 / 8); \
			O[N2] = lerp(O[N2], PIXEL, 3.0 / 8); \
			O[N3] = lerp(O[N3], PIXEL, 2.0 / 8); \
			O[N8] = lerp(O[N8], PIXEL, 3.0 / 8); \
			O[N9] = lerp(O[N9], PIXEL, 2.0 / 8); \
			O[N6] = lerp(O[N6], PIXEL, 2.0 / 8); \
			O[N10] = lerp(O[N10], PIXEL, 1.0 / 8); \
			O[N12] = lerp(O[N12], PIXEL, 2.0 / 8); \
			O[N13] = lerp(O[N13], PIXEL, 1.0 / 8); \
		} else if (GRAD > 8) { \
			O[N0] = lerp(PIXEL, O[N0], 6.0 / 8); \
			O[N1] = lerp(PIXEL, O[N1], 5.0 / 8); \
			O[N4] = lerp(PIXEL, O[N4], 5.0 / 8); \
			O[N5] = lerp(O[N5], PIXEL, 3.0 / 8); \
			O[N2] = lerp(O[N2], PIXEL, 2.0 / 8); \
			O[N3] = lerp(O[N3], PIXEL, 1.0 / 8); \
			O[N8] = lerp(O[N8], PIXEL, 2.0 / 8); \
			O[N9] = lerp(O[N9], PIXEL, 1.0 / 8); \
			O[N6] = lerp(O[N6], PIXEL, 1.0 / 8); \
			O[N10] = lerp(O[N10], PIXEL, 1.0 / 16); \
			O[N12] = lerp(O[N12], PIXEL, 1.0 / 8); \
			O[N13] = lerp(O[N13], PIXEL, 1.0 / 16); \
		} else { \
			LEFT_UP_2_4X_LV3(N0, N1, N2, N3, N4, N5, N6, N7, N8, N9, N10, N11, N12, N13, N14, N15, PIXEL, PE); \
		} \
	}

#define LEFT_2_4X_LV4(N0, N1, N2, N3, N4, N5, N6, N7, N8, N9, N10, N11, N12, N13, N14, N15, PIXEL, PE, GRAD) \
	{ \
		if (GRAD > 12) { \
			O[N0] = lerp(PIXEL, O[N0], 6.0 / 8); \
			O[N1] = lerp(O[N1], PIXEL, 2.0 / 8); \
			O[N4] = lerp(PIXEL, O[N4], 6.0 / 8); \
			O[N5] = lerp(O[N5], PIXEL, 2.0 / 8); \
			O[N8] = lerp(O[N8], PIXEL, 3.0 / 8); \
			O[N9] = lerp(O[N9], PIXEL, 2.0 / 8); \
			O[N12] = lerp(O[N12], PIXEL, 3.0 / 8); \
			O[N13] = lerp(O[N13], PIXEL, 2.0 / 8); \
		} else if (GRAD > 8) { \
			O[N0] = lerp(PIXEL, O[N0], 5.0 / 8); \
			O[N1] = lerp(O[N1], PIXEL, 3.0 / 8); \
			O[N4] = lerp(PIXEL, O[N4], 5.0 / 8); \
			O[N5] = lerp(O[N5], PIXEL, 3.0 / 8); \
			O[N8] = lerp(O[N8], PIXEL, 2.0 / 8); \
			O[N9] = lerp(O[N9], PIXEL, 1.0 / 8); \
			O[N12] = lerp(O[N12], PIXEL, 2.0 / 8); \
			O[N13] = lerp(O[N13], PIXEL, 1.0 / 8); \
		} else { \
			LEFT_2_4X_LV3(N0, N1, N2, N3, N4, N5, N6, N7, N8, N9, N10, N11, N12, N13, N14, N15, PIXEL, PE); \
		} \
	}

#define UP_2_4X_LV4(N0, N1, N2, N3, N4, N5, N6, N7, N8, N9, N10, N11, N12, N13, N14, N15, PIXEL, PE, GRAD) \
	{ \
		if (GRAD > 12) { \
			O[N0] = lerp(PIXEL, O[N0], 6.0 / 8); \
			O[N1] = lerp(PIXEL, O[N1], 6.0 / 8); \
			O[N4] = lerp(O[N4], PIXEL, 2.0 / 8); \
			O[N5] = lerp(O[N5], PIXEL, 2.0 / 8); \
			O[N2] = lerp(O[N2], PIXEL, 3.0 / 8); \
			O[N6] = lerp(O[N6], PIXEL, 2.0 / 8); \
			O[N3] = lerp(O[N3], PIXEL, 3.0 / 8); \
			O[N7] = lerp(O[N7], PIXEL, 2.0 / 8); \
		} else if (GRAD > 8) { \
			O[N0] = lerp(PIXEL, O[N0], 5.0 / 8); \
			O[N1] = lerp(PIXEL, O[N1], 5.0 / 8); \
			O[N4] = lerp(O[N4], PIXEL, 3.0 / 8); \
			O[N5] = lerp(O[N5], PIXEL, 3.0 / 8); \
			O[N2] = lerp(O[N2], PIXEL, 2.0 / 8); \
			O[N6] = lerp(O[N6], PIXEL, 1.0 / 8); \
			O[N3] = lerp(O[N3], PIXEL, 2.0 / 8); \
			O[N7] = lerp(O[N7], PIXEL, 1.0 / 8); \
		} else { \
			UP_2_4X_LV3(N0, N1, N2, N3, N4, N5, N6, N7, N8, N9, N10, N11, N12, N13, N14, N15, PIXEL, PE); \
		} \
	}

#define DIA_4X_LV4(N0, N1, N2, N3, N4, N5, N6, N7, N8, N9, N10, N11, N12, N13, N14, N15, PIXEL, PE) \
	O[N0] = lerp(O[N0], PIXEL, 3.0 / 8); \
	O[N1] = lerp(O[N1], PIXEL, 2.0 / 8); \
	O[N4] = lerp(O[N4], PIXEL, 2.0 / 8); \
	O[N5] = lerp(O[N5], PIXEL, 1.0 / 8);

			case 3 :
				/* xbr level 2 */
				O[0] = O[1] = O[2] = O[3] = O[4] = O[5] = O[6] = O[7] = O[8] = O[9] = O[10] = O[11] = O[12] = O[13] = O[14] = O[15] = E;

				XBR_LV2_4X(E, I, H, F, G, C, D, B, A, G5, C4, G0, D0, C1, B1, F4, I4, H5, I5, A0, A1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
				XBR_LV2_4X(E, C, F, B, I, A, H, D, G, I4, A1, I5, H5, A0, D0, B1, C1, F4, C4, G5, G0, 12, 8, 4, 0, 13, 9, 5, 1, 14, 10, 6, 2, 15, 11, 7, 3);
				XBR_LV2_4X(E, A, B, D, C, G, F, H, I, C1, G0, C4, F4, G5, H5, D0, A0, B1, A1, I4, I5, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0);
				XBR_LV2_4X(E, G, D, H, A, I, B, F, C, A0, I5, A1, B1, I4, F4, H5, G5, D0, G0, C1, C4, 3, 7, 11, 15, 2, 6, 10, 14, 1, 5, 9, 13, 0, 4, 8, 12);

				break;
			case 4 :
				/* xbr level 3 */
				O[0] = O[1] = O[2] = O[3] = O[4] = O[5] = O[6] = O[7] = O[8] = O[9] = O[10] = O[11] = O[12] = O[13] = O[14] = O[15] = E;

				XBR_LV3_4X(E, I, H, F, G, C, D, B, A, G5, C4, G0, D0, C1, B1, F4, I4, H5, I5, A0, A1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
				XBR_LV3_4X(E, C, F, B, I, A, H, D, G, I4, A1, I5, H5, A0, D0, B1, C1, F4, C4, G5, G0, 12, 8, 4, 0, 13, 9, 5, 1, 14, 10, 6, 2, 15, 11, 7, 3);
				XBR_LV3_4X(E, A, B, D, C, G, F, H, I, C1, G0, C4, F4, G5, H5, D0, A0, B1, A1, I4, I5, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0);
				XBR_LV3_4X(E, G, D, H, A, I, B, F, C, A0, I5, A1, B1, I4, F4, H5, G5, D0, G0, C1, C4, 3, 7, 11, 15, 2, 6, 10, 14, 1, 5, 9, 13, 0, 4, 8, 12);
				break;
			case 5 :
				/* xbr level 4 */
				O[0] = O[1] = O[2] = O[3] = O[4] = O[5] = O[6] = O[7] = O[8] = O[9] = O[10] = O[11] = O[12] = O[13] = O[14] = O[15] = E;

				XBR_LV4_4X(E, I, H, F, G, C, D, B, A, G5, C4, G0, D0, C1, B1, F4, I4, H5, I5, A0, A1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
				XBR_LV4_4X(E, C, F, B, I, A, H, D, G, I4, A1, I5, H5, A0, D0, B1, C1, F4, C4, G5, G0, 12, 8, 4, 0, 13, 9, 5, 1, 14, 10, 6, 2, 15, 11, 7, 3);
				XBR_LV4_4X(E, A, B, D, C, G, F, H, I, C1, G0, C4, F4, G5, H5, D0, A0, B1, A1, I4, I5, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0);
				XBR_LV4_4X(E, G, D, H, A, I, B, F, C, A0, I5, A1, B1, I4, F4, H5, G5, D0, G0, C1, C4, 3, 7, 11, 15, 2, 6, 10, 14, 1, 5, 9, 13, 0, 4, 8, 12);
				break;						
			}

			pixel_put(x * 4, y * 4, dst_ptr, dst_slice, pixel, width * 4, height * 4, O[0]);
			pixel_put(x * 4 + 1, y * 4, dst_ptr, dst_slice, pixel, width * 4, height * 4, O[1]);
			pixel_put(x * 4 + 2, y * 4, dst_ptr, dst_slice, pixel, width * 4, height * 4, O[2]);
			pixel_put(x * 4 + 3, y * 4, dst_ptr, dst_slice, pixel, width * 4, height * 4, O[3]);
			pixel_put(x * 4, y * 4 + 1, dst_ptr, dst_slice, pixel, width * 4, height * 4, O[4]);
			pixel_put(x * 4 + 1, y * 4 + 1, dst_ptr, dst_slice, pixel, width * 4, height * 4, O[5]);
			pixel_put(x * 4 + 2, y * 4 + 1, dst_ptr, dst_slice, pixel, width * 4, height * 4, O[6]);
			pixel_put(x * 4 + 3, y * 4 + 1, dst_ptr, dst_slice, pixel, width * 4, height * 4, O[7]);
			pixel_put(x * 4, y * 4 + 2, dst_ptr, dst_slice, pixel, width * 4, height * 4, O[8]);
			pixel_put(x * 4 + 1, y * 4 + 2, dst_ptr, dst_slice, pixel, width * 4, height * 4, O[9]);
			pixel_put(x * 4 + 2, y * 4 + 2, dst_ptr, dst_slice, pixel, width * 4, height * 4, O[10]);
			pixel_put(x * 4 + 3, y * 4 + 2, dst_ptr, dst_slice, pixel, width * 4, height * 4, O[11]);
			pixel_put(x * 4, y * 4 + 3, dst_ptr, dst_slice, pixel, width * 4, height * 4, O[12]);
			pixel_put(x * 4 + 1, y * 4 + 3, dst_ptr, dst_slice, pixel, width * 4, height * 4, O[13]);
			pixel_put(x * 4 + 2, y * 4 + 3, dst_ptr, dst_slice, pixel, width * 4, height * 4, O[14]);
			pixel_put(x * 4 + 3, y * 4 + 3, dst_ptr, dst_slice, pixel, width * 4, height * 4, O[15]);
		}
	}

	return 0;
}

#define SCALE2X3_REVISION_MAX 1

void scale2x3(unsigned char* dst_ptr, unsigned dst_slice, const unsigned char* src_ptr, unsigned src_slice, unsigned pixel, int width, int height, int opt_tes, int opt_ver)
{
	int x;
	int y;

	for (y = 0; y < height; ++y) {
		for (x = 0; x < width; ++x) {
			pixel_t O[6];
			pixel_t A, B, C, D, E, F, G, H, I;
			int k0, k1, k2, k3;

/*
        ABC
        DEF
        GHI

        O0O1
        O2O3
        O4O5
 */

			A = pixel_get(x - 1, y - 1, src_ptr, src_slice, pixel, width, height, opt_tes);
			B = pixel_get(x, y - 1, src_ptr, src_slice, pixel, width, height, opt_tes);
			C = pixel_get(x + 1, y - 1, src_ptr, src_slice, pixel, width, height, opt_tes);
			D = pixel_get(x - 1, y, src_ptr, src_slice, pixel, width, height, opt_tes);
			E = pixel_get(x, y, src_ptr, src_slice, pixel, width, height, opt_tes);
			F = pixel_get(x + 1, y, src_ptr, src_slice, pixel, width, height, opt_tes);
			G = pixel_get(x - 1, y + 1, src_ptr, src_slice, pixel, width, height, opt_tes);
			H = pixel_get(x, y + 1, src_ptr, src_slice, pixel, width, height, opt_tes);
			I = pixel_get(x + 1, y + 1, src_ptr, src_slice, pixel, width, height, opt_tes);

			switch (opt_ver) {
			default :
			case 0 :
				/* version 0, normal scaling */
				O[0] = E;
				O[1] = E;
				O[2] = E;
				O[3] = E;
				O[4] = E;
				O[5] = E;
				break;
			case 1 :
				/* default */
				k0 = D == B && B != F && D != H;
				k1 = B == F && B != D && F != H;
				k2 = D == H && D != B && H != F;
				k3 = H == F && D != H && B != F;
				O[0] = k0 ? D : E;
				O[1] = k1 ? F : E;
				O[2] = (k0 && E != G) || (k2 && E != A) ? D : E;
				O[3] = (k1 && E != I) || (k3 && E != C) ? F : E;
				O[4] = k2 ? D : E;
				O[5] = k3 ? F : E;
				break;
			}

			pixel_put(x * 2, y * 3, dst_ptr, dst_slice, pixel, width * 2, height * 3, O[0]);
			pixel_put(x * 2 + 1, y * 3, dst_ptr, dst_slice, pixel, width * 2, height * 3, O[1]);
			pixel_put(x * 2, y * 3 + 1, dst_ptr, dst_slice, pixel, width * 2, height * 3, O[2]);
			pixel_put(x * 2 + 1, y * 3 + 1, dst_ptr, dst_slice, pixel, width * 2, height * 3, O[3]);
			pixel_put(x * 2, y * 3 + 2, dst_ptr, dst_slice, pixel, width * 2, height * 3, O[4]);
			pixel_put(x * 2 + 1, y * 3 + 2, dst_ptr, dst_slice, pixel, width * 2, height * 3, O[5]);
		}
	}
}

#define SCALE2X4_REVISION_MAX 2

void scale2x4(unsigned char* dst_ptr, unsigned dst_slice, const unsigned char* src_ptr, unsigned src_slice, unsigned pixel, int width, int height, int opt_tes, int opt_ver)
{
	int x;
	int y;

	for (y = 0; y < height; ++y) {
		for (x = 0; x < width; ++x) {
			pixel_t O[8];
			pixel_t A, B, C, D, E, F, G, H, I;
			int k0, k1, k2, k3;

/*
        ABC
        DEF
        GHI

        O0O1
        O2O3
        O4O5
        O6O7
*/

			A = pixel_get(x - 1, y - 1, src_ptr, src_slice, pixel, width, height, opt_tes);
			B = pixel_get(x, y - 1, src_ptr, src_slice, pixel, width, height, opt_tes);
			C = pixel_get(x + 1, y - 1, src_ptr, src_slice, pixel, width, height, opt_tes);
			D = pixel_get(x - 1, y, src_ptr, src_slice, pixel, width, height, opt_tes);
			E = pixel_get(x, y, src_ptr, src_slice, pixel, width, height, opt_tes);
			F = pixel_get(x + 1, y, src_ptr, src_slice, pixel, width, height, opt_tes);
			G = pixel_get(x - 1, y + 1, src_ptr, src_slice, pixel, width, height, opt_tes);
			H = pixel_get(x, y + 1, src_ptr, src_slice, pixel, width, height, opt_tes);
			I = pixel_get(x + 1, y + 1, src_ptr, src_slice, pixel, width, height, opt_tes);

			switch (opt_ver) {
			default :
			case 0 :
				/* version 0, normal scaling */
				O[0] = E;
				O[1] = E;
				O[2] = E;
				O[3] = E;
				O[4] = E;
				O[5] = E;
				O[6] = E;
				O[7] = E;
				break;
			case 1 :
				/* default */
				k0 = D == B && B != F && D != H;
				k1 = B == F && B != D && F != H;
				k2 = D == H && D != B && H != F;
				k3 = H == F && D != H && B != F;
				O[0] = k0 ? D : E;
				O[1] = k1 ? F : E;
				O[2] = (k0 && E != G) || (k2 && E != A) ? D : E;
				O[3] = (k1 && E != I) || (k3 && E != C) ? F : E;
				O[4] = (k0 && E != G) || (k2 && E != A) ? D : E;
				O[5] = (k1 && E != I) || (k3 && E != C) ? F : E;
				O[6] = k2 ? D : E;
				O[7] = k3 ? F : E;
				break;
			case 2 :
				/* Hans de Goede variation, rejected */
				k0 = D == B && B != F && D != H;
				k1 = B == F && B != D && F != H;
				k2 = D == H && D != B && H != F;
				k3 = H == F && D != H && B != F;
				O[0] = k0 ? D : E;
				O[1] = k1 ? F : E;
				O[2] = k0 ? D : E;
				O[3] = k1 ? F : E;
				O[4] = k2 ? D : E;
				O[5] = k3 ? F : E;
				O[6] = k2 ? D : E;
				O[7] = k3 ? F : E;
				break;
			}

			pixel_put(x * 2, y * 4, dst_ptr, dst_slice, pixel, width * 2, height * 4, O[0]);
			pixel_put(x * 2 + 1, y * 4, dst_ptr, dst_slice, pixel, width * 2, height * 4, O[1]);
			pixel_put(x * 2, y * 4 + 1, dst_ptr, dst_slice, pixel, width * 2, height * 4, O[2]);
			pixel_put(x * 2 + 1, y * 4 + 1, dst_ptr, dst_slice, pixel, width * 2, height * 4, O[3]);
			pixel_put(x * 2, y * 4 + 2, dst_ptr, dst_slice, pixel, width * 2, height * 4, O[4]);
			pixel_put(x * 2 + 1, y * 4 + 2, dst_ptr, dst_slice, pixel, width * 2, height * 4, O[5]);
			pixel_put(x * 2, y * 4 + 3, dst_ptr, dst_slice, pixel, width * 2, height * 4, O[6]);
			pixel_put(x * 2 + 1, y * 4 + 3, dst_ptr, dst_slice, pixel, width * 2, height * 4, O[7]);
		}
	}
}

int file_process(const char* file0, const char* file1, int opt_scale_x, int opt_scale_y, int opt_tes, int opt_ver, int opt_crc, int opt_only124)
{
	unsigned pixel;
	unsigned width;
	unsigned height;
	void* src_alloc;
	unsigned char* src_ptr;
	unsigned src_slice;
	unsigned char* dst_ptr;
	unsigned dst_slice;
	int type;
	int channel;
	png_color* palette;
	unsigned palette_size;

	if (file_read(file0, &src_alloc, &src_ptr, &src_slice, &pixel, &width, &height, &type, &channel, &palette, &palette_size, opt_only124 ? 1 : 0) != 0) {
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
	default :
		scalex(dst_ptr, dst_slice, src_ptr, src_slice, pixel, width, height, opt_scale_x, opt_scale_y);
		break;
	}

	if (file_write(file1, dst_ptr, dst_slice, pixel, width * opt_scale_x, height * opt_scale_y, type, channel, palette, palette_size) != 0) {
		goto err_dst;
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

	free(dst_ptr);
	free(src_alloc);
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

int file_gen(void)
{
	unsigned width;
	unsigned height;
	unsigned slice;
	unsigned pixel;
	unsigned type;
	unsigned channel;
	unsigned char* ptr;
	unsigned x, y, i, j;
	unsigned delta;
	unsigned skew;

	delta = 5;
	skew = 1;
	pixel = 3;
	width = 32 * delta;
	height = 16 * delta;
	type = PNG_COLOR_TYPE_RGB;
	channel = 3;

	slice = width * pixel;
	ptr = malloc(slice * height);
	if (!ptr) {
		fprintf(stderr, "Low memory.\n");
		goto err;
	}

	/* red background */
	for (y = 0; y < height; ++y) {
		for (x = 0; x < width; ++x) {
			pixel_put(x, y, ptr, slice, pixel, width, height, 0x0000FF);
		}
	}

	for (y = 0; y < 16; ++y) {
		for (x = 0; x < 32; ++x) {
			unsigned mask = (y << 5) | x;
			for (j = 0; j < 3; ++j) {
				for (i = 0; i < 3; ++i) {
					unsigned index = j * 3 + i;
					unsigned color;
					if ((mask & (1 << index)) != 0) {
						color = 0xFFFFFF;
					} else {
						color = 0x000000;
					}
					pixel_put(x * delta + skew + i, y * delta + skew + j, ptr, slice, pixel, width, height, color);
				}
			}
		}
	}

	if (file_write("template.png", ptr, slice, pixel, width, height, type, channel, 0, 0) != 0) {
		goto err_ptr;
	}

	return 0;

err_ptr:
	free(ptr);
err:
	return -1;
}

void version(void)
{
	printf(PACKAGE " v" VERSION " by Andrea Mazzoleni, " PACKAGE_URL "\n");
}

void usage(void)
{
	version();
	printf("Reference implementation of the Scale2/3/4x effect\n");
	printf("\nSyntax: scalerx [-k N] [-w] [-r N] FROM.png TO.png\n");
	printf("\nOptions:\n");
	printf("\t-k N\tSelect the scale factor. 2, 2x3, 2x4, 3, 4 (default 2).\n");
	printf("\t-w\tWrap around on the borders.\n");
	printf("\t-r N\tSelect the revision of the algorithm 0-N (default 1).\n");
	printf("\nMore info at " PACKAGE_URL "\n");
	exit(EXIT_FAILURE);
}

#ifdef HAVE_GETOPT_LONG
struct option long_options[] = {
	{ "scale", 1, 0, 'k' },
	{ "crc", 0, 0, 'c' },
	{ "only124", 0, 0, 'o' },
	{ "wrap", 0, 0, 'w' },
	{ "revision", 1, 0, 'r' },
	{ "help", 0, 0, 'h' },
	{ "version", 0, 0, 'v' },
	{ 0, 0, 0, 0 }
};
#endif

#define OPTIONS "k:cowr:ghv"

int main(int argc, char* argv[])
{
	int opt_scale_x = 2;
	int opt_scale_y = 2;
	int opt_crc = 0;
	int opt_tes = 0;
	int opt_ver = 1;
	int opt_gen = 0;
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
			if (strcmp(optarg, "x") == 0)
				opt_ver = 1;
			else if (strcmp(optarg, "k") == 0)
				opt_ver = 2;
			else if (strcmp(optarg, "xbr") == 0 || strcmp(optarg, "xbr_l2") == 0)
				opt_ver = 3;
			else if (strcmp(optarg, "xbr_l3") == 0)
				opt_ver = 4;
			else if (strcmp(optarg, "xbr_l4") == 0)
				opt_ver = 5;
			else {
				char *e;
				opt_ver = strtol(optarg, &e, 10);
				if (*e != 0) {
					printf("Invalid -r option. Valid values are x, k, xbr, xbr_l3, xbr_l4.\n");
					exit(EXIT_FAILURE);
				}
			}
			break;
		case 'g' :
			opt_gen = 1;
			break;
		default :
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

	if (opt_gen) {
		if (optind != argc) {
			usage();
			exit(EXIT_FAILURE);
		}

		file_gen();
	} else {
		if (optind + 2 != argc) {
			usage();
			exit(EXIT_FAILURE);
		}

		if (opt_crc) {
			const char* base = strrchr(argv[optind], '/');
			if (!base)
				base = argv[optind];
			else
				++base;
			printf("%s %dx%d", base, opt_scale_x, opt_scale_y);
			switch (opt_ver) {
			case 0 : printf(" 0\n"); break;
			case 1 : printf(" x\n"); break;
			case 2 : printf(" k\n"); break;
			default : printf(" %d\n", opt_ver); break;
			}
		}

		if (file_process(argv[optind], argv[optind + 1], opt_scale_x, opt_scale_y, opt_tes, opt_ver, opt_crc, opt_only124) != 0) {
			exit(EXIT_FAILURE);
		}
	}

	return EXIT_SUCCESS;
}

