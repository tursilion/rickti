/*
 * xrick/src/scr_gameover.c
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

#include "stddef.h" /* NULL */

#include "config.h"
#include "game.h"
#include "screens.h"
#include "sysvid.h"
#include "sysarg.h"
#include "sounds.h"
#include "env.h"

#include "draw.h"
#include "control.h"
#include "tiles.h"
#include "fb.h"

/*
 * gameover
 * (from ds + 0x8810 + 0x2864)
 *
 * \376=0xfe \377=0xff
 */
static const U8 screen_gameovertxt[] = "\
@@@@@@@@@@@\377\
@GAME@OVER@\377\
@@@@@@@@@@@\376";

/*
 * Display the game over screen
 *
 * return: SCREEN_RUNNING, SCREEN_DONE, SCREEN_EXIT
 */
U16 screen_gameover(void)
{
	static U16 seq = 0;
	static U16 period = 0;
	static U32 tm = 0;

	if (seq == 0) {
#ifdef F18A
    // we need to make sure bitmap is in full bitmap mode
    // (the tables can't move, so no need to change the global pointers)
    VDP_INT_DISABLE;
    set_fullbitmap();
    tiles_setBank(0xff);    // force tile reload (only needed for game win)
    VDP_INT_ENABLE;
#endif

		tiles_setBank(0);
		seq = 1;
		period = game_period; /* save period, */
		game_period = 50;     /* and use our own */
#ifdef ENABLE_SOUND
		sounds_play(GAMEOVER_SND);
#endif
	}

	switch (seq) {
	case 1:  /* display banner */
		fb_clear();
		tm = sys_gettime();
		tiles_paintListAt((U8*)screen_gameovertxt, 88, 80);

        sysvid_setGamma(GAMMA_ON);
		seq = 2;
		break;

	case 2:  /* wait for key pressed */
		if (control_status & CONTROL_FIRE)
			seq = 3;
		else if (sys_gettime() - tm > SCREEN_TIMEOUT)
			seq = 4;
		else
			sys_sleep(50);
		break;

	case 3:  /* wait for key released */
		if (!(control_status & CONTROL_FIRE))
			seq = 4;
		else
			sys_sleep(50);
		break;
	}

	if (control_status & CONTROL_EXIT)  /* check for exit request */
		return SCREEN_EXIT;

	if (seq == 4) {  /* we're done */
		fb_clear();
		seq = 0;
		game_period = period;
		return SCREEN_DONE;
	}

  return SCREEN_RUNNING;
}

/* eof */

