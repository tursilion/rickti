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

#include <vdp.h>
#include "img.h"
#include "fb.h"

/*
 * paints an image of size <width>,<height> with data in <pic> at
 * position <x>,<y> (fb/px).
 */
/*
 * paints an image of size <width>,<height> with data in <pic> at
 * position <x>,<y> (fb/px). NOTE: 8 pixel character bounardies!
 * if either pic or col is 0, don't copy that part (we still rewrite the SIT, but that's okay)
 * If it is too slow, we can rewrite the SIT only for the pattern part
 */
void img_paintPic(U16 x, U16 y, U16 width, U16 height, const U8* pic, const U8* col, U16 chroff)
{
    U16 i,v, v2;
    U16 sz = width/8;
    U8 chr;     // starting character for SIT write

    // using character offset, not bitmap bytes
    v = (y/8)*32 + (x/8) + chroff;

    // this one is bitmap bytes, assuming bitmap layout
    chr = (U8)(v);
    v2 = gPattern + v*8;       // asuming pattern at 0 and color at 2000
    v += gImage - chroff;      // preserve position - assume caller knows what they are doing

    VDP_INT_DISABLE;
    for (i=0; i<height; i+=8) {
        vdpwriteinc(v, chr, sz);    // SIT
        if (pic) {
            vdpmemcpy(v2, pic, sz*8);   // pattern
            pic += sz*8;
        }
        if (col) {
            vdpmemcpy(v2+0x2000, col, sz*8);   // col
            col += sz*8;
        }
        v+=32;      // one full row down
        v2+=32*8;   // one full row down
        chr+=32;
    }
    VDP_INT_ENABLE;
}

// fullscreen image
//void img_paintImg(img_t *img) {
//    // TODO: for F18A mode we probably want to add the palette here
//    img_paintPic(0, 0, img->w, img->h, img->pixels, img->colors);
//}

/* eof */
