/*
 * xrick/src/game.c
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

#include "sysarg.h"
#include "sysvid.h"
#include "sysevt.h"
#include "env.h"

#include "game.h"

#include "draw.h"
#include "maps.h"
#include "ents.h"
#include "sounds.h"
#include "e_rick.h"
#include "e_sbonus.h"
#include "e_them.h"
#include "screens.h"
#include "scroller.h"
#include "control.h"
#include "fb.h"
#include "tiles.h"
#include "draw.h"
#include "pics.h"
#include "tigrom.h"

#ifdef F18A
#define BIN2INC_HEADER_ONLY
#include "splashf18_splitcol1.c"
#include "splashf18_splitcol2.c"
#include "splashf18_splitcol3.c"
#include "splashf18_splitcol4.c"
#include "splashf18_splitpat1.c"
#include "splashf18_splitpat2.c"
#include "splashf18_splitpat3.c"
#include "splashf18_splitpat4.c"
#endif

/*
 * local typedefs
 */
typedef enum {
    MAIN_INTRO, MAP_INTRO,
    INIT,
    INIT_MAP, INIT_SUBMAP,
    FADEOUT__GAMEOVER,
    PAUSE_PRESSED1, PAUSE_PRESSED1B, PAUSED, PAUSE_PRESSED2,
    CTRL_ACTION, CTRL_PAUSE, CTRL_RICK, PAINT, CTRL_SCROLL,
    NEXT_SUBMAP, NEXT_MAP,
    SCROLL_UP, SCROLL_DOWN,
    RESTART, GAMEOVER, GETNAME, EXIT
} game_state_t;

const map_t map_maps[MAP_NBR_MAPS] = {
  {0x0008, 0x008b, 0x0008, 000000, SAMERICA_SND},
  {0x0008, 0x008b, 0x0068, 0x0009, EGYPT_SND},
  {0x0010, 0x008b, 0x0010, 0x0014, SCHWARZ_SND},
  {0x0010, 0x008b, 0x0010, 0x0026, MBASE_SND},
  {0x0074, 0x00c8, 0x0008, 0x0026, RICK1VICTORY_SND},
};

/*
 * global vars
 */
U16 game_period = 0;
U16 game_waitevt = FALSE;

U16 game_dir = 0;

hscore_t game_hscores[8] = {
  { 0, 8000, "TURSILION@" },
  { 0, 7000, "JAYNESIMES" },
  { 0, 6000, "DANGERSTU@" },
  { 0, 5000, "JEZEBEL@@@" },
  { 0, 4000, "ROB@N@BOB@" },
  { 0, 3000, "TELLY@@@@@" },
  { 0, 2000, "NOBBY@KEN@" },
  { 0, 1000, "TI994A@@@@" }
};

/*
 * local vars
 */
static U16 save_map_row;
static game_state_t game_state;
static U32 tm, tmx;

/*
 * prototypes
 */
static void game_cycle(void);
static void init(void);
static void restart(void);
static void game_paintEntities();
static void game_save(void);

