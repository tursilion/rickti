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
#ifdef CLASSIC99
    // we don't have a interrupt sprite copy loop anymore, so copy it here
    // Well, the first part of it anyway
    VDP_INT_DISABLE;
    vdpmemcpy(gSprite,(unsigned char*)&sprite_table[0], 128);
    VDP_INT_ENABLE;
#endif
}    // game_rects - can delete this (except using it in the classic99 version
void sysvid_shutdown(void) { }  // nothing to really do here

// game often uses 0 and 255 to turn screen on and off, so we could implement that!
// all non zero is treated as visible
void sysvid_setGamma(U8 g) {
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



