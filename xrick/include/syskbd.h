/*
 * xrick/include/syskbd.h
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

#ifndef _SYSKBD_H
#define _SYSKBD_H

#include "ricksystem.h"

#ifdef ENABLE_KEYBOARD
extern U16 syskbd_up;
extern U16 syskbd_down;
extern U16 syskbd_left;
extern U16 syskbd_right;
extern U16 syskbd_fire;
#endif

extern U16 syskbd_pause;

#endif /* _SYSKBD_H */

/* eof */
