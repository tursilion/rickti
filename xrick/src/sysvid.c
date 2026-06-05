/*
 * xrick/src/sysvid.c
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

 /*
  * The purpose of this file is to implement a set of functions so that the
  * 8bit, palettized frame buffer onto which the entire game is painted can
  * be displayed onto the computer's screen.
  *
  * The only dependency between this and the game is that here we know the
  * frame buffer is 8bit. We don't know its size.
  */

#include "config.h"

#include <vdp.h>
#ifdef CLASSIC99
#include "sprites.h"
#endif

void sysvid_update(void *rects) { 
    (void)rects; 
    static U16 spriteStart = 0;
#ifdef CLASSIC99
    // TODO: since there are only 12 entities, try this:
    // First copy all active sprite first sprites - that's most of the character anyway
    // These 12 can rotate within themselves for flicker. That way everything gets a representation
    // Then we can rotate through the rest of the set for whatever hardware sprites are left.

    // we don't have a interrupt sprite copy loop anymore, so copy it here
    // Well, the first part of it anyway
    U16 cntHw = 32;                     // hardware sprite count
    U8* pSprite = &sprite_table[spriteStart].y;   // psuedo sprite table
    U16 cntPsuedo = ((ENT_ENTSNUM+1)*4)*4;  // psuedo sprite count
    U16 loop = 0;

    VDP_INT_DISABLE;
    VDP_SET_ADDRESS_WRITE(gSprite);

    while ((cntPsuedo > 0) && (cntHw > 0)) {
        if (*pSprite != 0xd1) {
            // copy this entry
            VDPWD(*(pSprite++));
            VDPWD(*(pSprite++));
            VDPWD(*(pSprite++));
            VDPWD(*(pSprite++));
            --cntHw;
        } else {
            pSprite += 4;
        }
        --cntPsuedo;
        // split up the various sprite parts
        pSprite+=12;
        if (pSprite >= &sprite_table[(ENT_ENTSNUM+1)*4].y) {
            // wraparound
            pSprite -= sizeof(sprite_table);
            pSprite+=loop;
            if (!loop) loop = 4;
        }
    }
    if (cntHw) VDPWD(0xd0);
    VDP_INT_ENABLE;

    // We can guarantee that every other sprite is stacked vertically, so
    // trying different steps to cover the whole table
    spriteStart += 31;
    if (spriteStart >= (ENT_ENTSNUM+1)*4) {
        spriteStart -= 32;
    }
#endif
}    // game_rects - can delete this (except using it in the classic99 version
void sysvid_shutdown(void) { }  // nothing to really do here

// game often uses 0 and 255 to turn screen on and off, so we could implement that!
// all non zero is treated as visible
void sysvid_setGamma(U16 g) {
    VDP_INT_DISABLE;

    if (g) {
        // we probably don't change modes from bitmap, but we have the kscan mirror, might as well use it
        VDP_REG1_KSCAN_MIRROR |= (VDP_MODE1_UNBLANK);
    } else {
        VDP_REG1_KSCAN_MIRROR &= ~(VDP_MODE1_UNBLANK);
    }
    VDP_SET_REGISTER(1, VDP_REG1_KSCAN_MIRROR);

    VDP_INT_ENABLE;
}


/* eof */



