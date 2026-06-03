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

#ifdef CLASSIC99
#include <stdarg.h>   /* args for sys_panic */
#include <stdio.h>    /* printf */
#include <stdlib.h>
#endif

#include <vdp.h>
#include <system.h>
#ifndef CLASSIC99
// vdpCountL is a 15.4 bit count. vdpCountH is 16.0
extern volatile U16 vdpCountL;
extern volatile U16 vdpCountH;
#else
// vdpCount is still a divide by 10 here and there's no high
// I might have to change that next time I debug...
extern U16 getVdpCount();
extern void setVdpCount(U16 val);
#endif

/*
 * Panic
 */
void
sys_panic(char *err, ...)
{
#ifdef CLASSIC99
	va_list argptr = 0;
	char s[1024];

	/* prepare message */
	va_start(argptr, err);
	vsprintf(s, err, argptr);
	va_end(argptr);

	/* print message and die */
	fprintf(stdout, "%s\npanic!\n", s);
#endif

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
#ifdef CLASSIC99
#ifdef ENABLE_LOG
	va_list argptr = 0;
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
	fprintf(stdout, "%s", s);
#endif
#endif
}

/*
 * Return number of milliseconds elapsed since first call - note FRAME accurate
 */
U32 sys_gettime(void)
{
    // vdpCount is initialized to 0, so we don't need to subtract an
    // offset. But we do need to divide! There are 4 bits of fraction to remove
#ifndef CLASSIC99
    return (vdpCountL>>4) | ((U32)vdpCountH<<12);
#else
    return getVdpCount()/10;
#endif
}

/*
 * Sleep a number of milliseconds
 */
void
sys_sleep(U16 s)
{
#ifndef CLASSIC99
    U32 now = vdpCountL | ((U32)vdpCountH<<16);
    U32 target = now + (s*16);
#else
    U32 now = getVdpCount();
    U32 target = now + (s*10);
#endif

    // wraparound case - just run this one frame fast. Shouldn't be noticable.
    // should be exceptionally rare - wrap at 24.85 days
    if (target < now) {
        return;
    }

    // wait for the target
#ifndef CLASSIC99
    while ((vdpCountL | ((U32)vdpCountH<<16)) < target) { }
#else
    U16 x;
    do {
        x = getVdpCount();
    } while ((x < target) && (x > now));
#endif

}

/* eof */
