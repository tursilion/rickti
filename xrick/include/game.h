/*
 * xrick/include/game.h
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

#ifndef _GAME_H
#define _GAME_H

#include <stddef.h> /* NULL */

#include "ricksystem.h"
#include "syssnd.h"
#include "maps.h"

#define LEFT 1
#define RIGHT 0

// defines the minimum frame time. But due to 60hz counting, 75 is actually closer to 83 when adjusted to frames (12fps).
// we count at 16.6ms per frame. 75 wants 13.3fps. 66 should be closer to 15fps.
#define GAME_PERIOD 66

#define GAME_BOMBS_INIT 6
#define GAME_BULLETS_INIT 6

typedef struct {
  U16 score_hi; // and above
  U16 score_lo; // 0-9999
  U8 name[11];
} hscore_t;

extern hscore_t game_hscores[8];  /* highest scores (hall of fame) */
extern const map_t map_maps[MAP_NBR_MAPS];

extern U16 game_dir;        /* direction (LEFT, RIGHT) */

extern U16 game_waitevt;    /* wait for events (TRUE, FALSE) */
extern U16 game_period;     /* time between each frame, in millisecond */

extern void draw_titlepage();
extern void draw_hof_title();

extern void game_run(char *path);
extern void game_toggleCheat(U16);

#endif

/* eof */


