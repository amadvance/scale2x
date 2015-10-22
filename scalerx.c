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

#define SCALE2X_REVISION_MAX 5

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

	r0 = r0 * f + r1 * (1.0-f);
	g0 = g0 * f + g1 * (1.0-f);
	b0 = b0 * f + b1 * (1.0-f);

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
				/* scalek */

#define SCALE2K_BASE(A,B,C,D,E,F,G,H,I,E0,E1,E2,E3) \
	if (D == B && B != E && D != E) { \
		/* diagonal */ \
		if (B == C && D == G) { \
			/* square block */ \
		} else if (B == C) { \
			/* horizontal slope */ \
			E0 = lerp(D, E0, 0.75); \
			E1 = lerp(D, E1, 0.25); \
		} else if (D == G) { \
			/* vertical slope */ \
			E0 = lerp(D, E0, 0.75); \
			E2 = lerp(D, E2, 0.25); \
		} else { \
			/* pure diagonal */ \
			E0 = lerp(E0,D,0.5); \
		} \
	}

/* Used by AdvanceMAME */
#define SCALE2K(A,B,C,D,E,F,G,H,I,E0,E1,E2,E3) \
	if (D == B && B != E && D != E) { \
		/* diagonal */ \
		if (B == C && D == G) { \
			/* square block */ \
			if (A != E) { \
				/* no star */ \
				E0 = lerp(D, E0, 0.75); \
				E1 = lerp(D, E1, 0.25); \
				E2 = lerp(D, E2, 0.25); \
			} \
		} else if (B == C && C != F) { \
			/* horizontal slope */ \
			E0 = lerp(D, E0, 0.75); \
			E1 = lerp(D, E1, 0.25); \
		} else if (D == G && G != H) { \
			/* vertical slope */ \
			E0 = lerp(D, E0, 0.75); \
			E2 = lerp(D, E2, 0.25); \
		} else { \
			/* pure diagonal */ \
			E0 = lerp(E0,D,0.5); \
		} \
	}

#define SCALE2K_SPIKE(A,B,C,D,E,F,G,H,I,E0,E1,E2,E3) \
	if (B != E && D != E) { \
		if (D == B) { \
			/* diagonal */ \
			if (B == C && D == G) { \
				/* square block */ \
			} else if (B == C) { \
				/* horizontal slope */ \
				E0 = lerp(D, E0, 0.75); \
				E1 = lerp(D, E1, 0.25); \
			} else if (D == G) { \
				/* vertical slope */ \
				E0 = lerp(D, E0, 0.75); \
				E2 = lerp(D, E2, 0.25); \
			} else { \
				/* pure diagonal */ \
				E0 = lerp(E0,D,0.5); \
			} \
		} else if (A == B && G == E) { \
			/* horizontal spike */ \
			E0 = lerp(D, E0, 0.5); \
		} else if (A == D && C == E) { \
			/* vertical spike */ \
			E0 = lerp(B, E0, 0.5); \
		} \
	}

#define SCALE2K_ALTERNATE(A,B,C,D,E,F,G,H,I,E0,E1,E2,E3) \
	if (D == B && B != F && D != H) { \
		/* diagonal */ \
		if (B == C && D == G) { \
			/* square block */ \
			if (A != E) { \
				E0 = lerp(D, E0, 0.75); \
				E1 = lerp(D, E1, 0.25); \
				E2 = lerp(D, E2, 0.25); \
			} \
		} else if (B == C && C != F) { \
			/* horizontal slope */ \
			E0 = lerp(D, E0, 0.75); \
			E1 = lerp(D, E1, 0.25); \
		} else if (D == G && G != H) { \
			/* vertical slope */ \
			E0 = lerp(D, E0, 0.75); \
			E2 = lerp(D, E2, 0.25); \
		} else { \
			/* pure diagonal */ \
			E0 = lerp(E0,D,0.5); \
		} \
	}

				E0 = E1 = E2 = E3 = E;

				SCALE2K(A,B,C,D,E,F,G,H,I,E0,E1,E2,E3);
				SCALE2K(G,D,A,H,E,B,I,F,C,E2,E0,E3,E1);
				SCALE2K(I,H,G,F,E,D,C,B,A,E3,E2,E1,E0);
				SCALE2K(C,F,I,B,E,H,A,D,G,E1,E3,E0,E2);

				break;

			case 3 :
				/* rejected */
				E0 = D == B && ((B != F && D != H) || D == A) ? D : E;
				E1 = B == F && ((B != D && F != H) || B == C) ? F : E;
				E2 = D == H && ((D != B && H != F) || D == G) ? D : E;
				E3 = H == F && ((D != H && B != F) || H == I) ? F : E;
				break;
			case 4 :
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

