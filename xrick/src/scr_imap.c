/*
 * xrick/src/scr_imap.c
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
#include "sysvid.h"
#include "sounds.h"

#include "game.h"
#include "draw.h"
#include "control.h"
#include "maps.h"
#include "tiles.h"
#include "sprites.h"
#include "fb.h"

#include <vdp.h>

/*
 * local vars
 */
static U16 step;              /* current step */
static U16 count;             /* number of loops for current step */
static U16 run;               /* 1 = run, 0 = no more step */
static U16 flipflop;           /* flipflop for top, bottom, left, right */
static U16 spnum;             /* sprite number */
static U16 spx, spdx;         /* sprite x position and delta */
static U16 spy, spdy;         /* sprite y position and delta */
static U16 spbase, spoffs;    /* base, offset for sprite numbers table */
static U16 seq = 0;            /* anim sequence */

/*
 * prototypes
 */
static void drawtb(void);
static void drawlr(void);
static void drawsprite(void);
static void drawcenter(void);
static void nextstep(void);
static void anim(void);
static void init(void);

typedef struct
{
	U8* title;
	U8* body;
} maps_intros_t;

const maps_intros_t maps_intros[] =
{
	{	/* south america */

		"@@@@@@SOUTH@AMERICA@1945@@@@@@" TILES_NULLCHAR,

		"RICK@DANGEROUS@CRASH@LANDS@HIS" TILES_CRLFCHAR
		"@PLANE@OVER@THE@AMAZON@WHILE@@" TILES_CRLFCHAR
		"@SEARCHING@FOR@THE@LOST@GOOLU@" TILES_CRLFCHAR
		"@@@@@@@@@@@@TRIBE.@@@@@@@@@@@@" TILES_CRLFCHAR TILES_CRLFCHAR
		"@BUT,@BY@A@TERRIBLE@TWIST@OF@@" TILES_CRLFCHAR
		"FATE@HE@LANDS@IN@THE@MIDDLE@OF" TILES_CRLFCHAR
		"@@@A@BUNCH@OF@WILD@GOOLUS.@@@@" TILES_CRLFCHAR TILES_CRLFCHAR
		"@CAN@RICK@ESCAPE@THESE@ANGRY@@" TILES_CRLFCHAR
		"@@@AMAZONIAN@ANTAGONISTS@?@@@@" TILES_NULLCHAR
	},

	{	/* egypt */

		"@@@@EGYPT,@SOMETIMES@LATER@@@@" TILES_NULLCHAR,

		"RICK@HEADS@FOR@THE@PYRAMIDS@AT" TILES_CRLFCHAR
		"@@@@THE@REQUEST@OF@LONDON.@@@@" TILES_CRLFCHAR TILES_CRLFCHAR
		"HE@IS@TO@RECOVER@THE@JEWEL@OF@" TILES_CRLFCHAR
		"ANKHEL@THAT@HAS@BEEN@STOLEN@BY" TILES_CRLFCHAR
		"FANATICS@WHO@THREATEN@TO@SMASH" TILES_CRLFCHAR
		"@IT,@IF@A@RANSOM@IS@NOT@PAID.@" TILES_CRLFCHAR TILES_CRLFCHAR
		"CAN@RICK@SAVE@THE@GEM,@OR@WILL" TILES_CRLFCHAR
		"HE@JUST@GET@A@BROKEN@ANKHEL@?@" TILES_NULLCHAR
	},

	{	/* europe: castle */

		"@@@@EUROPE,@LATER@THAT@WEEK@@@" TILES_NULLCHAR,

		"@@RICK@RECEIVES@A@COMMUNIQUE@@" TILES_CRLFCHAR
		"@@FROM@BRITISH@INTELLIGENCE@@@" TILES_CRLFCHAR
		"@@ASKING@HIM@TO@RESCUE@ALLIED@" TILES_CRLFCHAR
		"@PRISONERS@FROM@THE@NOTORIOUS@" TILES_CRLFCHAR
		"@@@@SCHWARZENDUMPF@CASTLE.@@@@" TILES_CRLFCHAR TILES_CRLFCHAR
		"@@RICK@ACCEPTS@THE@MISSION.@@@" TILES_CRLFCHAR TILES_CRLFCHAR
		"@@@BUT@CAN@HE@LIBERATE@THE@@@@" TILES_CRLFCHAR
		"@CRUELLY@CAPTURED@COMMANDOS@?@" TILES_NULLCHAR
	},

	{	/* europe: missile base */

		"@@@@@@EUROPE,@EVEN@LATER@@@@@@" TILES_NULLCHAR,

		"RICK@LEARNS@FROM@THE@PRISONERS" TILES_CRLFCHAR
		"@THAT@THE@ENEMY@ARE@TO@LAUNCH@" TILES_CRLFCHAR
		"AN@ATTACK@ON@LONDON@FROM@THEIR" TILES_CRLFCHAR
		"@@@@@SECRET@MISSILE@BASE.@@@@@" TILES_CRLFCHAR TILES_CRLFCHAR
		"WITHOUT@HESITATION,@HE@DECIDES" TILES_CRLFCHAR
		"@@@TO@INFILTRATE@THE@BASE.@@@@" TILES_CRLFCHAR TILES_CRLFCHAR
		"CAN@RICK@SAVE@LONDON@IN@TIME@?" TILES_NULLCHAR
	},

	{	/* much much later */

		"@@@LONDON,@MUCH,@MUCH@LATER@@@" TILES_NULLCHAR,

		"@RICK@RETURNS@TO@A@TRIUMPHANT@" TILES_CRLFCHAR
		"@@WELCOME@HOME@HAVING@HELPED@@" TILES_CRLFCHAR
		"@@@@SECURE@ALLIED@VICTORY.@@@@" TILES_CRLFCHAR TILES_CRLFCHAR
		"BUT,@MEANWHILE,@IN@SPACE,@THE@" TILES_CRLFCHAR
		"@@@MASSED@STARSHIPS@OF@THE@@@@" TILES_CRLFCHAR
		"@@@BARFIAN@EMPIRE@ARE@POISED@@" TILES_CRLFCHAR
		"@@@@@TO@INVADE@THE@EARTH.@@@@@" TILES_CRLFCHAR TILES_CRLFCHAR
		"@WHAT@WILL@RICK@DO@NEXT@...@?@" TILES_NULLCHAR
	}
};

