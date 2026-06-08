/*
 * xrick/src/scr_imain.c
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

#include <vdp.h>
#include <conio.h>
#include <string.h>

#include "game.h"
#include "screens.h"
#include "sysvid.h"
#include "sounds.h"

#include "draw.h"
#include "pics.h"
#include "control.h"
#include "fb.h"

#include "tiles.h"

#define IMAIN_PERIOD 50;

void draw_titlepage()
{
    unsigned int nOldBank = 0;

    nOldBank = nBank;
    // pattern
    SWITCH_IN_BANK10;
	img_paintPic(0, 0, 256, 192, (U8*)pic_splash_pat, 0, 0);
    // color
    SWITCH_IN_BANK11;
	img_paintPic(0, 0, 256, 192, 0, (U8*)pic_splash_col, 0);
    // restore
    SWITCH_IN_BANK(nOldBank);
    // tell the tile banking page to reload on next request
    tiles_setBank(0xff);
}

/*
 * Main introduction
 *
 * return: SCREEN_RUNNING, SCREEN_DONE, SCREEN_EXIT
 */
U16 screen_introMain(void)
{
	static U16 seq = 0;
	static U16 seen = 0;
	static U16 first = TRUE;
	static U16 period = 0;
	static U32 tm = 0;
    static U8 workbuf[8];   // accelerate the VDP fat font code
    static U8 workbuf2[8];  // accelerate the VDP fat font code
    unsigned int nOldBank = 0;
	U16 i;
    //U16 s[32];

	if (seq == 0)
	{
		tiles_setBank(0);
		if (first == TRUE)
			seq = 1;
		else
			seq = 10;
		period = game_period;
		game_period = IMAIN_PERIOD;
#ifdef ENABLE_SOUND
		sounds_setMusic("sounds/tune5.wav", -1);
#endif
	}

	switch (seq)
	{
		case 1:  /* title */
			fb_clear();
			sysvid_setGamma(GAMMA_OFF);
			tm = sys_gettime();

            draw_titlepage();

			game_period = IMAIN_PERIOD;
            sysvid_setGamma(GAMMA_ON);
			seq = 3;
			break;

		case 3:  /* wait for key pressed or timeout title */
			if (control_status & CONTROL_FIRE)
				seq = 4;
			else if (sys_gettime() - tm > SCREEN_TIMEOUT)
			{
				seen++;
				game_period = period/2;
				seq = 8;
			}
			break;

		case 4:  /* wait for key released title */
			if (!(control_status & CONTROL_FIRE))
			{
				if (seen++ == 0)
					seq = 8;
				else
				{
					game_period = period/2;
					seq = 28;
				}
			}
			break;

		case 8: /* fade-out title */
            sysvid_setGamma(GAMMA_OFF);
    		game_period = IMAIN_PERIOD;
			seq = 10;
			break;

		case 10:  /* display hall of fame */
			fb_clear();
			tm = sys_gettime();
            
            // we need the main tiles loaded to see text!
            tiles_setBank(0);

			/* hall of fame title */
            nOldBank = nBank;
            // pattern
            SWITCH_IN_BANK10;
			img_paintPic(0, 0, 256, 24, (U8*)pic_haf_pat, 0, 128);
            // color
            SWITCH_IN_BANK11;
			img_paintPic(0, 0, 256, 24, 0, (U8*)pic_haf_col, 128);
            // restore
            SWITCH_IN_BANK(nOldBank);
            // tell the bank engine it needs to reload on next request
            tiles_setBank(0xff);

			/* hall of fame content */
            VDP_INT_DISABLE;
			for (i = 0; i < 8; i++)
			{
                // conio is slow, but this will work
                gotoxy(4, 5+i*2);
                cprintf("%02d%04d@@@....@@@%s",
					game_hscores[i].score_hi, game_hscores[i].score_lo, game_hscores[i].name);
			}
            VDP_INT_ENABLE;

            sysvid_setGamma(GAMMA_ON);
			game_period = IMAIN_PERIOD
			seq = 12;
			break;

		case 12:  /* wait for key pressed or timeout hof */
			if (control_status & CONTROL_FIRE)
				seq = 13;
			else if (sys_gettime() - tm > SCREEN_TIMEOUT)
			{
				seen++;
				seq = 18;
			}
			break;

		case 13:  /* wait for key released hof */
			if (!(control_status & CONTROL_FIRE))
			{
				if (seen++ == 0)
					seq = 18;
				else
				{
					game_period = period/2;
					seq = 28;
				}
			}
			break;

		case 18: /* fade-out hof */
            sysvid_setGamma(GAMMA_OFF);
			game_period = IMAIN_PERIOD;
			seq = 20;
			break;

        case 20:  /* credits page */
			fb_clear();
			tm = sys_gettime();
            
            // load the tileset and fake a small captital set
            tiles_setBank(0);
            // tell the system no valid tile bank is loaded anymore
            tiles_setBank(0xff);

            // patch up the character set
            U16 out='a';
            VDP_INT_DISABLE;

            // because the font was converted from an image, the colors are randomly forward and backwards
            // in order to make a small version, we need to keep the same color rows, or convert all the letters
            // This is still smaller than pulling in a new tileset or even using charsetlc
            workbuf[7]=0;   // leave the bottom row zeroed
            for (U16 i='/'; i<='Z'; ++i) {
                if (i == '/') {
                    memcpy(workbuf, "\x03\x06\x0c\x18\x30\x60\xc0", 7);
                } else {
                    vdpmemread(gPattern+i*8, workbuf, 7);
                    vdpmemread(gColor+i*8, workbuf2, 7);
                    for (U16 j=0; j<7; ++j) {
                        if (workbuf2[j] < 0xe0) {
                            // ie: if black is the foreground color, invert it
                            // we don't have to do color as we'll write the whole thing below
                            workbuf[j] = ~workbuf[j];
                        }
                    }
                }
                bitmapcharcopy(gPattern+i*8, workbuf, 8);
                if (i<'A') continue;
                // delete rows 1 and 5
                memmove(&workbuf[1], &workbuf[0], 5);
                workbuf[2]=workbuf[1];
                workbuf[0]=0;
                workbuf[1]=0;
                bitmapcharcopy(gPattern+out*8, workbuf, 8);
                ++out;
            }
            vdpmemset(gColor, 0xe0, 0x1800);


			/* credit content */
            // conio is slow, but this will work
            gotoxy(0, 5);
                   //012345678901234567890123456789012
            cprintf("This-is-a-port-of-xrick-by-Tursi");
            cprintf("/Atari-ST-version/\n\n");
            cprintf("github.com/tursilion/rickti\n\n");
            cprintf("Original-creator-Simon-Phipps\n\n");
            cprintf("xrick-by-BigOrno,-and-I-started\n");
            cprintf("with-the-port-by-Stephan-Gay\n\n");
            cprintf("Written-with-gcc,-convert9918\n");
            cprintf("vgmcomp2,-libTI99ALL-and\n");
            cprintf("Classic99\n\n");

            VDP_INT_ENABLE;

            sysvid_setGamma(GAMMA_ON);
			game_period = IMAIN_PERIOD;
			seq = 22;
			break;

		case 22:  /* wait for key pressed or timeout hof */
			if (control_status & CONTROL_FIRE)
				seq = 23;
			else if (sys_gettime() - tm > SCREEN_TIMEOUT)
			{
				seen++;
                sysvid_setGamma(GAMMA_OFF);
                game_period = IMAIN_PERIOD;
				seq = 1;
			}
			break;

		case 23:  /* wait for key released hof */
			if (!(control_status & CONTROL_FIRE))
			{
				game_period = period/2;
				seq = 28;
			}
			break;

		case 28: /* fade-out final */
			seq = 30;   // handled below outside of switch
			break;

	}

	if (control_status & CONTROL_EXIT)  /* check for exit request */
		return SCREEN_EXIT;

	if (seq == 30) /* we're done */
	{
		fb_clear();
		seq = 0;
		seen = 0;
		first = FALSE;
		game_period = period;
		sysvid_setGamma(GAMMA_ON);
		return SCREEN_DONE;
	}
	else
		return SCREEN_RUNNING;
}

/* eof */


