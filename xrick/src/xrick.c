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

#include "config.h"
#include "ricksystem.h"
#include "sysarg.h"
#include "sysvid.h"
#include "sysjoy.h"
#include "game.h"
#include "fb.h"
#include "sprites.h"

#ifndef GFXTI
    #include <SDL.h>
    #include <signal.h>
    #ifdef __WIN32__
    #include <windows.h>
    #endif
#endif

#ifdef GFXTI
#include <vdp.h>
extern sprite_data_t sprite_table[ENT_ENTSNUM+1];

// timer updated by VDP interrupt - making it 32 bit so it can represent milliseconds better
// without wrapping very often. Note we are doing milliseconds*10 (so we can count by 16.6ms)
// TODO: this is wrong for PAL, obviously, which counts at 20ms
volatile U16 vdpCount;

// current bank, used so we can switch and switch back
unsigned int nBank;

// reset time at startup
void sys_resettime() {
    vdpCount = 0;
}

// gcc doesn't support naked functions, so we need to write the interrupt handler in assembly...
extern void sys_vdpint();
#endif

/*
 * Sets a console, if possible
 */
static void setConsole()
{
	// NOTE: does not handle parent process console being redirected
	// eg "./xrick > stdout.txt" still writes to the actual console

#ifdef __WIN32__
	// try to attach to parent process console
	if (AttachConsole(-1))
	{
		// reopen (SDL2 closed everything)
		freopen("CONIN$", "r", stdin);
		freopen("CONOUT$", "w", stdout);
		freopen("CONOUT$", "w", stderr);
	}
#endif
}



 /*
  * Initialize system
  */
void
sys_init(int argc, char** argv)
{
#ifndef GFXTI
	setConsole();
	sys_printf("xrick\n");

	// FIXME not writing to stdxxx.txt files anymore?
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS) < 0)
		sys_panic("xrick/video: could not init SDL\n");

	// FIXME logging
	// Solved (embarrassingly simple) :
	// SDL 1.2 did that outputting into files, SDL 2 does not.However most information(toturials and such) on the net is about SDL 1.2 since SDL 2 is new.
	// Furthermore compiling with -mwindows sends all stdout and stderr to null.
	// Compiling without solves my problem.

	//SDL_Log("SDL!");

	sysvid_init(FB_WIDTH, FB_HEIGHT);
#ifdef ENABLE_JOYSTICK
	sysjoy_init();
#endif
#ifdef ENABLE_SOUND
	if (sysarg_args_nosound == 0)
		syssnd_init();
#endif

	atexit(sys_shutdown);
	signal(SIGINT, exit);
	signal(SIGTERM, exit);
#endif

#ifdef GFXTI
    // make sure interrupts are off during init
    VDP_INT_DISABLE;

    // get banking into a known state
    SWITCH_IN_BANK0;

    // TODO: F18A can use bitmap with a single color/pattern table (half bitmap) and still
    // have access to all sprites. It will use palettes. Also set up for 8 color sprites.
    VDP_SET_REGISTER(VDP_REG_COL, COLOR_BLACK);
    set_bitmap(VDP_SPR_16x16);
    // get a blank screen up by initializing all three char 0 to blank and then writing all zeros to the SIT
    vdpmemset(gPattern, 0, 8);
    vdpmemset(gPattern+0x800, 0, 8);
    vdpmemset(gPattern+0x1000, 0, 8);
    vdpmemset(gImage, 0, 768);

    sys_resettime();
    VDP_INT_CTRL = VDP_INT_CTRL_DISABLE_ALL;
    VDP_INT_HOOK_SET(sys_vdpint);
    VDP_CLEAR_VBLANK;

    sysarg_init(0, 0);

#ifdef ENABLE_JOYSTICK
	sysjoy_init();
#endif

#ifdef ENABLE_SOUND
	if (sysarg_args_nosound == 0)
		syssnd_init();
#endif

    // Note: we run with interrupts enabled, but we expect no VDP activity during blank
    VDP_INT_ENABLE;
#endif
}



/*
 * Shutdown system
 */
void
sys_shutdown(void)
{
#ifdef ENABLE_SOUND
	syssnd_shutdown();
#endif

#ifdef ENABLE_JOYSTICK
	sysjoy_shutdown();
#endif

	sysvid_shutdown();

#ifndef GFXTI
	SDL_Quit();
#endif
}


#ifndef GFXTI
/*
 * main
 */
int
main(int argc, char *argv[])
{
	sys_init(argc, argv);

	char* path;
	if (sysarg_args_data)
		path = sysarg_args_data;
	else
		path = "data.zip";

	game_run(path);

	sys_shutdown();
	return 0;
}
#endif

#ifdef GFXTI
/*
 * main
 */
int
main()
{
	sys_init(0, 0);

	game_run(0);

	sys_shutdown();
	return 0;
}
#endif

/* eof */