void scale3x(unsigned char* dst_ptr, unsigned dst_slice, const unsigned char* src_ptr, unsigned src_slice, unsigned pixel, int width, int height, int opt_tes, int opt_ver)
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
				/* default */
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

			case 2:
				/* scalek */

			/*
				ABC
				DEF
				GHI

				E0E1E2
				E3E4E5
				E6E7E8
			*/

#define SCALE3K_BASE(A,B,C,D,E,F,G,H,I,E0,E1,E2,E3,E4,E5,E6,E7,E8) \
	if (D == B && B != E && D != E) { \
		/* diagonal */ \
		if (B == C && D == G) { \
			/* square block */ \
		} else if (B == C) { \
			/* horizontal slope */ \
			E0 = D; \
			E1 = lerp(D, E1, 0.75); \
			E2 = lerp(D, E2, 0.25); \
			E3 = lerp(D, E3, 0.25); \
		} else if (D == G) { \
			/* vertical slope */ \
			E0 = D; \
			E3 = lerp(D, E3, 0.75); \
			E6 = lerp(D, E6, 0.25); \
			E1 = lerp(D, E1, 0.25); \
		} else { \
			/* pure diagonal */ \
			E0 = lerp(D, E0, 0.875); \
			E1 = lerp(D, E1, 0.125); \
			E3 = lerp(D, E3, 0.125); \
		} \
	}

/* Used by AdvanceMAME */
#define SCALE3K(A,B,C,D,E,F,G,H,I,E0,E1,E2,E3,E4,E5,E6,E7,E8) \
	if (D == B && B != E && D != E) { \
		/* diagonal */ \
		if (B == C && D == G) { \
			/* square block */ \
			if (A != E) { \
				E0 = D; \
				E1 = lerp(D, E1, 0.75); \
				E2 = lerp(D, E2, 0.25); \
				E3 = lerp(D, E3, 0.75); \
				E6 = lerp(D, E6, 0.25); \
			} \
		} else if (B == C && C != F) { \
			/* horizontal slope */ \
			E0 = D; \
			E1 = lerp(D, E1, 0.75); \
			E2 = lerp(D, E2, 0.25); \
			E3 = lerp(D, E3, 0.25); \
		} else if (D == G && G != H) { \
			/* vertical slope */ \
			E0 = D; \
			E3 = lerp(D, E3, 0.75); \
			E6 = lerp(D, E6, 0.25); \
			E1 = lerp(D, E1, 0.25); \
		} else { \
			/* pure diagonal */ \
			E0 = lerp(D, E0, 0.875); \
			E1 = lerp(D, E1, 0.125); \
			E3 = lerp(D, E3, 0.125); \
		} \
	}

