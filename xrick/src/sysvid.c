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
#include "sysarg.h"

#include <vdp.h>

#ifdef F18A
#include <f18a.h>
#endif

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
// NOTE: very likely a different bank is mapped in!!
// Will pause periodically to allow interrupts so title music plays clean
void bitmapcharcopy(U16 adr, const U8* buf, U16 size) {
    // we'll copy 0x200 bytes at a time, we need to know how much extra there is
    U16 remsize = size%0x200;
    size -= remsize;

    VDP_INT_DISABLE;

    for (U16 outer=adr; outer<adr+0x1800; outer+=0x800) {
        for (U16 off=0; off<size; off+=0x200) {
            vdpmemcpy(outer+off, buf+off, 0x200);
            VDP_INT_POLL;
        }
        if (remsize > 0) {
            vdpmemcpy(outer+size, buf+size, remsize);
            VDP_INT_POLL;
        }
#ifdef F18A
        // only one table copy in half bitmap mode
        if (sysarg_half_bitmap) break;
#endif
    }

    VDP_INT_ENABLE;
}

#ifdef F18A
// F18A functions to set and clear half bitmap - assumes we are already setup in normal bitmap
// Make sure interrupts are disabled!
void set_halfbitmap() {
    // switch to half bitmap mode - no sprite limits with F18A!
	VDP_SET_REGISTER(VDP_REG_CT, 0x9F);
	VDP_SET_REGISTER(VDP_REG_PDT, 0x00);
    VDP_SET_REGISTER(F18A_REG_ECM, 0x03);   // enable 8 color sprites
    vdpmemset(gSpritePat, 0, 0x1800);    // make sure it's zeroed (all three tables)
    sysarg_half_bitmap = 1;
}

void set_fullbitmap() {
	VDP_SET_REGISTER(VDP_REG_CT, 0xFF);
	VDP_SET_REGISTER(VDP_REG_PDT, 0x03);
    VDP_SET_REGISTER(F18A_REG_ECM, 0x0);   // disable 8 color sprites
    sysarg_half_bitmap = 0;
}

#endif

/* eof */



