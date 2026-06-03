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

#ifdef ENABLE_JOYSTICK

#include "ricksystem.h"
#include "debug.h"

void
sysjoy_init(void)
{
    // TODO: maybe decide which stick to use, or whether to use a NES style?
}

void
sysjoy_shutdown(void)
{
}

#endif /* ENABLE_JOYSTICK */

/* eof */

