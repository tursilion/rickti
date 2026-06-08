/*
 * xrick/src/env.c
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
#include "env.h"

#include "fb.h"
#include "tiles.h"

#include <vdp.h>
#include <conio.h>

U16 env_trainer = FALSE;
U16 env_invicible = FALSE;
U16 env_highlight = FALSE;

U16 env_depth = TRUE;

U16 env_lives = 0;
U16 env_bombs = 0;
U16 env_bullets = 0;
U16 env_score_lo = 0;
U16 env_score_hi = 0;

U16 env_map = 0;
U16 env_submap = 0;
U16 env_changeSubmap = FALSE;


/*
 * FIXME counters positions in fp/px
 */
#define DRAW_STATUS_SCORE_X 24
#define DRAW_STATUS_BULLETS_X 80
#define DRAW_STATUS_BOMBS_X 136
#define DRAW_STATUS_LIVES_X 192
#define DRAW_STATUS_Y 0

// break up the 32 bit score into two 16s
void addscore(U16 val) {
    env_score_lo += val;
    if (env_score_lo > 9999) {
        env_score_lo -= 10000;
        env_score_hi++;
    }
}

/*
 * env_paintGame
 *
 * paints the game environment (score, lives, bullets, bombs).
 */
void env_paintGame(void)
{
    VDP_INT_DISABLE;
    // TODO: conio might be too slow here?
    // TODO: we don't have text or the correct characters in the game tileset - how do we get text?
    // bank 0 has font and cutscenes, but 2 and 3 have level graphics.
    gotoxy(DRAW_STATUS_SCORE_X/8, DRAW_STATUS_Y/8);
    cprintf("%02d%04d", env_score_hi, env_score_lo);
    hchar(DRAW_STATUS_Y/8, DRAW_STATUS_BULLETS_X/8, TILES_BULLET, env_bullets);
    hchar(DRAW_STATUS_Y/8, DRAW_STATUS_BOMBS_X/8, TILES_BOMB, env_bombs);
    hchar(DRAW_STATUS_Y/8, DRAW_STATUS_LIVES_X/8, TILES_RICK, env_lives);
    VDP_INT_ENABLE;
}

/* eof */