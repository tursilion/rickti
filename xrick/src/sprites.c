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

// there are four hardware sprites for every entity (ENTSNUM is 12, so this totals 52. Four bytes each makes 208)
// TODO: why is it +1?
// CAREFUL here - this *4 represents four sprites per entity, a sprite is 4 bytes (sprite_data_t)
sprite_data_t sprite_table[(ENT_ENTSNUM+1)*4];

// TODO: need to draw actual sprites... sprites are 32x32 (so four sprites each)
// There are up to 12 sprites in game (though our hardware supports 8). We'll let
// the copy function sort that out though.
// This is only called directly by the preview screen
void sprites_paint(U16 spriteNumber, U16 spriteIndex, U16 x, U16 y, U16 load_pattern)
{
    unsigned int nOldBank;

    // we just have to load the four sprites into the sprite table, and maybe also the pattern
    // (48*4 is only 192 characters, so all psuedo sprites are fine with a dedicated pattern)
    // TODO: need a mapping table for colors
    U8 chr = (U8)(spriteIndex*4);   // work around compiler bug - if we put this inline with the .ch (note: compiler bug is now fixed)
                    // there's also a weird inefficiency, it will use @1(r5) for sprite_table[i].x, but recalculates for .ch 
                    // instead of just doing @2(r5)
    U8* pSpriteTab = &sprite_table[spriteIndex].y;  // y is first entry in the struct
        
    *(pSpriteTab++) = (U8)y;
    *(pSpriteTab++) = (U8)x;
    *(pSpriteTab++) = chr;
    *(pSpriteTab++) = COLOR_WHITE;  // TODO: color table

    *(pSpriteTab++) = (U8)(y+16);
    *(pSpriteTab++) = (U8)x;
    *(pSpriteTab++) = chr+4;
    *(pSpriteTab++) = COLOR_WHITE;  // TODO: color table

    *(pSpriteTab++) = (U8)y;
    *(pSpriteTab++) = (U8)(x+16);
    *(pSpriteTab++) = chr+8;
    *(pSpriteTab++) = COLOR_WHITE;  // TODO: color table

    *(pSpriteTab++) = (U8)y+16;
    *(pSpriteTab++) = (U8)x+16;
    *(pSpriteTab++) = chr+12;
    *(pSpriteTab++) = COLOR_WHITE;  // TODO: color table

    if (load_pattern) {
        VDP_INT_DISABLE;
        // based on which pattern we need, we need to map and copy from ROM
        // each table has 48 of those big objects (8x6)
        nOldBank = nBank;
        // since calculated, we do the bank here manually. SINCE WE DO NOT
        // UPDATE nBank YOU CAN NOT BRANCH TO ANY CODE THAT MIGHT BANK SWITCH AGAIN
        unsigned int spritePage = spriteNumber / 48;    // sprite page to read from
        unsigned int spriteIdx = spriteNumber % 48;     // Index on a page, not to confuse with spriteIndex, which is psuedo hardware
        // Note: page index 0x6006 is determined by dat_sprintesTI0.o in the linker file
        SWITCH_IN_BANK((0x6006 + (spritePage*2)));
        // in the math below:
        // spriteIdx is the 128 byte object index on the Rick side
        // i is the starting 32 byte sprite tile index on the TI side (multiple of four, see above)
        // Since sprites are only 21 rows tall, not 32, we can save a few bytes of copy. To make it
        // simple we only do two memcpys (not four, cause of setup overhead), copying 53 bytes instead 
        // of 64 for each left and right half
#ifndef CLASSIC99
        vdpmemcpy(gSpritePat+(spriteIndex*4*8), spriteIdx*128+sprites_data0, 53);
        vdpmemcpy(gSpritePat+(spriteIndex*4*8)+64, spriteIdx*128+sprites_data0+64, 53);
#else
        // Classic99 build doesn't have banks to switch!
        switch(spritePage) {
            case 0:
                vdpmemcpy(gSpritePat+(spriteIndex*4*8), spriteIdx*128+sprites_data0, 128); break;
            case 1:
                vdpmemcpy(gSpritePat+(spriteIndex*4*8), spriteIdx*128+sprites_data1, 128); break;
            case 2:
                vdpmemcpy(gSpritePat+(spriteIndex*4*8), spriteIdx*128+sprites_data2, 128); break;
            case 3:
                vdpmemcpy(gSpritePat+(spriteIndex*4*8), spriteIdx*128+sprites_data3, 128); break;
            case 4:
                vdpmemcpy(gSpritePat+(spriteIndex*4*8), spriteIdx*128+sprites_data4, 128); break;
        }
#endif
        // restore the previous page
        SWITCH_IN_BANK(nOldBank);

        VDP_INT_ENABLE;
    }
}

void sprites_clear() {
    // once we have sprite indexes hard assigned to entities, we don't need to
    // call this function every frame, which is causing flicker
    memset(sprite_table, 0xd1, sizeof(sprite_table));
    // might as well deassign the entity sprites too
    for (int i=0; i<ENT_ENTSNUM + 1; ++i) {
        ent_ents[i].spriteIndex = 0xff;
    }
}

/*
 * sprites_paint2
 *
 * paints sprite <spriteNumber> at the position indicated by <x>, <y>.
 * <x>, <y> are map-coordinates, they are aligned to tile columns.
 * <front> when true indicates that the sprite must not be behind anything.
 * complex paint: manages highlight, depth.
 */
// still not doing 'front' management, but we can clip
void sprites_paint2(U16 entityNumber) {
	U16 x_map, y_map;
	U16 x_fb, y_fb;
	const U16 width = 0x20, height = 0x15;

	/* get map/px */
	x_map = ent_ents[entityNumber].x;
	y_map = ent_ents[entityNumber].y;

	/* clip */
	if (maps_clip(x_map, y_map, width, height)) {  /* return if not visible */
        // make sure sprite is offscren
        if (ent_ents[entityNumber].spriteIndex != 0xff) {
            // clip all four sprites
            sprite_table[ent_ents[entityNumber].spriteIndex].y = 0xd2;  // 0xd2 so it does not get re-assigned to someone else!
            sprite_table[ent_ents[entityNumber].spriteIndex+1].y = 0xd2;
            sprite_table[ent_ents[entityNumber].spriteIndex+2].y = 0xd2;
            sprite_table[ent_ents[entityNumber].spriteIndex+3].y = 0xd2;
        }
		return;
    }

    /* assign a sprite index if necessary */
    if (ent_ents[entityNumber].spriteIndex == 0xff) {
        for (unsigned int i=0; i<(ENT_ENTSNUM+1)*4; i+=4) {
            if (sprite_table[i].y == 0xd1) {
                ent_ents[entityNumber].spriteIndex = i;
                break;
            }
        }
    }

	/* convert to fb/px */
	x_fb = x_map + MAPS_FB_X + 32;  // fix offset
	y_fb = y_map - MAPS_FB_Y; // fix offset

    sprites_paint(ent_ents[entityNumber].sprite,        // pattern to draw
                  ent_ents[entityNumber].spriteIndex,   // pseudo sprite index
                  x_fb, y_fb,                           // location
                  ent_ents[entityNumber].lastSpriteDrawn != ent_ents[entityNumber].sprite); // whether the pattern needs reloading
}

/* eof */