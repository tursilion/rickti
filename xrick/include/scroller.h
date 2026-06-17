/*
 * xrick/include/scroller.h
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

#ifndef _SCROLLER_H
#define _SCROLLER_H

#define SCROLL_RUNNING 1
#define SCROLL_DONE 0

#define SCROLL_PERIOD 24

void scroll_init(void);
extern U16 scroll_up(void);
extern U16 scroll_down(void);
extern void vdpmemcpy2(U16 dest, const U8* src, U16 cnt);

// asm
extern void asmscrup(void* dest, void* src, void* end);
extern void asmscrdn(void* dest, void* src, void* end);
extern void asmvdpcp(U8* vdpwd, void *src, U16 evencnt);
extern void* asmscrend;


#endif

/* eof */


