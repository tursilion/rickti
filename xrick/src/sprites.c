/*
 * xrick/src/sprites.c
 *
 * Copyright (C) 1998-2019 bigorno (bigorno@bigorno.net). All rights reserved.
 *
 * The use and distribution terms for this software are contained in the file
 * named README, which can be found in the root of this distribution. By
 * using this software in any fashion, you are agreeing to be bound by the
 * terms of this license.
 *
 * You must not remove this notice, or any other, from this software.
 */



#include "config.h"
#include "env.h"

#include "sprites.h"
#include "fb.h"
#include "maps.h"
#include "tiles.h"

#ifdef GFXTI
#include <vdp.h>
#include <string.h>
#endif



/*
 * sprites_paint
 *
 * paints sprite <spriteNumber> at the position indicated by <x>, <y>.
 * <x>, <y> are fb-coordinates.
 * simple paint: no clipping, no depth management, nothing.
 */
#ifdef GFXPC
void sprites_paint(U8 spriteNumber, U16 x, U16 y)
{
	U16 i, j, k;
    U8 *f, *fb;
	U16 xm = 0, xp = 0;

	fb = fb_at(x, y);

	for (i = 0; i < 4; i++) /* 4 tile columns */
	{
		f = fb;
		for (j = 0; j < 0x15; j++) /* 0X15 pixel rows */
		{
			xm = sprites_data[spriteNumber][i][j].mask;  /* mask */
			xp = sprites_data[spriteNumber][i][j].pict;  /* picture */
			/* map CGA 2 bits to frame buffer 8 bits per pixels */
			for (k = 8; k--; xm >>= 2, xp >>= 2)
				f[k] = (f[k] & (xm & 3)) | (xp & 3);
			f += FB_WIDTH;
		}
		fb += 8;
	}
}
#endif

#ifdef GFXST
void sprites_paint(U8 spriteNumber, U16 x, U16 y)
{
	U16 i, j, k;
    U8 *f, *fb;
	U16 g;
	U32 d;

	fb = fb_at(x, y);
	g = 0;

	for (i = 0; i < 0x15; i++) /* 0x15 pixel rows */
	{
		f = fb;
		for (j = 0; j < 4; j++) /* 4 tile columns */
		{
			d = sprites_data[spriteNumber][g++];
			/* map ST 4 bits per pixel to frame buffer 8 bits per pixels */
			for (k = 8; k--; d >>= 4)
				if (d & 0x0f) f[k] = (f[k] & 0xf0) | (d & 0x0f);
			f += 8;
		}
		fb += FB_WIDTH;
	}
}
#endif
#ifdef GFXTI
sprite_data_t sprite_table[ENT_ENTSNUM+1];

// TODO: need to draw actual sprites... sprites are 32x32 (so four sprites each)
// There are up to 12 sprites in game (though our hardware supports 8). We'll let
// the copy function sort that out though.
void sprites_paint(U16 spriteNumber, U16 x, U16 y)
{
    unsigned int nOldBank;

    // we just have to load the four sprites into the sprite table at the first clear spot
    // TODO: need a mapping table for colors
    // we need to copy in the bitmap properly, we probably need to at this point,
    // which means we need to assign a psuedo-hardware sprite AND a character pattern.
    // If we assume entities don't switch around order, we can probably base the pattern
    // on the sprite table entry (48*4 is only 192 characters)
    for (int i=0; i<ENT_ENTSNUM+1; i+=4) {
        if (sprite_table[i].y == 0xd1) {
            sprite_table[i].y = y;
            sprite_table[i].x = x;
            sprite_table[i].ch = i*4;
            sprite_table[i].col = COLOR_WHITE;  // TODO: color table

            sprite_table[i+1].y = y+16;
            sprite_table[i+1].x = x;
            sprite_table[i+1].ch = i*4+4;
            sprite_table[i+1].col = COLOR_WHITE;  // TODO: color table

            sprite_table[i+2].y = y;
            sprite_table[i+2].x = x+16;
            sprite_table[i+2].ch = i*4+8;
            sprite_table[i+2].col = COLOR_WHITE;  // TODO: color table

            sprite_table[i+3].y = y+16;
            sprite_table[i+3].x = x+16;
            sprite_table[i+3].ch = i*4+12;
            sprite_table[i+3].col = COLOR_WHITE;  // TODO: color table

            VDP_INT_DISABLE;
            // based on which pattern we need, we need to map and copy from ROM
            // each table has 48 of those big objects (8x6)
            nOldBank = nBank;
            // since calculated, we do the bank here manually. SINCE WE DO NOT
            // UPDATE nBank YOU CAN NOT BRANCH TO ANY CODE THAT MIGHT BANK SWITCH AGAIN
            unsigned int spritePage = spriteNumber / 48;
            unsigned int spriteIdx = spriteNumber % 48;
            // Note: page index 0x6006 is determined by dat_sprintesTI0.o in the linker file
            SWITCH_IN_BANK((0x6006 + (spritePage*2)));
            vdpmemcpy(gSpritePat+(i*16)*8, spriteIdx*128+sprites_data0, 128);
            // restore the previous page
            SWITCH_IN_BANK(nOldBank);

            VDP_INT_ENABLE;

            break;
        }
    }
}