// puts up the title page for scr_imain (and previously scr_xrick)
void draw_titlepage()
{
    unsigned int nOldBank = 0;

    nOldBank = nBank;

#ifdef F18A
    // we need to make sure bitmap is in full bitmap mode
    // (the tables can't move, so no need to change the global pointers)
    VDP_INT_DISABLE;
    set_fullbitmap();
    VDP_INT_ENABLE;

    // for ROM space reasons, the title picture is split up over 8(!!) banks
    // we'll still use img_paintPic since it keeps interrupts flowing

    // pattern
    SWITCH_IN_BANK24;
	img_paintPic(0, 0, 256, 48, (U8*)splashf18_patA, 0, 0);

    SWITCH_IN_BANK26;
	img_paintPic(0, 48, 256, 48, (U8*)splashf18_patB, 0, 0);

    SWITCH_IN_BANK27;
	img_paintPic(0, 96, 256, 48, (U8*)splashf18_patC, 0, 0);

    SWITCH_IN_BANK28;
	img_paintPic(0, 144, 256, 48, (U8*)splashf18_patD, 0, 0);

    // color
    SWITCH_IN_BANK17;
	img_paintPic(0, 0, 256, 48, 0, (U8*)splashf18_colA, 0);

    SWITCH_IN_BANK18;
	img_paintPic(0, 48, 256, 48, 0, (U8*)splashf18_colB, 0);

    SWITCH_IN_BANK19;
	img_paintPic(0, 96, 256, 48, 0, (U8*)splashf18_colC, 0);

    SWITCH_IN_BANK23;
	img_paintPic(0, 144, 256, 48, 0, (U8*)splashf18_colD, 0);

    // and also load a palette
    SWITCH_IN_BANK17;
    VDP_INT_DISABLE;
    loadpal_f18a(splashf18_pal, 0, 16);
    VDP_INT_ENABLE;

#else

    // pattern
    SWITCH_IN_BANK10;
	img_paintPic(0, 0, 256, 192, (U8*)pic_splash_pat, 0, 0);

    // color
    SWITCH_IN_BANK11;
	img_paintPic(0, 0, 256, 192, 0, (U8*)pic_splash_col, 0);

#endif

    // restore
    SWITCH_IN_BANK(nOldBank);
    // tell the tile banking page to reload on next request
    tiles_setBank(0xff);
}

void draw_hof_title()
{
    unsigned int nOldBank = nBank;

    // we need the main tiles loaded to see text!
    sysarg_half_bitmap = 0;
    tiles_setBank(0);

	/* hall of fame title */

#ifdef F18A
    // we need to make sure bitmap is in full bitmap mode
    // (the tables can't move, so no need to change the global pointers)
    VDP_INT_DISABLE;
    set_fullbitmap();
    VDP_INT_ENABLE;

    // pattern
    SWITCH_IN_BANK22;
	img_paintPic(0, 0, 256, 24, (U8*)pic_hafF18_pat, 0, 128);

    // color
    SWITCH_IN_BANK21;
	img_paintPic(0, 0, 256, 24, 0, (U8*)pic_hafF18_col, 128);

#else
    // pattern
    SWITCH_IN_BANK10;
	img_paintPic(0, 0, 256, 24, (U8*)pic_haf_pat, 0, 128);

    // color
    SWITCH_IN_BANK11;
	img_paintPic(0, 0, 256, 24, 0, (U8*)pic_haf_col, 128);
#endif

    // restore
    SWITCH_IN_BANK(nOldBank);
    // tell the bank engine it needs to reload on next request
    tiles_setBank(0xff);
}

/*
 * game_toggleCheat
 *
 * toggles one of the three cheat options
 * FIXME weird dependencies here! + _state exclusion is not complete
 */
void game_toggleCheat(U16 nbr) {
#ifdef ENABLE_CHEATS
    if (game_state != MAIN_INTRO && game_state != MAP_INTRO &&
        game_state != GAMEOVER && game_state != GETNAME &&
        game_state != EXIT)
    {
        switch (nbr)
        {
            case 1:
                env_trainer = ~env_trainer;
                env_lives = 6;
                env_bombs = 6;
                env_bullets = 6;
                break;

            case 2:
                env_invicible = ~env_invicible;
                break;

            case 3:
                env_highlight = ~env_highlight;
                break;
        }
    }
#endif
}

/* prototype */
static void game_loop(void);
static void game_exit(void);


/*
 * game_run
 *
 * main loop.
 */
void game_run(char* path) {
    (void)path;

    game_period = sysarg_args_period ? sysarg_args_period : GAME_PERIOD;
    tm = sys_gettime();
    game_state = MAIN_INTRO;

    /* main loop */
    while (game_state != EXIT)
    {
        game_loop();
        sysvid_update();
    }

    game_exit();
}

static void game_exit(void) {
}

