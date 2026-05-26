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

#ifndef _IMG_H
#define _IMG_H

#include "config.h"

/* a color */
#ifndef GFXTI
typedef struct {
  U8 r, g, b, nothing;
} img_color_t;
#else
typedef U8 img_color_t;
#endif

/* an image */
typedef struct {
  U16 w, h;
  U16 ncolors;
  const img_color_t *colors;
  const U8 *pixels;
} img_t;

/* the splash image */
extern img_t *IMG_SPLASH;

/*
 * paints image <img> onto the frame buffer.
 * the image must have the appropriate size.
 * also manages palettes.
 */
extern void img_paintImg(img_t *);

#ifdef GFXST
/*
 * paints an image of size <width>,<height> with data in <pic> at
 * position <x>,<y> (fb/px).
 */
extern void img_paintPic(U16, U16, U16, U16, U32 *);
#endif

#ifdef GFXTI
/*
 * copy a bitmap image of size <width>,<height> with data in <pic> and color at <col> at
 * position <x>,<y> (fb/px). NOTE: character (8x8) boundaries!
 */
extern void img_paintPic(U16, U16, U16, U16, const U8*, const U8*);
#endif

#endif /* _IMG_H */


/* eof */
