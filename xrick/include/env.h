/*
 * xrick/include/env.h
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

#ifndef _ENV_H_
#define _ENV_H_

#include "ricksystem.h"

/* cheat modes */
extern U16 env_trainer;
extern U16 env_invicible;
extern U16 env_highlight;

/*
 * depth mode, TRUE if managed (sprites can be hidden by foreground tiles)
 * and FALSE if not (sprites are always before anything else).
 */
extern U16 env_depth;

/* number of lives, bombs and bullets currently available */
extern U16 env_lives;
extern U16 env_bombs;
extern U16 env_bullets;

/* game score */
extern U16 env_score_lo;
extern U16 env_score_hi;
extern void addscore(U16 val);

/* current map and submap */
extern U16 env_map;
extern U16 env_submap;
extern U16 env_changeSubmap; /* change submap request (TRUE, FALSE) */

/* offset of digits for status line */
extern U8 env_digits;

extern void env_paintGame(void);

#endif

/* eof */

