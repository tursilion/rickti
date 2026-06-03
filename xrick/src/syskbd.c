/*
 * xrick/src/syskbd.c
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

/*
 * Using the SDL_SCANCODE_xxx keysyms, which map to a QWERTY keyboard.
 * We get them via SDL_KEYDOWN.
 * We do *not* use SDL_TEXTINPUT nor SDLK_ to get true key mappings, so
 * for instance left on an AZERTY keyboard will be 'w' instead of 'z'.
 */

U16 syskbd_up = 'E';
U16 syskbd_down = 'X';
U16 syskbd_left = 'S';
U16 syskbd_right = 'D';
U16 syskbd_pause = 'P';
U16 syskbd_end = '.';
U16 syskbd_xtra = ',';
U16 syskbd_fire = ' ';

/* eof */


