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
#endif

/* joystick support */
#define ENABLE_JOYSTICK

#ifdef CLASSIC99
// I don't have this working here...
#undef ENABLE_SOUND
#else
/* sound support */
#define ENABLE_SOUND
#endif

/* cheats support */
#define ENABLE_CHEATS

/* debug support (only supported in Classic99 build */
#undef DEBUG /* see include/debug.h */

/* bank switching */
// bank switching - nOldBank is used to let a function restore the original bank
// our fixed bank is copied out of banks 0-1-2 into 32k memory expansion
extern unsigned int nBank;
#ifndef CLASSIC99

// In this application, we leave interrupts enabled. It is crucial that the bank switch code
// happens in this order. If the hardware changes before nBank, an interrupt between the two
// steps could change back to the old bank before we set nBank (I observed the compiler doing
// this.) The '__asm__ __volatile__("":::"memory");' introduces a compiler memory barrier
// to prevent it from re-ordering the instruction without always making nBank volatile
// (although in fairness, it seems to generate the same code).

// WARNING: don't pass nBank to this, the compiler will not do the right thing! And you probably didn't mean it anyway!
#define SWITCH_IN_BANK(nOldBank) nBank = (nOldBank); __asm__ __volatile__("":::"memory"); (*(volatile unsigned int*)(nOldBank)) = 0;
#define SWITCH_IN_BANK0    nBank=(unsigned int)0x6000; __asm__ __volatile__("":::"memory"); (*(volatile unsigned int*)0x6000)=0;  
#define SWITCH_IN_BANK1    nBank=(unsigned int)0x6002; __asm__ __volatile__("":::"memory"); (*(volatile unsigned int*)0x6002)=0;  
#define SWITCH_IN_BANK2    nBank=(unsigned int)0x6004; __asm__ __volatile__("":::"memory"); (*(volatile unsigned int*)0x6004)=0; 
#define SWITCH_IN_BANK3    nBank=(unsigned int)0x6006; __asm__ __volatile__("":::"memory"); (*(volatile unsigned int*)0x6006)=0; 
#define SWITCH_IN_BANK4    nBank=(unsigned int)0x6008; __asm__ __volatile__("":::"memory"); (*(volatile unsigned int*)0x6008)=0; 	
#define SWITCH_IN_BANK5    nBank=(unsigned int)0x600a; __asm__ __volatile__("":::"memory"); (*(volatile unsigned int*)0x600a)=0; 
#define SWITCH_IN_BANK6    nBank=(unsigned int)0x600c; __asm__ __volatile__("":::"memory"); (*(volatile unsigned int*)0x600c)=0; 
#define SWITCH_IN_BANK7    nBank=(unsigned int)0x600e; __asm__ __volatile__("":::"memory"); (*(volatile unsigned int*)0x600e)=0; 
#define SWITCH_IN_BANK8    nBank=(unsigned int)0x6010; __asm__ __volatile__("":::"memory"); (*(volatile unsigned int*)0x6010)=0; 
#define SWITCH_IN_BANK9    nBank=(unsigned int)0x6012; __asm__ __volatile__("":::"memory"); (*(volatile unsigned int*)0x6012)=0; 
#define SWITCH_IN_BANK10   nBank=(unsigned int)0x6014; __asm__ __volatile__("":::"memory"); (*(volatile unsigned int*)0x6014)=0; 
#define SWITCH_IN_BANK11   nBank=(unsigned int)0x6016; __asm__ __volatile__("":::"memory"); (*(volatile unsigned int*)0x6016)=0; 
#define SWITCH_IN_BANK12   nBank=(unsigned int)0x6018; __asm__ __volatile__("":::"memory"); (*(volatile unsigned int*)0x6018)=0; 
#define SWITCH_IN_BANK13   nBank=(unsigned int)0x601a; __asm__ __volatile__("":::"memory"); (*(volatile unsigned int*)0x601a)=0; 
#define SWITCH_IN_BANK14   nBank=(unsigned int)0x601c; __asm__ __volatile__("":::"memory"); (*(volatile unsigned int*)0x601c)=0; 
#define SWITCH_IN_BANK15   nBank=(unsigned int)0x601e; __asm__ __volatile__("":::"memory"); (*(volatile unsigned int*)0x601e)=0; 

