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

#include "ricksystem.h"

/* version */
#define VERSION "050500"

/* logging (write to console) */
#ifdef CLASSIC99
#define ENABLE_LOG
#else
    #undef ENABLE_LOG
    #ifdef EMSCRIPTEN
        #undef ENABLE_LOG
    #endif
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

/* bank switching */
// bank switching - nOldBank is used to let a function restore the original bank
// our fixed bank is copied out of banks 0-1-2 into 32k memory expansion
extern unsigned int nBank;
#ifndef CLASSIC99
#define SWITCH_IN_BANK(nOldBank) nBank=nOldBank; (*(volatile unsigned int*)nOldBank)=nBank; 
#define SWITCH_IN_BANK0    nBank=(unsigned int)0x6000; (*(volatile unsigned int*)0x6000)=nBank;  
#define SWITCH_IN_BANK1    nBank=(unsigned int)0x6002; (*(volatile unsigned int*)0x6002)=nBank;  
#define SWITCH_IN_BANK2    nBank=(unsigned int)0x6004; (*(volatile unsigned int*)0x6004)=nBank; 
#define SWITCH_IN_BANK3    nBank=(unsigned int)0x6006; (*(volatile unsigned int*)0x6006)=nBank; 
#define SWITCH_IN_BANK4    nBank=(unsigned int)0x6008; (*(volatile unsigned int*)0x6008)=nBank; 	
#define SWITCH_IN_BANK5    nBank=(unsigned int)0x600a; (*(volatile unsigned int*)0x600a)=nBank; 
#define SWITCH_IN_BANK6    nBank=(unsigned int)0x600c; (*(volatile unsigned int*)0x600c)=nBank; 
#define SWITCH_IN_BANK7    nBank=(unsigned int)0x600e; (*(volatile unsigned int*)0x600e)=nBank; 
#define SWITCH_IN_BANK8    nBank=(unsigned int)0x6010; (*(volatile unsigned int*)0x6010)=nBank; 
#define SWITCH_IN_BANK9    nBank=(unsigned int)0x6012; (*(volatile unsigned int*)0x6012)=nBank; 
#define SWITCH_IN_BANK10   nBank=(unsigned int)0x6014; (*(volatile unsigned int*)0x6014)=nBank; 
#define SWITCH_IN_BANK11   nBank=(unsigned int)0x6016; (*(volatile unsigned int*)0x6016)=nBank; 
#define SWITCH_IN_BANK12   nBank=(unsigned int)0x6018; (*(volatile unsigned int*)0x6018)=nBank; 
#define SWITCH_IN_BANK13   nBank=(unsigned int)0x601a; (*(volatile unsigned int*)0x601a)=nBank; 
#define SWITCH_IN_BANK14   nBank=(unsigned int)0x601c; (*(volatile unsigned int*)0x601c)=nBank; 
#define SWITCH_IN_BANK15   nBank=(unsigned int)0x601e; (*(volatile unsigned int*)0x601e)=nBank; 
#else
#define SWITCH_IN_BANK(nOldBank) nBank=nOldBank;
#define SWITCH_IN_BANK0    nBank=(unsigned int)0x6000;  
#define SWITCH_IN_BANK1    nBank=(unsigned int)0x6002;  
#define SWITCH_IN_BANK2    nBank=(unsigned int)0x6004; 
#define SWITCH_IN_BANK3    nBank=(unsigned int)0x6006; 
#define SWITCH_IN_BANK4    nBank=(unsigned int)0x6008; 	
#define SWITCH_IN_BANK5    nBank=(unsigned int)0x600a; 
#define SWITCH_IN_BANK6    nBank=(unsigned int)0x600c; 
#define SWITCH_IN_BANK7    nBank=(unsigned int)0x600e; 
#define SWITCH_IN_BANK8    nBank=(unsigned int)0x6010; 
#define SWITCH_IN_BANK9    nBank=(unsigned int)0x6012; 
#define SWITCH_IN_BANK10   nBank=(unsigned int)0x6014; 
#define SWITCH_IN_BANK11   nBank=(unsigned int)0x6016; 
#define SWITCH_IN_BANK12   nBank=(unsigned int)0x6018; 
#define SWITCH_IN_BANK13   nBank=(unsigned int)0x601a; 
#define SWITCH_IN_BANK14   nBank=(unsigned int)0x601c; 
#define SWITCH_IN_BANK15   nBank=(unsigned int)0x601e; 
#endif


#endif

/* eof */


