/*
 * XRICK
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
#include "tiles.h"
#include "fb.h"

#ifdef GFXTI
#include <vdp.h>
#endif

#ifndef GFXTI
static tile_t *tiles_bank;
static U16 tiles_filter;
#endif

/*
 * tiles_setBank
 *
 * sets current tiles bank to <bank>.
 */
void tiles_setBank(U8 bank)
{
    unsigned int nOldBank;

	if (bank >= TILES_BANKS_COUNT)
		sys_panic("xrick/tiles: invalid bank number %d\n", bank);

#ifndef GFXTI
	tiles_bank = tiles_banks[bank];
#endif

#ifdef GFXTI
    U16 off = bank*0x800;
    nOldBank = nBank;

    VDP_INT_DISABLE;
    
    // first patterns
    SWITCH_IN_BANK8;
    vdpmemcpy(gPattern, tiles_banks_pat+off, 0x800);
    vdpmemcpy(gPattern+0x800, tiles_banks_pat+off, 0x800);
    vdpmemcpy(gPattern+0x1000, tiles_banks_pat+off, 0x800);

    // then color
    SWITCH_IN_BANK9;
    vdpmemcpy(gColor, tiles_banks_col+off, 0x800);
    vdpmemcpy(gColor+0x800, tiles_banks_col+off, 0x800);
    vdpmemcpy(gColor+0x1000, tiles_banks_col+off, 0x800);

    // restore bank
    SWITCH_IN_BANK(nOldBank);
    VDP_INT_ENABLE;
#endif
}

/*
 * tiles_setFilter
 *
 * sets current tiles display filter to <filter>
 */
void tiles_setFilter(U16 filter)
{
#ifndef GFXTI
	tiles_filter = filter;
#else
    (void)filter;
#endif
}



/*
 * tiles_paint
 *
 * paints ONE tile <tileNumber> at the position indicated by <fb>.
 * returns next <fb> value.
 */
// TODO: it's likely everything that calls this can be replaced with a vdpmemcpy or hchar or vchar
U8 *tiles_paint(U8 tileNumber, U8 *fb)
{
#ifdef GFXPC
	U16 i;
    U16 k;
	U16 x;
    U8 *f;
	f = fb;
#endif
#ifdef GFXST
	U16 i;
    U16 k;
	U32 x;
    U8 *f;
	f = fb;
#endif
#ifdef GFXTI
    // TODO: could optimize by not converting to/from pointer
    U16 f = ((U16)fb)&0x3fff;     // I know this looks wrong, but fb is a VDP address, not a CPU one
#endif

#ifdef GFXPC
	for (i = 0; i < 8; i++) /* 8 pixel lines */
	{
		/* map CGA 2 bits per pixel to frame buffer 8 bits per pixels */
		x = tiles_bank[tileNumber][i] & tiles_filter;
		for (k = 8; k--; x >>= 2)
			f[k] = x & 3;
		f += FB_WIDTH; /* next line */
    }
#endif
#ifdef GFXST
	for (i = 0; i < 8; i++) /* 8 pixel lines */
	{
		/* map ST 4 bits per pixel to frame buffer 8 bits per pixels */
		x = tiles_bank[tileNumber][i];
		for (k = 8; k--; x >>= 4)
			f[k] = x & 0x0f;
		f += FB_WIDTH; /* next line */
    }
#endif
#ifdef GFXTI
    // TODO: We don't want to "paint tiles", we use the loaded tle and just place it
    vdpchar(gImage+f, tileNumber);
#endif

	return fb + 1;
}



/*
 * tiles_paintAt
 *
 * paints tile <tileNumber> at the position indicated by <x>, <y>.
 * <x>, <y> are fb-coordinates.
 */
void tiles_paintAt(U8 tileNumber, U16 x, U16 y)
{
	tiles_paint(tileNumber, fb_at(x, y));
}



/*
 * tiles_paintList
 *
 * paints list of tiles <tilesList> at the position indicated by <fb>. the
 * list must be TILES_NULL terminated and can contain TILES_CRLF elements
 * to produce crlf.
 *
 * returns next <fb> value.
 */
U8 *tiles_paintList(U8 *tilesList, U8 *fb)
{
	U8 *f;

	f = fb;

	while (1)
	{
		if (*tilesList == TILES_NULL) /* end of list */
			return f;

		if (*tilesList == TILES_CRLF) /* crlf */
		{
			fb += 8 * FB_WIDTH;
			f = fb;
			tilesList++;
			continue;
		}

		/* else paint */
		tiles_paint(*tilesList, f);
		f += 8;
		tilesList++;
	}
}



/*
 * tiles_paintListAt
 *
 * paints list of tiles <tilesList> at the position indicated by <x>, <y>. the
 * list must be TILES_NULL terminated and can contain TILES_CRLF elements to
 * produce crlf.
 * <x>, <y> are fb-coordinates.
 */
void tiles_paintListAt(U8 *tilesList, U16 x, U16 y)
{
	tiles_paintList(tilesList, fb_at(x, y));
}



/* eof */
