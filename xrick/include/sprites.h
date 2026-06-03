/*
 * xrick/include/sprites.h
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
 * NOTES -- PC version
 *
 * A sprite consists in 4 columns and 0x15 rows of (U16 mask, U16 pict),
 * each pair representing 8 pixels (cga encoding, two bits per pixels).
 * Sprites are stored in 'sprites.bin' and are loaded by spr_init. Memory
 * is freed by spr_shutdown.
 *
 * There are four sprites planes. Plane 0 is the raw content of 'sprites.bin',
 * and planes 1, 2 and 3 contain copies of plane 0 with all sprites shifted
 * 2, 4 and 6 pixels to the right.
 */


#ifndef _SPRITES_H_
#define _SPRITES_H_

#include "config.h"
#include "ents.h"

/*
 * methods
 */
void sprites_setDepth(U8);
void sprites_paint(U16, U16, U16);
void sprites_paint2(U16, U16, U16);

void sprites_clear();


#define SPRITES_NBR_SPRITES (0xD5)
#define SPRITE_SIZE (8*4*4)
#define SPRITE_PAGE_SIZE (6*8)
#define SPRITE_FINAL_SIZE (21)

typedef U8 sprite_t;

extern const sprite_t sprites_data0[SPRITE_PAGE_SIZE*SPRITE_SIZE];
extern const sprite_t sprites_data1[SPRITE_PAGE_SIZE*SPRITE_SIZE];
extern const sprite_t sprites_data2[SPRITE_PAGE_SIZE*SPRITE_SIZE];
extern const sprite_t sprites_data3[SPRITE_PAGE_SIZE*SPRITE_SIZE];
extern const sprite_t sprites_data4[SPRITE_FINAL_SIZE*SPRITE_SIZE];

typedef struct spr_ {
    U8 y;
    U8 x;
    U8 ch;
    U8 col;
} sprite_data_t;

extern sprite_data_t sprite_table[(ENT_ENTSNUM+1)*4];


#endif

/* eof */

