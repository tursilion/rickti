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

// not that we'll ever be able to use proper ASCII, but hey...
U8 env_digits = 48;


/*
 * FIXME counters positions in fp/px
 */
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

    VDP_SET_ADDRESS_WRITE(gImage);

    // cheats
    VDPWD(env_trainer?env_digits+1:0);
    VDPWD(env_invicible?env_digits+1:0);
    VDPWD(env_highlight?env_digits+1:0);

    // score
    VDPWD(env_digits+(env_score_hi/10));
    VDPWD(env_digits+(env_score_hi%10));
    U16 x = env_score_lo;
    VDPWD(env_digits+(x/1000));
    x %= 1000;
    VDPWD(env_digits+(x/100));
    x %= 100;
    VDPWD(env_digits+(x/10));
    VDPWD(env_digits+(x%10));

    VDPWD(0);

    // bullets
    for (U16 i=0; i<6; ++i) {
        // +9 because the defines are 1 based
        VDPWD(env_bullets>i ? TILES_BULLET+env_digits+9 : 0);
    }

    VDPWD(0);

    // bombs
    for (U16 i=0; i<6; ++i) {
        VDPWD(env_bombs>i ? TILES_BOMB+env_digits+9 : 0);
    }

    VDPWD(0);

    // ricks
    for (U16 i=0; i<6; ++i) {
        VDPWD(env_lives>i ? TILES_RICK+env_digits+9 : 0);
    }

    VDP_INT_ENABLE;
}

/* eof */