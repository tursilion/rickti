/*
 * xrick/src/scr_getname.c
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

#include "screens.h"

#include "game.h"
#include "draw.h"
#include "control.h"
#include "tiles.h"
#include "fb.h"
#include "sysvid.h"
#include <vdp.h>
#include <string.h>

/*
 * local vars
 */
static U16 seq = 0;
static U16 x, y, p;
static U8 name[10];

#define TILE_POINTER '\072'
#define TILE_CURSOR '\073'
#define TOPLEFT_X 84
#define TOPLEFT_Y 56
#define NAMEPOS_X 88
#define NAMEPOS_Y 152
#define AUTOREPEAT_TMOUT 500


/*
 * prototypes
 */
static void pointer_show(U16);
static void name_update(void);
static void name_draw(void);

/*
 * Get name
 *
 * return: 0 while running, 1 when finished.
 */
U16 screen_getname(void) {
#if 0
    // TODO: temporarily removed to save code space
    return SCREEN_DONE;
#else
    static U16 tm = 0;
    U16 i, j;

    if (seq == 0) {
      /* figure out if this is a high score */
        if (env_score_hi < game_hscores[7].score_hi) {
            return SCREEN_DONE;
        }
        if ((env_score_hi == game_hscores[7].score_hi) && (env_score_lo < game_hscores[7].score_lo)) {
            return SCREEN_DONE;
        }

        /* prepare */
        tiles_setBank(0);

        //for (i = 0; i < 10; i++)
        //  name[i] = '@';
        memset(name, '@', 10);

        x = y = p = 0;
        seq = 1;
    }

    switch (seq) {
        case 1:  /* prepare screen */
            fb_clear();
            tiles_paintListAt((U8*)"PLEASE@ENTER@YOUR@NAME\376", 44, 32);

            VDP_INT_DISABLE;

            for (i = 0; i < 6; i++) {
                for (j = 0; j < 4; j++) {
                    VDP_SET_ADDRESS_WRITE(gImage + fb_at(TOPLEFT_X + i * 8 * 2, TOPLEFT_Y + j * 8 * 2));
                    VDPWD('A' + i + j * 6);
                }
            }

            VDP_INT_ENABLE;

            tiles_paintListAt((U8*)"Y@Z@.@@@\074\373\374\375\376", TOPLEFT_X, TOPLEFT_Y + 64);
            name_draw();
            pointer_show(TRUE);
            sysvid_setGamma(GAMMA_ON);
            seq = 2;
            break;

            // TODO: check autorepeat delay or better, kill autorepeat
        case 2:  /* wait for key pressed */
            if (control_status & CONTROL_FIRE)
                seq = 3;
            if (control_status & CONTROL_UP) {
                if (y > 0) {
                    pointer_show(FALSE);
                    y--;
                    pointer_show(TRUE);
                    tm = sys_gettime();
                }
                seq = 4;
            }
            if (control_status & CONTROL_DOWN) {
                if (y < 4) {
                    pointer_show(FALSE);
                    y++;
                    pointer_show(TRUE);
                    tm = sys_gettime();
                }
                seq = 5;
            }
            if (control_status & CONTROL_LEFT) {
                if (x > 0) {
                    pointer_show(FALSE);
                    x--;
                    pointer_show(TRUE);
                    tm = sys_gettime();
                }
                seq = 6;
            }
            if (control_status & CONTROL_RIGHT) {
                if (x < 5) {
                    pointer_show(FALSE);
                    x++;
                    pointer_show(TRUE);
                    tm = sys_gettime();
                }
                seq = 7;
            }
            if (seq == 2)
                sys_sleep(50);
            break;

        case 3:  /* wait for FIRE released */
            if (!(control_status & CONTROL_FIRE)) {
                if (x == 5 && y == 4) {  /* end */
                    i = 0;
                    while ((env_score_hi < game_hscores[i].score_hi) || ((env_score_hi == game_hscores[i].score_hi) && (env_score_lo < game_hscores[i].score_lo))) {
                        i++;
                    }
                    j = 7;
                    while (j > i) {
                        game_hscores[j].score_hi = game_hscores[j - 1].score_hi;
                        game_hscores[j].score_lo = game_hscores[j - 1].score_lo;
                        for (x = 0; x < 10; x++)
                            game_hscores[j].name[x] = game_hscores[j - 1].name[x];
                        j--;
                    }
                    game_hscores[i].score_hi = env_score_hi;
                    game_hscores[i].score_lo = env_score_lo;
                    for (x = 0; x < 10; x++)
                        game_hscores[i].name[x] = name[x];
                    seq = 99;
                } else {
                    name_update();
                    name_draw();
                    seq = 2;
                }
            } else
                sys_sleep(50);
            break;

        case 4:  /* wait for UP released */
            if (!(control_status & CONTROL_UP) ||
            sys_gettime() - tm > AUTOREPEAT_TMOUT)
                seq = 2;
            else
                sys_sleep(50);
            break;

        case 5:  /* wait for DOWN released */
            if (!(control_status & CONTROL_DOWN) ||
            sys_gettime() - tm > AUTOREPEAT_TMOUT)
                seq = 2;
            else
                sys_sleep(50);
            break;

        case 6:  /* wait for LEFT released */
            if (!(control_status & CONTROL_LEFT) ||
            sys_gettime() - tm > AUTOREPEAT_TMOUT)
                seq = 2;
            else
                sys_sleep(50);
            break;

        case 7:  /* wait for RIGHT released */
            if (!(control_status & CONTROL_RIGHT) ||
            sys_gettime() - tm > AUTOREPEAT_TMOUT)
                seq = 2;
            else
                sys_sleep(50);
            break;

    }

    if (control_status & CONTROL_EXIT)  /* check for exit request */
        return SCREEN_EXIT;

    if (seq == 99) {  /* seq 99, we're done */
        fb_clear();
        seq = 0;
        return SCREEN_DONE;
    } else
        return SCREEN_RUNNING;
#endif
}


