/*
 * xrick/src/sysarg.c
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
 * 20021010 added test to prevent buffer overrun in -keys parsing.
 */

#include "config.h"

#include <kscan.h>
#include <system.h>

#include "syskbd.h"
#include "syssnd.h"

#include "game.h"
#include "maps.h"

/* handle Microsoft Visual C (must come after system.h!) */
#ifdef __MSVC__
#define strcasecmp _stricmp
#endif

// Not too useful for TI port, but that's okay
int sysarg_args_period = 0;
int sysarg_args_map = 0;
int sysarg_args_submap = 0;
int sysarg_args_fullscreen = 0;
int sysarg_args_zoom = 0;
int sysarg_args_nosound = 0;
int sysarg_args_vol = 0;
char *sysarg_args_data = NULL;

/*
 * Fail
 */
void
sysarg_fail(char *msg)
{
#ifdef CLASSIC99
    #ifdef ENABLE_SOUND
	    sys_printf("xrick [version #%s]: %s\nusage: xrick [<options>]\n<option> =\n  -h, -help : Display this information.\n-fullscreen : Run in fullscreen mode.\n    The default is to run in a window.\n  -speed <speed> : Run at speed <speed>. Speed must be an integer between 1\n    (fast) and 100 (slow). The default is %d\n  -zoom <zoom> : Display with zoom factor <zoom>. <zoom> must be an integer\n   between 1 (320x200) and x (x times bigger). The default is 2.\n  -map <map> : Start at map number <map>. <map> must be an integer between\n    1 and %d. The default is to start at map number 1\n  -submap <submap> : Start at submap <submap>. <submap> must be an integer\n    between 1 and %d. The default is to start at submap number 1 or, if a map\n    was specified, at the first submap of that map.\n  -keys <left>-<right>-<up>-<down>-<fire> : Override the default key\n    bindings (cf. KeyCodes)\n  -nosound : Disable sounds. The default is to play with sounds enabled.\n  -vol <vol> : Play sounds at volume <vol>. <vol> must be an integer\n    between 0 (silence) and %d (max). The default is to play sounds\n    at maximal volume (%d).\n", VERSION, msg, GAME_PERIOD, MAP_NBR_MAPS-1, MAP_NBR_SUBMAPS, SYSSND_MAXVOL, SYSSND_MAXVOL);
    #else
	    sys_printf("xrick [version #%s]: %s\nusage: xrick [<options>]\n<option> =\n  -h, -help : Display this information.\n-fullscreen : Run in fullscreen mode.\n    The default is to run in a window.\n  -speed <speed> : Run at speed <speed>. Speed must be an integer between 1\n    (fast) and 100 (slow). The default is %d\n  -zoom <zoom> : Display with zoom factor <zoom>. <zoom> must be an integer\n   between 1 (320x200) and x (x times bigger). The default is 2.\n  -map <map> : Start at map number <map>. <map> must be an integer between\n    1 and %d. The default is to start at map number 1\n  -submap <submap> : Start at submap <submap>. <submap> must be an integer\n    between 1 and %d. The default is to start at submap number 1 or, if a map\n    was specified, at the first submap of that map.\n  -keys <left>-<right>-<up>-<down>-<fire> : Override the default key\n    bindings (cf. KeyCodes)\n", VERSION, msg, GAME_PERIOD, MAP_NBR_MAPS-1, MAP_NBR_SUBMAPS);
    #endif
#endif
    exit();
}


/*
 * Read and process arguments
 */
void
sysarg_init(int argc, char **argv)
{
    (void)argc;
    (void)argv;


}

/* eof */





