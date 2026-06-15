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
#include "env.h"

#ifdef F18A
#define BIN2INC_HEADER_ONLY
#include "tilesf18_splitcol1.c"
#include "tilesf18_splitcol2.c"
#include "tilesf18_splitcol3.c"
#include "tilesf18_splitpat1.c"
#include "tilesf18_splitpat2.c"
#include "tilesf18_splitpat3.c"
#endif

#include <vdp.h>

// load the digits from tile bank 0 to the correct place set by env_digits
void loadDigitTiles(void) {
    unsigned int nOldBank = nBank;

    VDP_INT_DISABLE;

    // chars 8-20. Don't use bitmapcharcopy, only want the first page
    // assumes gPattern is 0!

#ifdef F18A
    // pattern and tiles in same bank - we only need tile bank0
    SWITCH_IN_BANK20;
    vdpmemcpy(env_digits*8, tilesf18_patA+48*8, 10*8); // digits
    vdpmemcpy((env_digits+10)*8, tilesf18_patA+1*8, 3*8);  // icons
    VDP_INT_POLL;
    vdpmemcpy(gColor+env_digits*8, tilesf18_colA+48*8, 10*8); // digits
    vdpmemcpy(gColor+(env_digits+10)*8, tilesf18_colA+1*8, 3*8);  // icons
#else
    SWITCH_IN_BANK8;
    vdpmemcpy(env_digits*8, tiles_banks_pat+48*8, 10*8); // digits
    vdpmemcpy((env_digits+10)*8, tiles_banks_pat+1*8, 3*8);  // icons
    VDP_INT_POLL;
    SWITCH_IN_BANK9;
    vdpmemcpy(gColor+env_digits*8, tiles_banks_col+48*8, 10*8); // digits
    vdpmemcpy(gColor+(env_digits+10)*8, tiles_banks_col+1*8, 3*8);  // icons
#endif

    VDP_INT_ENABLE;

    // restore bank
    SWITCH_IN_BANK(nOldBank);
}

// load the characters from a string to the correct place set by env_digits
// your own problem if the string is more than 10 chars!
void loadStringTiles(const char* str) {
    unsigned int nOldBank = nBank;

    VDP_INT_DISABLE;

    // chars 8-20. Don't use bitmapcharcopy, only want the first page
    // assumes gPattern is 0!
    U16 off = 0;
    while (*str) {
#ifdef F18A
        // pattern and tiles in same bank - we only need tile bank0
        SWITCH_IN_BANK20;
        vdpmemcpy((env_digits+off)*8, tilesf18_patA+(*str)*8, 8); // digits
        VDP_INT_POLL;
        vdpmemcpy(gColor+(env_digits+off)*8, tilesf18_colA+(*str)*8, 8); // digits
#else
        SWITCH_IN_BANK8;
        vdpmemcpy((env_digits+off)*8, tiles_banks_pat+(*str)*8, 8); // digits
        VDP_INT_POLL;
        SWITCH_IN_BANK9;
        vdpmemcpy(gColor+(env_digits+off)*8, tiles_banks_col+(*str)*8, 8); // digits
#endif

        ++str;
        ++off;
    }

    VDP_INT_ENABLE;

    // restore bank
    SWITCH_IN_BANK(nOldBank);
}


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

    if (bank >= TILES_BANKS_COUNT) {
		sys_panic("xrick/tiles: invalid bank number %d\n", bank);
    }

    // bank switching, not tile bank!
    nOldBank = nBank;

#ifdef F18A
    // the data is split among banks
    switch (bank) {
        case 0:
            SWITCH_IN_BANK20;
            bitmapcharcopy(gPattern, tilesf18_patA, 0x800);
            bitmapcharcopy(gColor, tilesf18_colA, 0x800);
            break;

        case 1:
            SWITCH_IN_BANK25;
            bitmapcharcopy(gPattern, tilesf18_patB, 0x800);
            bitmapcharcopy(gColor, tilesf18_colB, 0x800);
            break;

        case 2:
            SWITCH_IN_BANK31;
            bitmapcharcopy(gPattern, tilesf18_patC, 0x800);
            bitmapcharcopy(gColor, tilesf18_colC, 0x800);
            break;
    }

    // and also load a palette
    SWITCH_IN_BANK20;
    VDP_INT_DISABLE;
    loadpal_f18a(tilesf18_pal, 0, 16);
    VDP_INT_ENABLE;

#else
    U16 off = bank*0x800;

    // first patterns
    SWITCH_IN_BANK8;
    bitmapcharcopy(gPattern, tiles_banks_pat+off, 0x800);

    // then color
    SWITCH_IN_BANK9;
    bitmapcharcopy(gColor, tiles_banks_col+off, 0x800);

#endif

    // okay, screw it. Just find 13 characters we can overwrite, something we KNOW
    // is only for a different map.
    if (bank != 0) {
        switch (env_map) {
            case 0: // cavern
                env_digits = 8;
                break;

            case 1: // egypt
                env_digits = 60;
                break;

            case 2: // castle
                env_digits = 160;
                break;

            case 3: // missile
                env_digits = 26;
                break;
        }

        loadDigitTiles();
    }

    // restore bank
    SWITCH_IN_BANK(nOldBank);
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