void sprites_clear() {
    memset(sprite_table, 0xd1, sizeof(sprite_table));
}

#endif



/*
 * sprites_paint2
 *
 * paints sprite <spriteNumber> at the position indicated by <x>, <y>.
 * <x>, <y> are map-coordinates, they are aligned to tile columns.
 * <front> when true indicates that the sprite must not be behind anything.
 * complex paint: manages highlight, depth.
 */
#ifdef GFXPC
void sprites_paint2(U8 spriteNumber, U16 x, U16 y, U8 front)
{
	U8 k, *f, *fb, c, r, dx;
	U16 mask, pict;
	U16 x_map, y_map;
	U16 x_fb, y_fb;
	U16 width, height;

	/* if depth is not managed then sprites are always in front of everything */
	if (!env_depth) front = TRUE;

	/* get map/px */
	x_map = x;
	y_map = y;

	/* align to tile column */
	x_map = x_map & 0xfff8;

	/* sprite dimension in px */
	width = 0x20; /* width = 4 tile columns, 8 pixels each */
	height = 0x15; /* height = 0x15 pixels */

	/* shift */
	dx = (x - x_map) * 2;

	/* clip */
	if (maps_clip(&x_map, &y_map, &width, &height))  /* return if not visible */
		return;

	/* convert to fb/px */
	x_fb = x_map - MAPS_FB_X;
	x_fb = y_map - MAPS_FB_Y;

	/* get buffer */
	fb = fb_at(x_fb, y_fb);

	/* convert from px to tl */
	x_map >>= 3;
	width >>= 3;

	/* draw */
	for (c = 0; c < width; c++) /* for each tile column */
	{
		f = fb;
		for (r = 0; r < height; r++) /* for each pixel row */
		{
			/*
			 * paint only if: <front> is true or env_highlight is true or the
			 * sprite is not behind foreground tiles.
			 */
			if (front || env_highlight ||
				!(map_eflg[map_map[(ymap + r) >> 3][xmap + c]] & MAP_EFLG_FGND))
			{
				pict = mask = 0;
				if (c > 0)
				{
					mask |= sprites_data[spriteNumber][c - 1][r].mask << (16 - dx);
					pict |= sprites_data[spriteNumber][c - 1][r].pict << (16 - dx);
				}
				else
				{
					mask |= 0xffff << (16 - dx);
				}
				if (c < cmax)
				{
					mask |= sprites_data[spriteNumber][c][r].mask >> dx;
					pict |= sprites_data[spriteNumber][c][r].pict >> dx;
				}
				else
				{
					mask |= 0xffff >> dx;
				}

				/* map CGA 2 bits to frame buffer 8 bits per pixels */
				for (k = 8; k--; xm >>= 2, xp >>= 2)
				{
					f[k] = ((f[k] & (mask & 3)) | (pict & 3));
					if (env_highlight) f[k] |= 4;
				}
			}
			f += FB_WIDTH;
		}
		fb += 8;
	}
}
#endif

