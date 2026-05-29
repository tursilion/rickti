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

#ifdef GFXTI
#include <vdp.h>
#include <conio.h>
#endif

U8 env_trainer = FALSE;
U8 env_invicible = FALSE;
U8 env_highlight = FALSE;

U8 env_depth = TRUE;

U8 env_lives = 0;
U8 env_bombs = 0;
U8 env_bullets = 0;
U16 env_score_lo = 0;
U16 env_score_hi = 0;

U16 env_map = 0;
U16 env_submap = 0;
U8 env_changeSubmap = FALSE;


/*
 * FIXME counters positions in fp/px
 */
#ifdef GFXPC
#define DRAW_STATUS_SCORE_X 0x28
#define DRAW_STATUS_BULLETS_X 0x68
#define DRAW_STATUS_BOMBS_X 0xA8
#define DRAW_STATUS_LIVES_X 0xE8
#define DRAW_STATUS_Y 0x08
#endif
#ifdef GFXST
#define DRAW_STATUS_SCORE_X 0x20
#define DRAW_STATUS_BULLETS_X 0x68
#define DRAW_STATUS_BOMBS_X 0xA8
#define DRAW_STATUS_LIVES_X 0xF0
#define DRAW_STATUS_Y 0
#endif
#ifdef GFXTI
#define DRAW_STATUS_SCORE_X 24
#define DRAW_STATUS_BULLETS_X 80
#define DRAW_STATUS_BOMBS_X 136
#define DRAW_STATUS_LIVES_X 192
#define DRAW_STATUS_Y 0
#endif

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
#ifndef GFXTI
	S8 i;
	U32 sv;
	static U8 s[7] = {0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0xfe};

	tiles_setBank(0);

    // TODO: this loop not ported to the 16 bit scores
	for (i = 5, sv = env_score; i >= 0; i--)
	{
		s[i] = 0x30 + (U8)(sv % 10);
		sv /= 10;
	}

	tiles_paintListAt(s, DRAW_STATUS_SCORE_X, DRAW_STATUS_Y);

	for (i = 0; i < env_bullets; i++)
		tiles_paintAt(TILES_BULLET, DRAW_STATUS_BULLETS_X + i * 8, DRAW_STATUS_Y);

	for (i = 0; i < env_bombs; i++)
		tiles_paintAt(TILES_BOMB, DRAW_STATUS_BOMBS_X + i * 8, DRAW_STATUS_Y);

	for (i = 0; i < env_lives; i++)
		tiles_paintAt(TILES_RICK, DRAW_STATUS_LIVES_X + i * 8, DRAW_STATUS_Y);
#else
    
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
#endif
}


/*
 * env_paintXtra
 *
 * paints the extra environment (cheats, modes...).
 */
void env_paintXtra(void)
{
	S8 i;
	U16 sv;
	static U8 s[8] = {'M', 0x30, 0x30, TILES_CRLF, 'S', 0x30, 0x30, TILES_NULL};
	static U8 c[8] = {'@', '@', '@', TILES_CRLF, '@', '@', '@', TILES_NULL};

	tiles_setBank(0);

#ifdef GFXPC
	tiles_setFilter(0xffff);
#endif

	tiles_paintAt(env_trainer ? 'T' : '@', 0, DRAW_STATUS_Y);
	tiles_paintAt(env_invicible ? 'I' : '@', 8, DRAW_STATUS_Y);
	tiles_paintAt(env_highlight ? 'H' : '@', 16, DRAW_STATUS_Y);

	/* show map number */
	for (i = 2, sv = env_map; i >= 1; i--) {
		s[i] = 0x30 + (U8)(sv % 10);
		sv /= 10;
	}

	/* show submap number */
	for (i = 6, sv = env_submap; i >= 5; i--) {
		s[i] = 0x30 + (U8)(sv % 10);
		sv /= 10;
	}

	tiles_paintListAt(s, 0, DRAW_STATUS_Y + 8*2);
}


/*
 * env_clearGame
 *
 * clears the game environment (repaints the map).
 */
void
env_clearGame(void)
{
	U16 i;
    U8 *f;

#ifdef GFXPC
	tiles_setBank(map_tilesBank);
#endif
#ifdef GFXST
	tiles_setBank(0);
#endif
#ifdef GFXTI
	tiles_setBank(0);
#endif

  f = fb_at(DRAW_STATUS_SCORE_X, DRAW_STATUS_Y);
#ifdef GFXPC
	for (i = 0; i < DRAW_STATUS_LIVES_X/8 + 6 - DRAW_STATUS_SCORE_X/8; i++)
		f = tile_paint(map_map[MAP_ROW_SCRTOP + (DRAW_STATUS_Y / 8)][i], f);
#endif
#ifdef GFXST
	for (i = 0; i < DRAW_STATUS_LIVES_X/8 + 6 - DRAW_STATUS_SCORE_X/8; i++)
		f = tiles_paint('@', f);
#endif
#ifdef GFXTI
	for (i = 0; i < DRAW_STATUS_LIVES_X/8 + 6 - DRAW_STATUS_SCORE_X/8; i++)
		f = tiles_paint('@', f);
#endif
}






/* eof */