#ifdef F18A
#define SWITCH_IN_BANK16   nBank=(unsigned int)0x6020; __asm__ __volatile__("":::"memory"); (*(volatile unsigned int*)0x6020)=0;  
#define SWITCH_IN_BANK17   nBank=(unsigned int)0x6022; __asm__ __volatile__("":::"memory"); (*(volatile unsigned int*)0x6022)=0;  
#define SWITCH_IN_BANK18   nBank=(unsigned int)0x6024; __asm__ __volatile__("":::"memory"); (*(volatile unsigned int*)0x6024)=0; 
#define SWITCH_IN_BANK19   nBank=(unsigned int)0x6026; __asm__ __volatile__("":::"memory"); (*(volatile unsigned int*)0x6026)=0; 
#define SWITCH_IN_BANK20   nBank=(unsigned int)0x6028; __asm__ __volatile__("":::"memory"); (*(volatile unsigned int*)0x6028)=0; 	
#define SWITCH_IN_BANK21   nBank=(unsigned int)0x602a; __asm__ __volatile__("":::"memory"); (*(volatile unsigned int*)0x602a)=0; 
#define SWITCH_IN_BANK22   nBank=(unsigned int)0x602c; __asm__ __volatile__("":::"memory"); (*(volatile unsigned int*)0x602c)=0; 
#define SWITCH_IN_BANK23   nBank=(unsigned int)0x602e; __asm__ __volatile__("":::"memory"); (*(volatile unsigned int*)0x602e)=0; 
#define SWITCH_IN_BANK24   nBank=(unsigned int)0x6030; __asm__ __volatile__("":::"memory"); (*(volatile unsigned int*)0x6030)=0; 
#define SWITCH_IN_BANK25   nBank=(unsigned int)0x6032; __asm__ __volatile__("":::"memory"); (*(volatile unsigned int*)0x6032)=0; 
#define SWITCH_IN_BANK26   nBank=(unsigned int)0x6034; __asm__ __volatile__("":::"memory"); (*(volatile unsigned int*)0x6034)=0; 
#define SWITCH_IN_BANK27   nBank=(unsigned int)0x6036; __asm__ __volatile__("":::"memory"); (*(volatile unsigned int*)0x6036)=0; 
#define SWITCH_IN_BANK28   nBank=(unsigned int)0x6038; __asm__ __volatile__("":::"memory"); (*(volatile unsigned int*)0x6038)=0; 
#define SWITCH_IN_BANK29   nBank=(unsigned int)0x603a; __asm__ __volatile__("":::"memory"); (*(volatile unsigned int*)0x603a)=0; 
#define SWITCH_IN_BANK30   nBank=(unsigned int)0x603c; __asm__ __volatile__("":::"memory"); (*(volatile unsigned int*)0x603c)=0; 
#define SWITCH_IN_BANK31   nBank=(unsigned int)0x603e; __asm__ __volatile__("":::"memory"); (*(volatile unsigned int*)0x603e)=0; 
#endif

#else

// in Classic99 we can check if you accidentally passed in nBank
// I could check address, that was a good idea, but in one place I pass a calculated number,
// so that doesn't HAVE an address. And defines don't let you test by name.
static void SWITCH_IN_BANK(unsigned int nOldBank) {
    nBank = (nOldBank);
}

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

#ifdef F18A
#define SWITCH_IN_BANK16   nBank=(unsigned int)0x6020; 
#define SWITCH_IN_BANK17   nBank=(unsigned int)0x6022; 
#define SWITCH_IN_BANK18   nBank=(unsigned int)0x6024; 
#define SWITCH_IN_BANK19   nBank=(unsigned int)0x6026; 
#define SWITCH_IN_BANK20   nBank=(unsigned int)0x6028; 
#define SWITCH_IN_BANK21   nBank=(unsigned int)0x602a; 
#define SWITCH_IN_BANK22   nBank=(unsigned int)0x602c; 
#define SWITCH_IN_BANK23   nBank=(unsigned int)0x602e; 
#define SWITCH_IN_BANK24   nBank=(unsigned int)0x6030; 
#define SWITCH_IN_BANK25   nBank=(unsigned int)0x6032; 
#define SWITCH_IN_BANK26   nBank=(unsigned int)0x6034; 
#define SWITCH_IN_BANK27   nBank=(unsigned int)0x6036; 
#define SWITCH_IN_BANK28   nBank=(unsigned int)0x6038; 
#define SWITCH_IN_BANK29   nBank=(unsigned int)0x603a; 
#define SWITCH_IN_BANK30   nBank=(unsigned int)0x603c; 
#define SWITCH_IN_BANK31   nBank=(unsigned int)0x603e; 
#endif

#endif


#endif

/* eof */