#ifdef GFXST
void sprites_paint2(U8 spriteNumber, U16 x, U16 y, U8 front)
{
	U32 d = 0;	/* sprite data */
	U16 x0, y0;	/* clipped x, y */
	U16 width, height;
	S16 g;		/* sprite data offset*/
	S16 r, c;	/* row, column */ /* S/U: loop while >=0 */
	S16 i;		/* frame buffer shifter */
	S16 im;		/* tile flag shifter */
	U8 flg;		/* tile flag */
	U8 *fb;		/* frame buffer */
	U16 x_fb, y_fb;

	/* if depth is not managed then sprites are always in front of everything */
	if (!env_depth) front = TRUE;

	x0 = x;
	y0 = y;

	/* sprite dimension in px */
	width = 0x20; /* width = 4 tile columns, 8 pixels each */
	height = 0x15; /* height = 0x15 pixels */

	/* clip */
	if (maps_clip(&x0, &y0, &width, &height))  /* return if not visible */
		return;

	g = 0;


	/* convert to fb/px */
	x_fb = x0 - MAPS_FB_X;
	y_fb = y0 - MAPS_FB_Y+8; /* FIXME =8? */

	/* get buffer */
	fb = fb_at(x_fb, y_fb);

	/* draw */
	for (r = 0; r < 0x15; r++) /* for each pixel row */
	{
		if (r >= height || y + r < y0) continue;

		i = 0x1f;
		im = x - (x & 0xfff8);
		flg = map_eflg[map_map[(y + r) >> 3][(x + 0x1f)>> 3]];

#define LOOP(N, C0, C1) \
		d = sprites_data[spriteNumber][g + N]; \
		for (c = C0; c >= C1; c--, i--, d >>= 4, im--) \
		{ \
			if (im == 0) \
			{ \
				flg = map_eflg[map_map[(y + r) >> 3][(x + c) >> 3]]; \
				im = 8; \
			} \
			if (c >= width || x + c < x0) continue; \
			if (!front && !env_highlight && (flg & MAP_EFLG_FGND)) continue; \
			if (d & 0x0f) fb[i] = (fb[i] & 0xf0) | (d & 0x0f); \
			if (env_highlight) fb[i] |= 0x10; \
		}

		LOOP(3, 0x1f, 0x18);
		LOOP(2, 0x17, 0x10);
		LOOP(1, 0x0f, 0x08);
		LOOP(0, 0x07, 0x00);

#undef LOOP

		fb += FB_WIDTH;
		g += 4;
	}
}
#endif

#ifndef GFXTI
/*
 * sprites_clear
 *
 * repaints the map behind a sprite at position <x>, <y>.
 * <x>, <y> are map-coordinates, they are aligned to tile columns and clipped.
 */
void
sprites_clear(U16 x, U16 y)
{
	U8 r, c;
	U16 rmax, cmax;
	U16 xmap, ymap;
	U16 xs, ys;
	U8 *fb;

  	/* align to column and row */
	xmap = x & 0xFFF8;
	ymap = y & 0xFFF8;

	cmax = (x - xmap == 0 ? 0x20 : 0x28);  /* width, 4 tl cols, 8 pix each */
	rmax = (y & 0x04) ? 0x20 : 0x18;  /* height, 3 or 4 tile rows */

	/* clip */
	if (maps_clip(&xmap, &ymap, &cmax, &rmax))  /* return if not visible */
		return;

	/* convert to fb-coordinates */
	xs = xmap - MAPS_FB_X;
	ys = ymap - MAPS_FB_Y;
	xmap >>= 3;
	ymap >>= 3;
	cmax >>= 3;
	rmax >>= 3;

	/* draw */
	for (r = 0; r < rmax; r++) /* for each row */
	{
#ifdef GFXPC
		fb = fb_at(xs, ys + r * 8);
#endif
#ifdef GFXST
		fb = fb_at(xs, 8 + ys + r * 8);
#endif
		for (c = 0; c < cmax; c++) /* for each column */
		{
			fb = tiles_paint(map_map[ymap + r][xmap + c], fb);
		}
	}
}
#endif

/* eof */