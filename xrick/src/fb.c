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
#include "rects.h"
#include "draw.h"
#include "game.h"

#ifndef GFXTI
#include <string.h> /* memset */
// note: we don't have a FB for the TI version, we just write direct to VRAM
U8 fb[FB_HEIGHT][FB_WIDTH];
#endif

/*
 * color tables (palettes)
 */
#ifdef GFXPC
#define FB_PALSZ 8
static U8 RED[] = { 0x00, 0x50, 0xf0, 0xf0, 0x00, 0x50, 0xf0, 0xf0 };
static U8 GREEN[] = { 0x00, 0xf8, 0x50, 0xf8, 0x00, 0xf8, 0x50, 0xf8 };
static U8 BLUE[] = { 0x00, 0x50, 0x50, 0x50, 0x00, 0xf8, 0xf8, 0xf8 };
#endif
#ifdef GFXST
// TODO: Need to use these palettes to convert the graphics for GFXTI
#define FB_PALSZ 32
static U8 RED[] = {		0x00, 0xd8, 0xb0, 0xf8,
						0x20, 0x00, 0x00, 0x20,
						0x48, 0x48, 0x90, 0xd8,
						0x48, 0x68, 0x90, 0xb0,
						/* highlight colors */
						0x50, 0xe0, 0xc8, 0xf8,
						0x68, 0x50, 0x50, 0x68,
						0x80, 0x80, 0xb0, 0xe0,
						0x80, 0x98, 0xb0, 0xc8
};
static U8 GREEN[] = {	0x00, 0x00, 0x6c, 0x90,
						0x24, 0x48, 0x6c, 0x48,
						0x6c, 0x24, 0x48, 0x6c,
						0x48, 0x6c, 0x90, 0xb4,
						/* highlight colors */
						0x54, 0x54, 0x9c, 0xb4,
						0x6c, 0x84, 0x9c, 0x84,
						0x9c, 0x6c, 0x84, 0x9c,
						0x84, 0x9c, 0xb4, 0xcc
};
static U8 BLUE[] = {	0x00, 0x00, 0x68, 0x68,
						0x20, 0xb0, 0xd8, 0x00,
						0x20, 0x00, 0x00, 0x00,
						0x48, 0x68, 0x90, 0xb0,
						/* highlight colors */
						0x50, 0x50, 0x98, 0x98,
						0x68, 0xc8, 0xe0, 0x50,
						0x68, 0x50, 0x50, 0x50,
						0x80, 0x98, 0xb0, 0xc8};
#endif



/*
 * returns the fb pointer at <x>, <y>.
 * <x>, <y> are fb-coordinates.
 */
U8 *fb_at(U16 x, U16 y)
{
#ifndef GFXTI
	return ((U8*)&fb) + x + y * FB_WIDTH;
	//return &fb + x + y * FB_WIDTH;
#else
    // return it as a character offset, gImage is not added here
    // Not into the tile set, but into the SIT
    return (U8*)((y/8)*32+(x/8));
#endif
}

/*
 * clears the frame buffer
 */
void fb_clear()
{
#ifndef GFXTI
	memset(fb, 0, FB_WIDTH * FB_HEIGHT);
#else
    // TODO: do we need to clear anything at all? If yes, to what tile?
    // VDP_INT_DISABLE;
    //vdpmemset(gImage, 0, 768);
    // VDP_INT_ENABLE;
#endif
}

/*
 * ramp the fb from black to visible.
 * returns TRUE when done, FALSE when ongoing.
 */
U8 fb_fadeIn()
{
#ifndef GFXTI
	static U8 fade = 0;

	while (fade < 8)
	{
		/* const = 255 * 2 / (max_fade+2) */
		sysvid_setGamma((U8)(56 + 255.0 * (1 - 2.0/(fade+2.0))));
		fade++;
		game_rects = &draw_SCREENRECT; // FIXME
		return FALSE;
	}

	fade = 0;
	sysvid_setGamma(255);
	return TRUE;
#else
    // TOOD: we could potentially do a pixel dither, loading the tile graphics.
    // If we did that, then we COULD enable fb_clear as clearing the tiles
    return TRUE;
#endif
}

/*
 * ramp the fb from visible to black.
 * returns TRUE when done, FALSE when ongoing.
 */
U8 fb_fadeOut()
{
#ifndef GFXTI
	static U8 fade = 0;

	while (fade < 8)
	{
		sysvid_setGamma((U8)(255.0 * 3.0/(fade+3.0)));
		fade++;
		game_rects = &draw_SCREENRECT; // FIXME
		return FALSE;
	}

	fade = 0;
	sysvid_setGamma(0);
	return TRUE;
#else
    // as with fadein
    return TRUE;
#endif
}

/*
 * sets fb visibility to black (FALSE) or full (TRUE).
 */
void fb_setVisible(U8 vis)
{
#ifndef GFXTI
	sysvid_setGamma(vis ? 255 : 0);
#else
    // TODO: we can use the enable/disable bit.
    // if we do, then we should also set it in fadein and fadeout above
#endif
}


/*
 * initializes the video layer with the game palette
 */
void fb_initPalette()
{
#ifndef GFXTI
	sysvid_setPaletteFromRGB(RED, GREEN, BLUE, FB_PALSZ);
#endif
}



/*
 * sets the palette from the image
 */
void fb_setPaletteFromImg(img_t* img)
{
#ifndef GFXTI
	sysvid_setPaletteFromImg(img);
#endif
}

/* eof */