/*
 * map intro, step offset per map
 */
const U8 screen_imapsofs[] = {
  0x00, 0x03, 0x07, 0x0a, 0x0f
};


/*
 * Map introduction
 *
 * return: SCREEN_RUNNING, SCREEN_DONE, SCREEN_EXIT
 */
U16 screen_introMap(void)
{
	switch (seq)
	{
		case 0: /* initialize */
			fb_clear();
			sysvid_setGamma(GAMMA_OFF);

			tiles_setBank(0);
			tiles_paintListAt(maps_intros[env_map].title, 0, 0);
			tiles_paintListAt(maps_intros[env_map].body, 0, 96);

			init();
			nextstep();
			drawcenter();
			drawtb();
			drawlr();
			drawsprite();
			control_last = 0;

#ifdef ENABLE_SOUND
            nOldBank = nBank;
			sounds_setMusic(map_maps[env_map].tune, 1);
#endif
            sysvid_setGamma(GAMMA_ON);
			seq = 10;
			break;

		case 10:  /* top and bottom borders */
			if (control_status & CONTROL_FIRE)
			{
				seq = 20;
			}
			else
			{
				drawtb();
				seq = 12;
			}
			break;

		case 12:  /* background and sprite */
			anim();
			drawcenter();
			drawsprite();
			seq = 13;
			break;

		case 13:  /* all borders */
			drawtb();
			drawlr();
			seq = 10;
			break;

		case 20:  /* wait for key release */
			if (!(control_status & CONTROL_FIRE)) {
				seq = 30;
            } else {
				sys_sleep(50);
            }
			break;
	}

	if (control_status & CONTROL_EXIT)  /* check for exit request */
		return SCREEN_EXIT;

	if (seq == 30)
	{
		fb_clear();
		sysvid_setGamma(GAMMA_ON);
		seq = 0;
		return SCREEN_DONE;
	}
	else
		return SCREEN_RUNNING;
}


/*
 * Display top and bottom borders (0x1B1F)
 *
 */
static void drawtb(void)
{
	flipflop++;
    
    VDP_INT_DISABLE;

	if (flipflop & 0x01)
	{
        hchar(2, 13, 0x40, 6);
        hchar(9, 13, 0x06, 6);
	}
	else
	{
        hchar(2, 13, 0x05, 6);
        hchar(9, 13, 0x40, 6);
	}

    VDP_INT_ENABLE;
}


/*
 * Display left and right borders (0x1B7C)
 *
 */
static void drawlr(void)
{
    VDP_INT_DISABLE;

	if (flipflop & 0x02)
	{
        vchar(2, 12, 4, 8);
        vchar(2, 19, 4, 8);
	}
	else
	{
        vchar(2, 12, 0x2b, 8);
        vchar(2, 19, 0x2b, 8);
	}

    VDP_INT_ENABLE;
}


/*
 * Draw the sprite (0x19C6)
 *
 */
static void drawsprite(void)
{
	U16 x = 104 + ((spx << 1) & 0x1C);
	U16 y = 24 + (spy << 1);
	sprites_paint(spnum, 0, x, y, 1);  // always load the pattern - we can afford the waste here
}


/*
 * Draw the background (0x1AF1)
 *
 */
static void drawcenter(void)
{
	static U16 tn0[] = { 0x07, 0x5B, 0x7F, 0xA3, 0xC7 };
	U16 i;
    U16 tn;

	tn = tn0[env_map];

    VDP_INT_DISABLE;

	for (i = 0; i < 6; i++) {
        vdpwriteinc(fb_at(104, 24+8*i)+gImage, tn, 6);
        tn+=6;
    }

    VDP_INT_ENABLE;
}


/*
 * Next Step (0x1A74)
 *
 */
static void nextstep(void)
{
	if (screen_imapsteps[step].count)
	{
		count = screen_imapsteps[step].count;
		spdx = screen_imapsteps[step].dx;
		spdy = screen_imapsteps[step].dy;
		spbase = screen_imapsteps[step].base;
		spoffs = 0;
		step++;
	}
	else
	{
		run = 0;
	}
}


/*
 * Anim (0x1AA8)
 *
 */
static void anim(void)
{
	U16 i;

	if (run)
	{
		i = screen_imapsl[spbase + spoffs];
		if (i == 0)
		{
			spoffs = 0;
			i = screen_imapsl[spbase];
		}
		spnum = i;
        spoffs++;
		spx += spdx;
		spy += spdy;
		count--;
		if (count == 0)
			nextstep();
	}
}


/*
 * Initialize (0x1A43)
 *
 */
static void init(void)
{
	run = 0; run--;
	step = screen_imapsofs[env_map];
	spx = screen_imapsteps[step].dx;
	spy = screen_imapsteps[step].dy;
	step++;
	spnum = 0; /* NOTE spnum in [8728] is never initialized ? */
}

/* eof */