static void game_loop(void) {
    unsigned int nOldBank;

    /* timer */
    if ((game_state != SCROLL_DOWN) && (game_state != SCROLL_UP)) {
        // this if doesn't help right now - scrolling is too slow, but it'll do
        // sys_gettime() and sys_sleep() use milliseconds
        tmx = tm; tm = sys_gettime(); tmx = tm - tmx;
        if (tmx < game_period) sys_sleep(game_period - (tmx & 0xffff));
    }

    /* sound: nothing to do here, everything is managed via callbacks */

    /* events */
    nOldBank = nBank;
#ifdef F18A
    // sysevt doesn't really care about F18A, but it was easier to move the whole page
    SWITCH_IN_BANK23
#else
    SWITCH_IN_BANK4;
#endif
    if (game_waitevt) {
        sysevt_wait();  /* wait for an event, stop doing anything */
    } else {
        sysevt_poll();  /* process events (non-blocking) */
    }
    SWITCH_IN_BANK(nOldBank);

    /*
     * game_cycle: depending on the game state
     * - process events
     * - run the game logic, AI, ...
     * - paints a new frame onto the frame buffer
     * - updates fb_updatedRects
     */
    game_cycle();
}

/*
 * game_cycle
 *
 * This function loops forever: use 'return' when a frame is ready.
 */
