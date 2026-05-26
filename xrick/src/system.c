/*
 * xrick/src/system.c
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

#ifndef GFXTI
#include <SDL.h>

#include <stdarg.h>   /* args for sys_panic */
#include <fcntl.h>    /* fcntl in sys_panic */
#include <stdio.h>    /* printf */
#include <stdlib.h>

/*
 * Panic
 */
void
sys_panic(char *err, ...)
{
	va_list argptr;
	char s[1024];

	/* FIXME what is this? */
	/* change stdin to non blocking */
	/*fcntl(0, F_SETFL, fcntl (0, F_GETFL, 0) & ~FNDELAY);*/
	/* NOTE HPUX: use ... is it OK on Linux ? */
	/* fcntl(0, F_SETFL, fcntl (0, F_GETFL, 0) & ~O_NDELAY); */

	/* prepare message */
	va_start(argptr, err);
	vsprintf(s, err, argptr);
	va_end(argptr);

	/* print message and die */
	printf("%s\npanic!\n", s);
	exit(1);
}


/*
 * Print a message
 */
void
sys_printf(char *msg, ...)
{
#ifdef ENABLE_LOG
	va_list argptr;
	char s[1024];

	/* FIXME what is this? */
	/* change stdin to non blocking */
	/* fcntl(0, F_SETFL, fcntl (0, F_GETFL, 0) & ~FNDELAY); */
	/* NOTE HPUX: use ... is it OK on Linux ? */
	/* fcntl(0, F_SETFL, fcntl (0, F_GETFL, 0) & ~O_NDELAY); */

	/* prepare message */
	va_start(argptr, msg);
	vsprintf(s, msg, argptr);
	va_end(argptr);
	printf(s);
#endif
}

/*
 * Return number of milliseconds elapsed since first call
 */
U32
sys_gettime(void)
{
	static U32 ticks_base = 0;
	U32 ticks;

	ticks = SDL_GetTicks();

	if (!ticks_base)
		ticks_base = ticks;

	return ticks - ticks_base;
}

/*
 * Sleep a number of milliseconds
 */
void
sys_sleep(int s)
{
	SDL_Delay(s);
}

#endif

#ifdef GFXTI

#include <vdp.h>
#include <system.h>
extern volatile U16 vdpCount;

/*
 * Panic
 */
void
sys_panic(char *err, ...)
{
    // We don't have vararg support, so we're kind of out of luck, but we can display panic and lockup - data should be
    // findable in a debugger...
    VDP_INT_DISABLE;
    charset();
    vdpmemcpy(gImage, "PANIC!", 6);
    halt();
}

/*
 * Print a message
 */
void
sys_printf(char *msg, ...)
{
#ifdef ENABLE_LOG
    // TODO: out of luck here too... could use Classic99 debug but need varargs...
#endif
}

/*
 * Return number of milliseconds elapsed since first call - note FRAME accurate
 */
U16
sys_gettime(void)
{
    // vdpCount is initialized to 0, so we don't need to subtract an
    // offset. But we do need to divide by 10!
    // Here's where we find out if the compiler has a decent 32-bit divide...
	return vdpCount / 10;
}

/*
 * Sleep a number of milliseconds
 */
void
sys_sleep(int s)
{
    U16 now = vdpCount;
    U16 target = now + (s*10);

    // wraparound case
    if (target < now) {
        // wait till the wrap happens
        // the spin is fine, the CPU isn't doing anything anyway
        while (vdpCount > target) { }
    }

    // wait for the target
    while (vdpCount < target) { }
}
#endif

/* eof */