#define SCALE3K_SPIKE(A,B,C,D,E,F,G,H,I,E0,E1,E2,E3,E4,E5,E6,E7,E8) \
	if (B != E && D != E) { \
		if (D == B) { \
			/* diagonal */ \
			if (B == C && D == G) { \
				/* square block */ \
			} else if (B == C) { \
				/* horizontal slope */ \
				E0 = D; \
				E1 = lerp(D, E1, 0.75); \
				E2 = lerp(D, E2, 0.25); \
				E3 = lerp(D, E3, 0.25); \
			} else if (D == G) { \
				/* vertical slope */ \
				E0 = D; \
				E3 = lerp(D, E3, 0.75); \
				E6 = lerp(D, E6, 0.25); \
				E1 = lerp(D, E1, 0.25); \
			} else { \
				/* pure diagonal */ \
				E0 = lerp(D, E0, 0.875); \
				E1 = lerp(D, E1, 0.125); \
				E3 = lerp(D, E3, 0.125); \
			} \
		} else if (A == B && G == E) { \
			/* horizontal spike */ \
			E0 = lerp(D, E0, 0.875); \
			E1 = lerp(D, E1, 0.125); \
			E3 = lerp(D, E3, 0.125); \
		} else if (A == D && C == E) { \
			/* vertical spike */ \
			E0 = lerp(B, E0, 0.875); \
			E1 = lerp(B, E1, 0.125); \
			E3 = lerp(B, E3, 0.125); \
		} \
	}

