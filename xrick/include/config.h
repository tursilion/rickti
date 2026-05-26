/*
 * xrick/include/config.h
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

#ifndef _CONFIG_H
#define _CONFIG_H

/* graphics (choose one) */
#undef GFXST
#undef GFXPC
#define GFXTI

#include "ricksystem.h"

/* version */
#define VERSION "050500"

/* logging (write to console) */
#undef ENABLE_LOG
#ifdef EMSCRIPTEN
#undef ENABLE_LOG
#endif

/* joystick support */
#define ENABLE_JOYSTICK

/* sound support TODO */
#undef ENABLE_SOUND

/* cheats support */
#define ENABLE_CHEATS

/* auto-defocus support */
/* does seem to cause all sorts of problems on BeOS, Windows... */
#undef ENABLE_FOCUS

/* development tools */
#undef ENABLE_DEVTOOLS
#define DEBUG /* see include/debug.h */

/* zlib */
#define NOZLIB
#ifndef NOZLIB
#define WITH_ZLIB
#endif

/* files */
#undef USE_DATA_FILES

#endif

/* eof */


