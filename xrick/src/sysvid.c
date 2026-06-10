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

void sysvid_update() { 
#ifdef CLASSIC99
    // we don't have a interrupt sprite copy loop in Classic99, so copy it here
    // Well, the first part of it anyway

    // First copy all active sprite first sprites - that's most of the character anyway
    // These 12 can rotate within themselves for flicker. That way everything gets a representation
    // Then we can rotate through the rest of the set for whatever hardware sprites are left.

    static U16 firstSprite = 0;         // this will step through the 12 objects to select the first sprite to draw
                                        // we count by SPRITE, and each object is 4 sprites. Each object represent
                                        // 2 sprites across, so we have to count by 2*4 to make four sprites across.
    U16 cntHw = 32;                     // hardware sprites left
    U8* pSprite;                        // starting point to read sprite data
                                        // Note: this is U8, so reads BYTES. Each entity is 4 sprites
                                        // times 4 bytes - 16 bytes.

    VDP_INT_DISABLE;

    // first count the first sprite of each object - these ALWAYS fit in the hardware sprite count
    // then for as long as we have left, proceed with the next three
    VDP_SET_ADDRESS_WRITE(gSprite);

    for (int j=0; ((j<4)&&(cntHw > 0)); ++j) {
        pSprite = &sprite_table[j+firstSprite].y;
        for (int i=0; i<ENT_ENTSNUM; ++i) {
            if (*pSprite < 0xc0) {
                // visible on screen, even a little
                VDPWD(*(pSprite++));
                VDPWD(*(pSprite++));
                VDPWD(*(pSprite++));
                VDPWD(*(pSprite++));
                --cntHw;
                if (cntHw == 0) {
                    break;
                }
            } else {
                pSprite += 4;
            }
            // skip the next three
            pSprite += 12;
            // sprite_table is in sprite_data_t's, and sprite is in U8s
            if (pSprite >= &sprite_table[ENT_ENTSNUM*4].y) {
                pSprite -= ENT_ENTSNUM*4*4;
            }
        }
    }

    // all done, end the table and rotate the sprite start point
    VDPWD(0xd0);    // in fairness, this might write to 0x1B80, but if that's an issue then we'll worry about it

    firstSprite += 8;
    if (firstSprite >= ENT_ENTSNUM*4) {
        firstSprite -= ENT_ENTSNUM*4;
    }

    VDP_INT_ENABLE;

#endif
}

// 0 is off, anything else is on
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

// copy to the three tables of the bitmap screen
// TODO: if not F18A
// NOTE: very likely a different bank is mapped in!!
// VDP INTS MUST BE DISABLED
void bitmapcharcopy(U16 adr, const U8* buf, U16 size) {
#ifdef CLASSIC99
    if (classic99InterruptState) {
        sys_printf("WARNING: bitmapcharcopy with interrupts enabled\n");
    }
#endif

    vdpmemcpy(adr, buf, size);
    vdpmemcpy(adr+0x800, buf, size);
    vdpmemcpy(adr+0x1000, buf, size);
}


/* eof */



