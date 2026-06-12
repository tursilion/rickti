/*
 * xrick/src/scr_pause.c
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

#include <stddef.h> /* NULL */

#include "ricksystem.h"
#include "config.h"
#include "env.h"

#include "game.h"
#include "screens.h"

#include "draw.h"
#include "control.h"
#include "ents.h"

#include "tiles.h"
#include "maps.h"

#include <vdp.h>


/*
 * Display the pause indicator
 */
void
screen_pause(U16 pause)
{
    VDP_INT_DISABLE;
    hchar(0,0,0,32);
    VDP_INT_ENABLE;

    if (pause == TRUE)
	{
        // we'll do this the same way as the status line, overwriting the digits
        // "PAUSED" takes just 6 characters
        loadStringTiles("PAUSED");
        VDP_INT_DISABLE;
        VDP_SET_ADDRESS_WRITE(gImage+3);
        for (U8 c=env_digits; c<env_digits+6; ++c) {
            VDPWD(c);
        }
        VDP_INT_ENABLE;
	}
	else
	{
        loadDigitTiles();
		env_paintGame();
	}
}


/* eof */

