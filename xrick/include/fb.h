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

#ifndef _FB_H
#define _FB_H

#include "ricksystem.h"
#include "img.h"

// cells instead of pixels, but we'll maintain the code's pixel assumptions
#define FB_WIDTH 256
#define FB_HEIGHT 192

/*
 * returns the fb pointer at <x>, <y>.
 * <x>, <y> are fb-coordinates.
 */
#define fb_at(x,y) (((y)/8)*32+((x)/8))

/*
 * clears the frame buffer
 */
extern void fb_clear();

#endif

/* eof */
