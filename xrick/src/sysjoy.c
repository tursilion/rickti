/*
 * xrick/src/sysjoy.c
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
#endif

#ifdef ENABLE_JOYSTICK

#include "ricksystem.h"
#include "debug.h"

#ifndef GFXTI
static SDL_Joystick *j = NULL;

void
sysjoy_init(void)
{
  U16 i, jcount;

  if (SDL_InitSubSystem(SDL_INIT_JOYSTICK) < 0) {
    IFDEBUG_JOYSTICK(
      sys_printf("xrick/joystick: can not initialize joystick subsystem\n");
      );
    return;
  }

  jcount = SDL_NumJoysticks();
  if (!jcount) {  /* no joystick on this system */
    IFDEBUG_JOYSTICK(sys_printf("xrick/joystick: no joystick available\n"););
    return;
  }

  /* use the first joystick that we can open */
  for (i = 0; i < jcount; i++) {
    j = SDL_JoystickOpen(i);
    if (j)
      break;
  }

  /* enable events */
  SDL_JoystickEventState(SDL_ENABLE);
}

void
sysjoy_shutdown(void)
{
  if (j)
    SDL_JoystickClose(j);
}
#endif

#ifdef GFXTI
void
sysjoy_init(void)
{
    // TODO: maybe decide which stick to use, or whether to use a NES style?
}

void
sysjoy_shutdown(void)
{
}
#endif

#endif /* ENABLE_JOYSTICK */

/* eof */

