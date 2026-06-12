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

#ifndef _TILES_H
#define _TILES_H

#include "config.h"

/*
 * methods
 */
void loadDigitTiles(void);
void loadStringTiles(const char* str);
void tiles_setBank(U16);
void tiles_setFilter(U16);
int tiles_paintList(U8* , int);
void tiles_paintListAt(U8* , U16, U16);

/*
 * one single tile
 *
 * a tile is 8x8 pixels.
 * PC: CGA encoding = 2 bits per pixel, one U16 per line.
 * ST: encoding = 4 bits per pixel, one U32 per line.
 * TI: bitmap encoding, 8 bytes pattern then 8 bytes color in two separate tables, tile_t here is one byte, not a full tile
 */
typedef U8 tile_t;

/*
 * banks (each bank contains 256 (0x100) tiles)
 *
 * FIXME is this true?
 * bank 0: tiles for main intro
 * bank 1: tiles for map intro (skipped as per ST version)
 * bank 2: unused
 * bank 3: game tiles, page 0
 * bank 4: game tiles, page 1
 */
#define TILES_BANKS_COUNT 3
extern const tile_t tiles_banks_col[TILES_BANKS_COUNT*256*8];
extern const tile_t tiles_banks_pat[TILES_BANKS_COUNT*256*8];

/*
 * special tile numbers
 */
#define TILES_BULLET 0x01
#define TILES_BOMB 0x02
#define TILES_RICK 0x03

#define TILES_NULL 0xfe
#define TILES_NULLCHAR "\376"
#define TILES_CRLF 0xff
#define TILES_CRLFCHAR "\377"

#endif

/* eof */
