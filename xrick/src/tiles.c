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

#include <vdp.h>

/*
 * tiles_setBank
 *
 * sets current tiles bank to <bank>.
 */
void tiles_setBank(U8 bank)
{
    static U8 lastSetBank = 0xff;
    unsigned int nOldBank;

    if (lastSetBank == bank) {
        return;
    }
    lastSetBank = bank;
    if (bank == 0xff) {
        // magic for reset - needed for the title page and HOF
        return;
    }

	if (bank >= TILES_BANKS_COUNT)
		sys_panic("xrick/tiles: invalid bank number %d\n", bank);

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
}

/*
 * tiles_setFilter
 *
 * sets current tiles display filter to <filter>
 */
void tiles_setFilter(U16 filter)
{
    (void)filter;
}



/*
 * tiles_paint
 *
 * paints ONE tile <tileNumber> at the position indicated by <fb>.
 * returns next <fb> value.
 */
// TODO: it's likely everything that calls this can be replaced with a vdpmemcpy or hchar or vchar
int tiles_paint(U8 tileNumber, int fb)
{
    // TODO: could optimize by not converting to/from pointer
    U16 f = fb&0x3fff;     // I know this looks wrong, but fb is a VDP address, not a CPU one

    // TODO: We don't want to "paint tiles", we use the loaded tle and just place it
    vdpchar(gImage+f, tileNumber);

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
int tiles_paintList(U8 *tilesList, int fb)
{
	int f;

	f = fb;

	while (1)
	{
		if (*tilesList == TILES_NULL) /* end of list */
			return f;

		if (*tilesList == TILES_CRLF) /* crlf */
		{
			fb += 32;
			f = fb;
			tilesList++;
			continue;
		}

		/* else paint */
		f = tiles_paint(*(tilesList++), f);
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