static void pointer_show(U16 show) {
    VDP_INT_DISABLE;
    vdpchar(gImage + fb_at(TOPLEFT_X + x * 8 * 2, TOPLEFT_Y + y * 8 * 2 + 8), show == TRUE ? TILE_POINTER : '@');
    VDP_INT_ENABLE;
}

static void name_update(void) {
    U16 i;

    i = x + y * 6;
    if (i < 26 && p < 10)
        name[p++] = 'A' + i;
    if (i == 26 && p < 10)
        name[p++] = '.';
    if (i == 27 && p < 10)
        name[p++] = '@';
    if (i == 28 && p > 0) {
        name[p]='@';
        p--;
    }
    if (p == 10) {
        // jump to end
        pointer_show(FALSE);
        x = 5;
        y = 4;
        pointer_show(TRUE);
    }
}

/* P is how many letters we have so far */
static void name_draw(void) {
    VDP_INT_DISABLE;

#if 0
    // draw name letters
    for (i = 0; i < p; i++)
        tiles_paintAt(name[i],     NAMEPOS_X + i * 8, NAMEPOS_Y);

    // fill in the rest with the tile mark
    for (i = p; i < 10; i++)
        tiles_paintAt(TILE_CURSOR, NAMEPOS_X + i * 8, NAMEPOS_Y);

    // draw blanks underneath (to erase the marker)
    for (i = 0; i < 10; i++)
        tiles_paintAt('@', NAMEPOS_X + i * 8, NAMEPOS_Y + 8);

    // draw the marker
    tiles_paintAt(TILE_POINTER, NAMEPOS_X + 8 * (p < 9 ? p : 9), NAMEPOS_Y + 8);
#else
    // draw name letters
    if (p > 0) vdpmemcpy(fb_at(NAMEPOS_X, NAMEPOS_Y) + gImage, name, p);

    // fill in the rest with the tile mark
    if (p < 10) vdpmemset(fb_at(NAMEPOS_X, NAMEPOS_Y) + gImage + p, TILE_CURSOR, 10 - p);

    // draw blanks underneath (to erase the marker)
    vdpmemset(fb_at(NAMEPOS_X, NAMEPOS_Y + 8) + gImage, '@', 10);

    // draw the marker
    vdpchar(gImage + fb_at(NAMEPOS_X, NAMEPOS_Y + 8) + (p < 9 ? p : 9), TILE_POINTER);
#endif

    VDP_INT_ENABLE;
}


/* eof */
