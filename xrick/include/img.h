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
typedef U16 img_color_t;

/* an image */
typedef struct {
  U16 w, h;
  U16 ncolors;
  const img_color_t *colors;
  const U8* pixels;
} img_t;

/*
 * paints image <img> onto the frame buffer.
 * the image must have the appropriate size.
 * also manages palettes.
 */
extern void img_paintImg(img_t *);

/*
 * copy a bitmap image of size <width>,<height> with data in <pic> and color at <col> at
 * position <x>,<y> (fb/px). NOTE: character (8x8) boundaries!
 */
extern void img_paintPic(U16 x, U16 y, U16 width, U16 height, const U8* pat, const U8* col, U16 chroff);

#endif /* _IMG_H */


/* eof */
