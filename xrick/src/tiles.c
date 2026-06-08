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
#include "sysvid.h"

#include <vdp.h>

/*
 * tiles_setBank
 *
 * sets current tiles bank to <bank>.
 */
void tiles_setBank(U16 bank)
{
    static U16 lastSetBank = 0xff;
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
    bitmapcharcopy(gPattern, tiles_banks_pat+off, 0x800);

    // then color
    SWITCH_IN_BANK9;
    bitmapcharcopy(gColor, tiles_banks_col+off, 0x800);

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
 * tiles_paintList
 *
 * paints list of tiles <tilesList> at the position indicated by <fb>. the
 * list must be TILES_NULL terminated and can contain TILES_CRLF elements
 * to produce crlf.
 *
 * returns next <fb> value.
 */
int tiles_paintList(U8* tilesList, int fb)
{
	int f;

	f = fb&0x3fff;

    VDP_INT_DISABLE;

    VDP_SET_ADDRESS_WRITE(f+gImage);

	while (1)
	{
		if (*tilesList == TILES_NULL) /* end of list */ {
            VDP_INT_ENABLE;
			return f;
        }

		if (*tilesList == TILES_CRLF) /* crlf */
		{
			fb += 32;
			f = fb&0x3fff;
            VDP_SET_ADDRESS_WRITE(f+gImage);
			tilesList++;
			continue;
		}

		/* else paint */
        VDPWD(*(tilesList++));
        ++f;
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
void tiles_paintListAt(U8* tilesList, U16 x, U16 y)
{
	tiles_paintList(tilesList, fb_at(x, y));
}



/* eof */
