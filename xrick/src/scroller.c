/*
 * xrick/src/scroller.c
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

#include "game.h"
#include "env.h"

#include "scroller.h"
#include <string.h>
#include "debug.h"
#include "draw.h"
#include "maps.h"
#include "ents.h"
#include <vdp.h>

static U16 period = 0;
static void (*myasmscrup)(void*,void*,void*) = NULL;
static void (*myasmscrdn)(void*,void*,void*) = NULL;
static void (*myasmvdpcp)(U8*, const void*, U16) = NULL;

/* load the asm to scratchpad */
void scroll_init(void)
{
#ifndef CLASSIC99
    void *pDest = (void*)0x8340;
    memcpy(pDest, asmscrup, (U8*)&asmscrend-(U8*)&asmscrup);
    myasmscrup = (void(*)(void*,void*,void*))0x8340;
    myasmscrdn = (void(*)(void*,void*,void*))0x834A;
    myasmvdpcp = (void(*)(U8*, const void*, U16))0x8364;
#endif
}

/*
 * Scroll up
 *
 */
U16 scroll_up(void)
{
  U16 i;
  static U16 n = 0;

  /* last call: restore */
  if (n == 8) {
    n = 0;
    game_period = period;
    return SCROLL_DONE;
  }

  /* first call: prepare */
  if (n == 0) {
    period = game_period;
    game_period = SCROLL_PERIOD;
  }

  /* translate map */
#ifdef CLASSIC99
  for (i = MAP_ROW_SCRTOP; i < MAP_ROW_HBBOT; i++) {
      memcpy(map_map[i], map_map[i+1], 32);
  }
#else
  void *dest = &map_map[MAP_ROW_SCRTOP];
  void *src = &map_map[MAP_ROW_SCRTOP+1];
  void *end = &map_map[MAP_ROW_HBBOT];
  myasmscrup(dest, src, end);
#endif

  /* translate entities */
  for (i = 0; ent_ents[i].n != 0xFF; i++) {
    if (ent_ents[i].n) {
      ent_ents[i].ysave -= 8;
      ent_ents[i].trig_y -= 8;
      ent_ents[i].y -= 8;
      if (ent_ents[i].y & 0x8000) {  /* map coord. from 0x0000 to 0x0140 */
	IFDEBUG_SCROLLER(
	  sys_printf("xrick/scroller: entity %#04X is gone\n", i);
	  );
        delete_ent(i);
      }
    }
  }

  /* display */
  maps_paint();
  ents_paintAll();
  env_paintGame();
  map_frow++;

  /* loop */
  if (n++ == 7) {
    /* activate visible entities */
    ent_actvis(map_frow + MAP_ROW_HBTOP, map_frow + MAP_ROW_HBBOT);

    /* prepare map */
    map_expand();

    /* display */
	maps_paint();
    ents_paintAll();
    env_paintGame();
  }

  return SCROLL_RUNNING;
}

/*
 * Scroll down
 *
 */
U16 scroll_down(void)
{
  U16 i;
  static U16 n = 0;

  /* last call: restore */
  if (n == 8) {
    n = 0;
    game_period = period;
    return SCROLL_DONE;
  }

  /* first call: prepare */
  if (n == 0) {
    period = game_period;
    game_period = SCROLL_PERIOD;
  }

  /* translate map */
#ifdef CLASSIC99
  for (i = MAP_ROW_SCRBOT; i > MAP_ROW_HTTOP; i--) {
      memcpy(map_map[i], map_map[i-1], 32);
  }
#else
  void *src = &map_map[MAP_ROW_SCRBOT-1];
  void *dest = &map_map[MAP_ROW_SCRBOT];
  void *end = &map_map[MAP_ROW_HTTOP];
  myasmscrdn(dest, src, end);
#endif

  /* translate entities */
  for (i = 0; ent_ents[i].n != 0xFF; i++) {
    if (ent_ents[i].n) {
      ent_ents[i].ysave += 8;
      ent_ents[i].trig_y += 8;
      ent_ents[i].y += 8;
      if (ent_ents[i].y > 0x140) {  /* map coord. from 0x0000 to 0x0140 */
	IFDEBUG_SCROLLER(
	  sys_printf("xrick/scroller: entity %#04X is gone\n", i);
	  );
        delete_ent(i);
      }
    }
  }

  /* display */
	maps_paint();
  ents_paintAll();
  env_paintGame();
  map_frow--;

  /* loop */
  if (n++ == 7) {
    /* activate visible entities */
    ent_actvis(map_frow + MAP_ROW_HTTOP, map_frow + MAP_ROW_HTBOT);

    /* prepare map */
    map_expand();

    /* display */
	maps_paint();
    ents_paintAll();
    env_paintGame();
  }

  return SCROLL_RUNNING;
}

/* generic fast replacement for vdpmemcpy */
void vdpmemcpy2(U16 dest, const U8* src, U16 cnt) {
    VDP_SET_ADDRESS_WRITE(dest);
    myasmvdpcp((U8*)0x8c00, src, cnt);
}

/* eof */