static void game_cycle(void) {
    unsigned int nOldBank = 0;
    U16 ret = 0;

    while (1) {
        switch (game_state) {

            case MAIN_INTRO:
                nOldBank = nBank;
#ifdef F18A
                SWITCH_IN_BANK16
#else
                SWITCH_IN_BANK4;
#endif
                ret = screen_introMain();
                SWITCH_IN_BANK(nOldBank);
                switch (ret)
                {
                    case SCREEN_RUNNING:
                        return;
                    case SCREEN_DONE:
                        game_state = INIT;
                        break;
                    case SCREEN_EXIT:
                        game_state = EXIT;
                        return;
                }
                break;

            case INIT:

                init();
                if (env_submap == map_maps[env_map].submap)
                {
                    game_state = MAP_INTRO;
                } else
                {
                    game_state = INIT_MAP; /* no intro if not first submap */
                }
                break;

            case MAP_INTRO:
                nOldBank = nBank;
#ifdef F18A
                SWITCH_IN_BANK30;
#else
                SWITCH_IN_BANK15;
#endif
                ret = screen_introMap();
                SWITCH_IN_BANK(nOldBank);
                switch (ret)
                {
                    case SCREEN_RUNNING:
                        return;
                    case SCREEN_DONE:
                        game_waitevt = FALSE;
                        game_state = INIT_MAP;
                        break;
                    case SCREEN_EXIT:
                        game_state = EXIT;
                        return;
                }
                break;

            case INIT_MAP:
#ifdef F18A
                set_halfbitmap();
#endif

                if (env_map >= 0x04) /* reached end of game */
                {
                    sysarg_args_map = 0; // FIXME game completed, start all over. fine, but... ack...
                    sysarg_args_submap = 0;
                    game_state = FADEOUT__GAMEOVER;
                } else
                {
                    map_init();
                    game_save();
                    fb_clear();                 /* clear buffer */
                    //ent_clprev();
                    maps_paint();               /* draw the map onto the buffer */
                    //ents_paintAll();
                    env_paintGame();            /* draw the status bar onto the buffer */
                    sysvid_setGamma(GAMMA_ON);
                    game_state = CTRL_ACTION;
                }
                break;

            case PAUSE_PRESSED1:

                screen_pause(TRUE);
                game_state = PAUSE_PRESSED1B;
                break;

            case PAUSE_PRESSED1B:

                if (control_status & CONTROL_PAUSE)
                    return;
                game_state = PAUSED;
                break;

            case PAUSED:

                if (control_status & CONTROL_PAUSE)
                {
                    game_state = PAUSE_PRESSED2;
                }
                if (control_status & CONTROL_EXIT)
                {
                    game_state = EXIT;
                }
                return;

            case PAUSE_PRESSED2:

                if (!(control_status & CONTROL_PAUSE))
                {
                    game_waitevt = FALSE;
                    screen_pause(FALSE);
#ifdef ENABLE_SOUND
                    // nothing - we'll let the last sound play out
                    //syssnd_pause(FALSE, FALSE);
#endif
                    game_state = CTRL_RICK;
                }
                return;

            case CTRL_ACTION:

                if (control_status & CONTROL_END) /* request to end the game */
                {
                    game_state = FADEOUT__GAMEOVER;
                } else
                    if (control_last == CONTROL_EXIT) /* request to exit the game */
                    {
                        game_state = EXIT;
                    } else
                    {
                        ent_action();      /* run entities */
                        e_them_rndseed++;  /* (0270) */
                        game_state = CTRL_PAUSE;
                    }
                    break;

            case CTRL_PAUSE:

                if (control_status & CONTROL_PAUSE)
                {
#ifdef ENABLE_SOUND
                    // let it play out
                    //syssnd_pause(TRUE, FALSE);
#endif
                    game_waitevt = TRUE;
                    game_state = PAUSE_PRESSED1;
                } else
                    if (control_active == FALSE)
                    {
#ifdef ENABLE_SOUND
                        // let it play out
                        //syssnd_pause(TRUE, FALSE);
#endif
                        game_waitevt = TRUE;
                        screen_pause(TRUE);
                        game_state = PAUSED;
                    } else
                    {
                        game_state = CTRL_RICK;
                    }
                    break;

            case CTRL_RICK:

                // FIXME if (e_rick_isDead)
                if E_RICK_STTST(E_RICK_STDEAD) /* rick is dead */
                {
                    if (env_trainer || --env_lives)
                    {
                        game_state = RESTART;
                    } else
                    {
                        game_state = FADEOUT__GAMEOVER;
                    }
                } else
                    if (e_rick_atExit) /* rick is exiting the submap, must chain to next submap */
                    {
                        //	e_rick_enterMap(); // akn
                        e_rick_atExit = FALSE;
                        game_state = NEXT_SUBMAP;
                    } else
                    {
                        game_state = PAINT;
                    }
                    break;

            case PAINT:

                game_paintEntities();
                game_state = CTRL_SCROLL;
                return;

            case CTRL_SCROLL:
                if (!E_RICK_STTST(E_RICK_STZOMBIE))
                {
                    if (ent_ents[1].y >= 0xcc)
                    {
                        game_state = SCROLL_UP;
                    } else
                        if (ent_ents[1].y <= 0x60)
                        {
                            game_state = SCROLL_DOWN;
                        } else
                        {
                            game_state = CTRL_ACTION;
                        }
                } else
                {
                    game_state = CTRL_ACTION;
                }
                break;

            case NEXT_SUBMAP:

                if (map_chain())
                {
                    /* next submap, now initialize */
                    game_state = INIT_SUBMAP;
                } else
                {
                    /* end of submap, chain to next map */

                    env_bullets = 0x06;
                    env_bombs = 0x06;
                    env_map++;
                    tiles_setBank(0xff);    // reload the tile bank, cause of the text font overwrite

                    if (env_map == 0x04)
                    {
                        /* reached end of game */
                        /* FIXME @292?*/
                    }

                    game_state = NEXT_MAP;
                }
                break;

            case NEXT_MAP:

                ent_ents[1].x = map_maps[env_map].x;
                ent_ents[1].y = map_maps[env_map].y;
                map_frow = (U16)map_maps[env_map].row;
                env_submap = map_maps[env_map].submap;
                sysvid_setGamma(GAMMA_OFF);
                game_state = MAP_INTRO;
                break;

            case INIT_SUBMAP:

                map_init();                     /* initialize the map */
                game_save();                        /* save data in case of a restart */
                fb_clear();
                ent_clprev();                   /* cleanup entities */
                maps_paint();                     /* draw the map onto the buffer */
                ents_paintAll();
                env_paintGame();              /* draw the status bar onto the buffer */
                game_state = CTRL_ACTION;
                return;

            case SCROLL_UP:
                ret = scroll_up();
                switch (ret)
                {
                    case SCROLL_RUNNING:
                        return;
                    case SCROLL_DONE:
                        game_state = CTRL_ACTION;
                        break;
                }
                break;

            case SCROLL_DOWN:
                ret = scroll_down();
                switch (ret)
                {
                    case SCROLL_RUNNING:
                        return;
                    case SCROLL_DONE:
                        game_state = CTRL_ACTION;
                        break;
                }
                break;

            case RESTART:

                restart();
                game_state = CTRL_ACTION;
                return;

            case FADEOUT__GAMEOVER:

                sysvid_setGamma(GAMMA_OFF);
                game_state = GAMEOVER;
                return;

            case GAMEOVER:
                nOldBank = nBank;
                SWITCH_IN_BANK10;
                ret = screen_gameover();
                SWITCH_IN_BANK(nOldBank);

                switch (ret)
                {
                    case SCREEN_RUNNING:
                        return;
                    case SCREEN_DONE:
                        game_state = GETNAME;
                        break;
                    case SCREEN_EXIT:
                        game_state = EXIT;
                        break;
                }
                break;

            case GETNAME:
                nOldBank = nBank;
                SWITCH_IN_BANK3;
                ret = screen_getname();
                SWITCH_IN_BANK(nOldBank);

                switch (ret)
                {
                    case SCREEN_RUNNING:
                        return;
                    case SCREEN_DONE:
                        SWITCH_IN_BANK5;
                        savegrom();
                        SWITCH_IN_BANK(nOldBank);
                        game_state = MAIN_INTRO;
                        return;
                    case SCREEN_EXIT:
                        game_state = EXIT;
                        break;
                }
                break;

            case EXIT:
                return;
        }
    }
}

