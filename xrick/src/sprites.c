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

#include <vdp.h>
#include <string.h>


/*
 * sprites_paint
 *
 * paints sprite <spriteNumber> at the position indicated by <x>, <y>.
 * <x>, <y> are fb-coordinates.
 * simple paint: no clipping, no depth management, nothing.
 */

// there are four hardware sprites for every entity
sprite_data_t sprite_table[(ENT_ENTSNUM+1)*4];

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
            // in the math below:
            // spriteIdx is the 128 byte object index on the Rick side
            // i is the starting 32 byte sprite tile index on the TI side (multiple of four, see above)
#ifndef CLASSIC99
            vdpmemcpy(gSpritePat+(i*4*8), spriteIdx*128+sprites_data0, 128);
#else
            // Classic99 build doesn't have banks to switch!
            switch(spritePage) {
                case 0:
                    vdpmemcpy(gSpritePat+(i*4*8), spriteIdx*128+sprites_data0, 128); break;
                case 1:
                    vdpmemcpy(gSpritePat+(i*4*8), spriteIdx*128+sprites_data1, 128); break;
                case 2:
                    vdpmemcpy(gSpritePat+(i*4*8), spriteIdx*128+sprites_data2, 128); break;
                case 3:
                    vdpmemcpy(gSpritePat+(i*4*8), spriteIdx*128+sprites_data3, 128); break;
                case 4:
                    vdpmemcpy(gSpritePat+(i*4*8), spriteIdx*128+sprites_data4, 128); break;
            }
#endif
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

// still not doing 'front' management, but we can clip and map adjust
void sprites_paint2(U16 spriteNumber, U16 x, U16 y) {
	U16 x_map, y_map;
	U16 x_fb, y_fb;
	U16 width, height;

	/* get map/px */
	x_map = x;
	y_map = y;

	/* sprite dimension in px */
	width = 0x20; /* width = 4 tile columns, 8 pixels each */
	height = 0x15; /* height = 0x15 pixels */

	/* clip */
	if (maps_clip(&x_map, &y_map, &width, &height))  /* return if not visible */
		return;

	/* convert to fb/px */
	x_fb = x_map + MAPS_FB_X + 32;  // fix offset
	y_fb = y_map - MAPS_FB_Y; // fix offset

    sprites_paint(spriteNumber, x_fb, y_fb);
}


/*
 * sprites_paint2
 *
 * paints sprite <spriteNumber> at the position indicated by <x>, <y>.
 * <x>, <y> are map-coordinates, they are aligned to tile columns.
 * <front> when true indicates that the sprite must not be behind anything.
 * complex paint: manages highlight, depth.
 */

/* eof */