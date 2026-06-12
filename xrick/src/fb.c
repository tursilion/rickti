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
#include "fb.h"

#include "sysvid.h"
#include "draw.h"
#include "game.h"

#include <vdp.h>
#include "sprites.h"

/*
 * clears the frame buffer
 */
void fb_clear()
{
    // Clear by wiping tile 0 (all three) and clear to tile 0
    // good place to wipe sprite table too
    VDP_INT_DISABLE;
    // Tile 0 should already be cleared in all tilesets
    vdpmemset(gImage, 0, 384);
    VDP_INT_POLL;   // split up for the title music
    vdpmemset(gImage+384, 0, 384);
    VDP_INT_ENABLE;
    // this is the CPU buffer, so it's safe without VDP
    sprites_clear();
}


/* eof */
