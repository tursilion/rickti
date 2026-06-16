/*
 * xrick/include/sysvid.h
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

#ifndef _SYSVID_H
#define _SYSVID_H

#include "img.h"
// was originally 255 in original game (to allow fade in from 0-255)
#define GAMMA_ON 1
#define GAMMA_OFF 0

extern void sysvid_update(void);
extern void sysvid_setGamma(U16 g);
extern void bitmapcharcopy(U16 adr, const U8* buf, U16 size);

#ifdef F18A
void set_halfbitmap();
void set_fullbitmap();
#endif

#endif /* _SYSVID_H */

/* eof */