/*
 * init
 *
 * FIXME some dirty hacks here
 */
static void init(void) {
    U16 i;
    unsigned int nOldBank;

    E_RICK_STRST(0xffff);

    env_lives = 6;
    env_bombs = 6;
    env_bullets = 6;
    env_score_lo = 0;
    env_score_hi = 0;

    env_map = sysarg_args_map;
    nOldBank = nBank;

    SWITCH_IN_BANK12;     // map_connect

    if (sysarg_args_submap == 0) {
        env_submap = map_maps[env_map].submap;
        map_frow = (U16)map_maps[env_map].row;
    } else {
      /* dirty hack to determine frow by chaining submaps...*/
        env_submap = sysarg_args_submap;
        i = 0;
        while (i < 4 && map_maps[i++].submap <= env_submap);
        env_map = i - 1;
        i = 0;
        while (i < MAP_NBR_CONNECT &&
           (map_connect[i].submap != env_submap ||
               map_connect[i].dir != RIGHT))
            i++;
        map_frow = map_connect[i].rowin - 0x10; // WHY 0x10??
    }

    ent_ents[1].x = map_maps[env_map].x;
    ent_ents[1].y = map_maps[env_map].y;
    ent_ents[1].w = 0x18;
    ent_ents[1].h = 0x15;
    ent_ents[1].n = 0x01;
    ent_ents[1].sprite = 0x01;
    ent_ents[1].spriteIndex = 0xff;
    ent_ents[1].lastSpriteDrawn = 0xff;
    ent_ents[ENT_ENTSNUM].n = 0xFF;

    SWITCH_IN_BANK(nOldBank);

    map_resetMarks();
}

/*
 * game_paintEntities
 *
 * paints the entities.
 */
static void game_paintEntities() {
    ents_paintAll();  /* draw all entities onto the buffer */
    env_paintGame();  /* draw the status bar onto the buffer*/
}

/*
 * restart
 *
 * restarts the game after rick died. just come back to the beginning
 * of the current submap, restore positions and flags and...
 */
static void restart(void) {
    E_RICK_STRST(E_RICK_STDEAD | E_RICK_STZOMBIE); // should be part of e_rick

    env_bullets = 6;
    env_bombs = 6;

    ent_ents[1].n = 1; // FIXMEwhy??

    e_rick_restore(); // FIXME that should restore the state ?!!?
    map_frow = save_map_row;

    map_init(); // see INIT_MAP check that everything is OK here
    game_save();
    ent_clprev();
    maps_paint();
    env_paintGame();
}

/*
 * game_save
 *
 * save game state so it can be restored when rick dies, by <restart>.
 * it is NOT a "save game" option!
 */
static void game_save(void) {
    e_rick_save();
    save_map_row = map_frow;
}

/* eof */