#define SCALE3K_ALTERNATE(A,B,C,D,E,F,G,H,I,E0,E1,E2,E3,E4,E5,E6,E7,E8) \
	if (D == B && B != F && D != H) { \
		/* diagonal */ \
		if (B == C && D == G) { \
			/* square block */ \
			if (A != E) { \
				E0 = D; \
				E1 = lerp(D, E1, 0.75); \
				E2 = lerp(D, E2, 0.25); \
				E3 = lerp(D, E3, 0.75); \
				E6 = lerp(D, E6, 0.25); \
			} \
		} else if (B == C && C != F) { \
			/* horizontal slope */ \
			E0 = D; \
			E1 = lerp(D, E1, 0.75); \
			E2 = lerp(D, E2, 0.25); \
			E3 = lerp(D, E3, 0.25); \
		} else if (D == G && G != H) { \
			/* vertical slope */ \
			E0 = D; \
			E3 = lerp(D, E3, 0.75); \
			E6 = lerp(D, E6, 0.25); \
			E1 = lerp(D, E1, 0.25); \
		} else { \
			/* pure diagonal */ \
			E0 = lerp(D, E0, 0.875); \
			E1 = lerp(D, E1, 0.125); \
			E3 = lerp(D, E3, 0.125); \
		} \
	}

				E0 = E1 = E2 = E3 = E4 = E5 = E6 = E7 = E8 = E;

				SCALE3K(A,B,C,D,E,F,G,H,I,E0,E1,E2,E3,E4,E5,E6,E7,E8);
				SCALE3K(G,D,A,H,E,B,I,F,C,E6,E3,E0,E7,E4,E1,E8,E5,E2);
				SCALE3K(I,H,G,F,E,D,C,B,A,E8,E7,E6,E5,E4,E3,E2,E1,E0);
				SCALE3K(C,F,I,B,E,H,A,D,G,E2,E5,E8,E1,E4,E7,E0,E3,E6);
				break;

			case 3 :
				/* rejected */
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
			case 4 :
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
			case 5 :
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
			case 6 :
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
			case 7 :
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
			case 8 :
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

	for(y=0;y<height;++y) {
		for(x=0;x<width;++x) {
			pixel_t E0, E1, E2, E3, E4, E5, E6, E7, E8, E9, EA, EB, EC, ED, EE, EF;
			pixel_t EV[16];
			pixel_t A, B, C, D, E, F, G, H, I;
			pixel_t A1, B1, C1, G5, H5, I5, A0, D0, G0, C4, F4, I4;

			/* scalex */
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
     0  1  2  3  4

0       A1 B1 C1
1    A0 PA PB PC C4
2    D0 PD PE PF F4
3    G0 PG PH PI I4
4       G5 H5 I5
*/

			/* xbr */
			A1 = pixel_get(x-1, y-2, src_ptr, src_slice, pixel, width, height, opt_tes);
			B1 = pixel_get(x, y-2, src_ptr, src_slice, pixel, width, height, opt_tes);
			C1 = pixel_get(x+1, y-2, src_ptr, src_slice, pixel, width, height, opt_tes);
			G5 = pixel_get(x-1, y+2, src_ptr, src_slice, pixel, width, height, opt_tes);
			H5 = pixel_get(x, y+2, src_ptr, src_slice, pixel, width, height, opt_tes);
			I5 = pixel_get(x+1, y+2, src_ptr, src_slice, pixel, width, height, opt_tes);
			A0 = pixel_get(x-2, y-1, src_ptr, src_slice, pixel, width, height, opt_tes);
			D0 = pixel_get(x-2, y, src_ptr, src_slice, pixel, width, height, opt_tes);
			G0 = pixel_get(x-2, y+1, src_ptr, src_slice, pixel, width, height, opt_tes);
			C4 = pixel_get(x+2, y-1, src_ptr, src_slice, pixel, width, height, opt_tes);
			F4 = pixel_get(x+2, y, src_ptr, src_slice, pixel, width, height, opt_tes);
			I4 = pixel_get(x+2, y+1, src_ptr, src_slice, pixel, width, height, opt_tes);


			/*
				ABC
				DEF
				GHI

				E0E1E2E3
				E4E5E6E7
				E8E9EAEB
				ECEDEEEF
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
				E9 = E;
				EA = E;
				EB = E;
				EC = E;
				ED = E;
				EE = E;
				EF = E;
				break;

			case 2:
				/* scalek */
				
			/*
				ABC
				DEF
				GHI

				E0E1E2E3
				E4E5E6E7
				E8E9EAEB
				ECEDEEEF
			*/


#define SCALE4K_BASE(A,B,C,D,E,F,G,H,I,E0,E1,E2,E3,E4,E5,E6,E7,E8,E9,EA,EB,EC,ED,EE,EF) \
	if (D == B && B != E && D != E) { \
		/* diagonal */ \
		if (B == C && D == G) { \
			/* square block */ \
		} else if (B == C && C != F) { \
			/* horizontal slope */ \
			E0 = D; \
			E1 = D; \
			E2 = lerp(D, E2, 0.75); \
			E3 = lerp(D, E3, 0.25); \
			E4 = lerp(D, E4, 0.75); \
			E5 = lerp(D, E5, 0.25); \
		} else if (D == G && G != H) { \
			/* vertical slope */ \
			E0 = D; \
			E4 = D; \
			E8 = lerp(D, E8, 0.75); \
			EC = lerp(D, EC, 0.25); \
			E1 = lerp(D, E1, 0.75); \
			E5 = lerp(D, E5, 0.25); \
		} else { \
			/* pure diagonal */ \
			E0 = D; \
			E1 = lerp(D, E1, 0.5); \
			E4 = lerp(D, E4, 0.5); \
		} \
	}

/* Used by AdvanceMAME */
#define SCALE4K(A,B,C,D,E,F,G,H,I,E0,E1,E2,E3,E4,E5,E6,E7,E8,E9,EA,EB,EC,ED,EE,EF) \
	if (D == B && B != E && D != E) { \
		/* diagonal */ \
		if (B == C && D == G) { \
			if (A != E) { \
				/* no star */ \
				E0 = D; \
				E1 = D; \
				E2 = lerp(D, E2, 0.75); \
				E3 = lerp(D, E3, 0.25); \
				E4 = D; \
				E8 = lerp(D, E8, 0.75); \
				EC = lerp(D, EC, 0.25); \
				E5 = lerp(D, E5, 0.50); \
			} \
		} else if (B == C && C != F) { \
			/* horizontal slope */ \
			E0 = D; \
			E1 = D; \
			E2 = lerp(D, E2, 0.75); \
			E3 = lerp(D, E3, 0.25); \
			E4 = lerp(D, E4, 0.75); \
			E5 = lerp(D, E5, 0.25); \
		} else if (D == G && G != H) { \
			/* vertical slope */ \
			E0 = D; \
			E4 = D; \
			E8 = lerp(D, E8, 0.75); \
			EC = lerp(D, EC, 0.25); \
			E1 = lerp(D, E1, 0.75); \
			E5 = lerp(D, E5, 0.25); \
		} else { \
			/* pure diagonal */ \
			E0 = D; \
			E1 = lerp(D, E1, 0.5); \
			E4 = lerp(D, E4, 0.5); \
		} \
	}

#define SCALE4K_ALTERNATE(A,B,C,D,E,F,G,H,I,E0,E1,E2,E3,E4,E5,E6,E7,E8,E9,EA,EB,EC,ED,EE,EF) \
	if (D == B && B != F && D != H) { \
		/* diagonal */ \
		if (B == C && D == G) { \
			/* avoid starts */ \
			if (A != E) { \
				E0 = D; \
				E1 = D; \
				E2 = lerp(D, E2, 0.75); \
				E3 = lerp(D, E3, 0.25); \
				E4 = D; \
				E8 = lerp(D, E8, 0.75); \
				EC = lerp(D, EC, 0.25); \
				E5 = lerp(D, E5, 0.50); \
			} \
		} else if (B == C && C != F) { \
			/* horizontal slope */ \
			E0 = D; \
			E1 = D; \
			E2 = lerp(D, E2, 0.75); \
			E3 = lerp(D, E3, 0.25); \
			E4 = lerp(D, E4, 0.75); \
			E5 = lerp(D, E5, 0.25); \
		} else if (D == G && G != H) { \
			/* vertical slope */ \
			E0 = D; \
			E4 = D; \
			E8 = lerp(D, E8, 0.75); \
			EC = lerp(D, EC, 0.25); \
			E1 = lerp(D, E1, 0.75); \
			E5 = lerp(D, E5, 0.25); \
		} else { \
			/* pure diagonal */ \
			E0 = D; \
			E1 = lerp(D, E1, 0.5); \
			E4 = lerp(D, E4, 0.5); \
		} \
	}

				E0 = E1 = E2 = E3 = E4 = E5 = E6 = E7 = E8 = E9 = EA = EB = EC = ED = EE = EF = E;

				SCALE4K(A,B,C,D,E,F,G,H,I,E0,E1,E2,E3,E4,E5,E6,E7,E8,E9,EA,EB,EC,ED,EE,EF);
				SCALE4K(G,D,A,H,E,B,I,F,C,EC,E8,E4,E0,ED,E9,E5,E1,EE,EA,E6,E2,EF,EB,E7,E3);
				SCALE4K(I,H,G,F,E,D,C,B,A,EF,EE,ED,EC,EB,EA,E9,E8,E7,E6,E5,E4,E3,E2,E1,E0);
				SCALE4K(C,F,I,B,E,H,A,D,G,E3,E7,EB,EF,E2,E6,EA,EE,E1,E5,E9,ED,E0,E4,E8,EC);
				break;

			case 3:
				/* xbr */

/*
     0  1  2  3  4

0       A1 B1 C1
1    A0 PA PB PC C4
2    D0 PD PE PF F4
3    G0 PG PH PI I4
4       G5 H5 I5
*/

#define XBR(PE, PI, PH, PF, PG, PC, PD, PB, PA, G5, C4, G0, D0, C1, B1, F4, I4, H5, I5, A0, A1, N0, N1, N2, N3, N4, N5, N6, N7, N8, N9, N10, N11, N12, N13, N14, N15) \
	if (PE!=PH && PE!=PF) {\
		unsigned e = df(PE,PC) + df(PE,PG) + df(PI,H5) + df(PI,F4) + 4*df(PH,PF); \
		unsigned i = df(PH,PD) + df(PH,I5) + df(PF,I4) + df(PF,PB) + 4*df(PE,PI); \
		if (e<i) { \
			int ex2 = PE!=PC && PB!=PC; \
			int ex3 = PE!=PG && PD!=PG; \
			unsigned ke = df(PF,PG); \
			unsigned ki = df(PH,PC); \
			pixel_t px = df(PE,PF) <= df(PE,PH) ? PF : PH; \
			if (ke == 0 && ki == 0 && ex3 && ex2) { \
				LEFT_UP_2(N15, N14, N11, N13, N12, N10, N7, N3, px); \
			} else if (2*ke <= ki && ex3) { \
				LEFT_2(N15, N14, N11, N13, N12, N10, px); \
			} else if (ke >= 2*ki && ex2)  { \
				UP_2(N15, N14, N11, N3, N7, N10, px); \
			} else { \
				DIA_2(N15, N14, N11, px); \
			} \
		} \
	}

#define LEFT_UP_2(N15, N14, N11, N13, N12, N10, N7, N3, PIXEL) \
	EV[N7] = EV[N13] = lerp(PIXEL, EV[N13], 0.75); \
	EV[N3] = EV[N10] = EV[N12] = lerp(EV[N12], PIXEL, 0.75); \
	EV[N11] = EV[N14] = EV[N15] = PIXEL;

#define LEFT_2(N15, N14, N11, N13, N12, N10, PIXEL)\
	EV[N11] = lerp(PIXEL, EV[N11], 0.75); \
	EV[N13] = lerp(PIXEL, EV[N13], 0.75); \
	EV[N10] = lerp(EV[N10], PIXEL, 0.75); \
	EV[N12] = lerp(EV[N12], PIXEL, 0.75); \
	EV[N14] = EV[N15] = PIXEL;

#define UP_2(N15, N14, N11, N3, N7, N10, PIXEL)\
	EV[N14] = lerp(PIXEL, EV[N14], 0.75); \
	EV[N7] = lerp(PIXEL, EV[N7], 0.75); \
	EV[N10] = lerp(EV[N10], PIXEL, 0.75); \
	EV[N3] = lerp(EV[N3], PIXEL, 0.75); \
	EV[N11] = EV[N15] = PIXEL;

#define DIA_2(N15, N14, N11, PIXEL)\
	EV[N11] = lerp(EV[N11], PIXEL, 0.5); \
	EV[N14] = lerp(EV[N14], PIXEL, 0.5); \
	EV[N15] = PIXEL;

#define df(A, B) dist(A, B)

				EV[0] = EV[1] = EV[2] = EV[3] = EV[4] = EV[5] = EV[6] = EV[7] = EV[8] = EV[9] = EV[10] = EV[11] = EV[12] = EV[13] = EV[14] = EV[15] = E;

				XBR(E, I, H, F, G, C, D, B, A, G5, C4, G0, D0, C1, B1, F4, I4, H5, I5, A0, A1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15);
				XBR(E, C, F, B, I, A, H, D, G, I4, A1, I5, H5, A0, D0, B1, C1, F4, C4, G5, G0, 12,  8,  4,  0, 13,  9,  5,  1, 14, 10,  6,  2, 15, 11,  7,  3);
				XBR(E, A, B, D, C, G, F, H, I, C1, G0, C4, F4, G5, H5, D0, A0, B1, A1, I4, I5, 15, 14, 13, 12, 11, 10,  9,  8,  7,  6,  5,  4,  3,  2,  1,  0);
				XBR(E, G, D, H, A, I, B, F, C, A0, I5, A1, B1, I4, F4, H5, G5, D0, G0, C1, C4,  3,  7, 11, 15,  2,  6, 10, 14,  1,  5,  9, 13,  0,  4,  8, 12);

				E0 = EV[0];
				E1 = EV[1];
				E2 = EV[2];
				E3 = EV[3];
				E4 = EV[4];
				E5 = EV[5];
				E6 = EV[6];
				E7 = EV[7];
				E8 = EV[8];
				E9 = EV[9];
				EA = EV[10];
				EB = EV[11];
				EC = EV[12];
				ED = EV[13];
				EE = EV[14];
				EF = EV[15];
				break;
			}

			pixel_put(x*4, y*4, dst_ptr, dst_slice, pixel, width*4, height*4, E0);
			pixel_put(x*4+1, y*4, dst_ptr, dst_slice, pixel, width*4, height*4, E1);
			pixel_put(x*4+2, y*4, dst_ptr, dst_slice, pixel, width*4, height*4, E2);
			pixel_put(x*4+3, y*4, dst_ptr, dst_slice, pixel, width*4, height*4, E3);
			pixel_put(x*4, y*4+1, dst_ptr, dst_slice, pixel, width*4, height*4, E4);
			pixel_put(x*4+1, y*4+1, dst_ptr, dst_slice, pixel, width*4, height*4, E5);
			pixel_put(x*4+2, y*4+1, dst_ptr, dst_slice, pixel, width*4, height*4, E6);
			pixel_put(x*4+3, y*4+1, dst_ptr, dst_slice, pixel, width*4, height*4, E7);
			pixel_put(x*4, y*4+2, dst_ptr, dst_slice, pixel, width*4, height*4, E8);
			pixel_put(x*4+1, y*4+2, dst_ptr, dst_slice, pixel, width*4, height*4, E9);
			pixel_put(x*4+2, y*4+2, dst_ptr, dst_slice, pixel, width*4, height*4, EA);
			pixel_put(x*4+3, y*4+2, dst_ptr, dst_slice, pixel, width*4, height*4, EB);
			pixel_put(x*4, y*4+3, dst_ptr, dst_slice, pixel, width*4, height*4, EC);
			pixel_put(x*4+1, y*4+3, dst_ptr, dst_slice, pixel, width*4, height*4, ED);
			pixel_put(x*4+2, y*4+3, dst_ptr, dst_slice, pixel, width*4, height*4, EE);
			pixel_put(x*4+3, y*4+3, dst_ptr, dst_slice, pixel, width*4, height*4, EF);
		}
	}

	return 0;
}

