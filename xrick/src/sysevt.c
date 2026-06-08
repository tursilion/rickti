/*
 * xrick/src/sysevt.c
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

/*
 * 20021010 SDL_SCANCODE_n replaced by SDL_SCANCODE_Fn because some non-US keyboards
 *          requires that SHIFT be pressed to input numbers.
 */

#include "config.h"

#include <kscan.h>
#include <system.h>

#include "ricksystem.h"
#include "syskbd.h"
#include "sysvid.h"
#include "game.h"
#include "debug.h"

#include "control.h"
#include "draw.h"

#define SETBIT(x,b) x |= (b)
#define CLRBIT(x,b) x &= ~(b)

/*
 * Scan inputs, return non-zero if any were detected
 */
static int processEvent()
{
    int ret = 0;
    U16 key;

    // Keyboard first
    kscanfast(0);
    key = KSCAN_KEY;

    // assume nothing set
    control_status = 0;

    // check important keys - only setting control_last for exit, ignoring keybd_end and xtra
    if (key != 0xff) {
        if (key == syskbd_up) SETBIT(control_status, CONTROL_UP);
		else if (key == syskbd_down) SETBIT(control_status, CONTROL_DOWN);
		else if (key == syskbd_left) SETBIT(control_status, CONTROL_LEFT);
		else if (key == syskbd_right) SETBIT(control_status, CONTROL_RIGHT);
		else if (key == syskbd_pause) SETBIT(control_status, CONTROL_PAUSE);
		else if (key == syskbd_fire) SETBIT(control_status, CONTROL_FIRE);
#ifdef ENABLE_SOUND
        else if (key == '4') { syssnd_toggleMute(); ret = 1; }
#endif
		else if (key == '7') { game_toggleCheat(1); ret = 1; }
		else if (key == '8') { game_toggleCheat(2); ret = 1; }
		else if (key == '9') { game_toggleCheat(3); ret = 1; }
    }

    if (check_reset()) {
        // TODO: remember F18A needs a reset_f18a() when we act on this
        /* player tries to close the window -- this is the same as pressing ESC */
        SETBIT(control_status, CONTROL_EXIT);
        control_last = CONTROL_EXIT;
    }

#ifdef ENABLE_JOYSTICK
    joystfast(1);

    key = KSCAN_JOYX;
    if (key == JOY_LEFT) SETBIT(control_status, CONTROL_LEFT);
    if (key == JOY_RIGHT) SETBIT(control_status, CONTROL_RIGHT);

    key = KSCAN_JOYY;
    if (key == JOY_UP) SETBIT(control_status, CONTROL_UP);
    if (key == JOY_DOWN) SETBIT(control_status, CONTROL_DOWN);

    kscanfast(1);
    if (KSCAN_KEY == JOY_FIRE) SETBIT(control_status, CONTROL_FIRE);    // just one check, no need to cache
#endif

    if (control_status != 0) ret = 1;

    return 1;
}

/*
 * Process events, if any, then return
 */
void
sysevt_poll(void)
{
    processEvent();
}

/*
 * Wait for an event, then process it and return
 */
void
sysevt_wait(void)
{
    while (processEvent() == 0) { }
}

/* eof */



