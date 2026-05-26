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

#ifdef GFXTI
#include <vdp.h>
#endif

#include "img.h"
#include "fb.h"

#ifndef GFXTI
img_t *IMG_SPLASH;
#endif

/*
 * paints an image of size <width>,<height> with data in <pic> at
 * position <x>,<y> (fb/px).
 */
#ifdef GFXST
void img_paintPic(U16 x, U16 y, U16 width, U16 height, U32 *pic)
{
	U8 *f, *fb;
	U16 i, j, k, pp;
	U32 v;

	fb = fb_at(x, y);
	pp = 0;

	for (i = 0; i < height; i++) /* rows */
	{
		f = fb;
		for (j = 0; j < width; j += 8) /* cols */
		{
			v = pic[pp++];
			for (k = 8; k--; v >>=4)
				f[k] = v & 0x0F;
			f += 8;
		}
		fb += FB_WIDTH;
	}
}

#endif

#ifndef GFXTI
/*
 * paints image <img> onto the frame buffer.
 * the image must have the appropriate size.
 * also manages palettes.
 */
void img_paintImg(img_t *img)
{
	U16 k;
	U8 *fb;

	fb = fb_at(0, 0);

	fb_setPaletteFromImg(img);
	for (k = 0; k < FB_WIDTH * FB_HEIGHT; k++)
		fb[k] = img->pixels[k];
}
#endif

/*
 * paints an image of size <width>,<height> with data in <pic> at
 * position <x>,<y> (fb/px). NOTE: 8 pixel character bounardies!
 */
#ifdef GFXTI
void img_paintPic(U16 x, U16 y, U16 width, U16 height, U8 *pic, U8 *col)
{
    U16 i,v, v2;
    U16 sz = width/8;
    U8 chr;

    // using character offset, not bitmap bytes
    v = (y/8)*32 + (x/8);

    // this one is bitmap bytes, assuming bitmap layout
    chr = (U8)((y/8)*32+(x/8));
    v2 = gPattern + v*8;       // asuming pattern at 0 and color at 2000
    v += gImage;

    VDP_INT_DISABLE;
    for (i=0; i<height; i+=8) {
        vdpwriteinc(v, chr, sz);    // SIT
        vdpmemcpy(v2, pic, sz*8);   // pattern
        vdpmemcpy(v2+0x2000, col, sz*8);   // col
        pic+=sz;
        col+=sz;
        v+=32;
        chr+=32;
    }
    VDP_INT_ENABLE;
}

// fullscreen image
void img_paintImg(img_t *img) {
    // TODO: for F18A mode we probably want to add the palette here
    img_paintPic(0, 0, img->w, img->h, img->pixels, img->colors);
}
#endif

/* eof */