#define SCALE2X3_REVISION_MAX 1

void scale2x3(unsigned char* dst_ptr, unsigned dst_slice, const unsigned char* src_ptr, unsigned src_slice, unsigned pixel, int width, int height, int opt_tes, int opt_ver)
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

void scale2x4(unsigned char* dst_ptr, unsigned dst_slice, const unsigned char* src_ptr, unsigned src_slice, unsigned pixel, int width, int height, int opt_tes, int opt_ver)
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

int file_gen(void)
{
	unsigned width;
	unsigned height;
	unsigned slice;
	unsigned pixel;
	unsigned type;
	unsigned channel;
	unsigned char* ptr;
	unsigned x,y,i,j;
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
	for(y=0;y<height;++y) {
		for(x=0;x<width;++x) {
			pixel_put(x, y, ptr, slice, pixel, width, height, 0x0000FF);
		}
	}

	for(y=0;y<16;++y) {
		for(x=0;x<32;++x) {
			unsigned mask = (y << 5) | x;
			for(j=0;j<3;++j) {
				for(i=0;i<3;++i) {
					unsigned index = j*3+i;
					unsigned color;
					if ((mask & (1 << index)) != 0) {
						color = 0xFFFFFF;
					} else {
						color = 0x000000;
					}
					pixel_put(x*delta+skew+i, y*delta+skew+j, ptr, slice, pixel, width, height, color);
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

void version(void) {
	printf(PACKAGE " v" VERSION " by Andrea Mazzoleni, " PACKAGE_URL "\n");
}

void usage(void) {
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

#define OPTIONS "k:cowr:ghv"

int main(int argc, char* argv[]) {
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
				else
					opt_ver = atoi(optarg);
				break;
			case 'g' :
				opt_gen = 1;
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

		if (file_process(argv[optind], argv[optind+1], opt_scale_x, opt_scale_y, opt_tes, opt_ver, opt_crc, opt_only124) != 0) {
			exit(EXIT_FAILURE);
		}
	}

	return EXIT_SUCCESS;
}

