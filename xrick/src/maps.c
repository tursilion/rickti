/*
 * xrick/src/maps.c
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
 * NOTES
 *
 * A map is composed of submaps, which in turn are composed of rows of
 * 0x20 tiles. map_map contains the tiles for the current portion of the
 * current submap, i.e. a little bit more than what appear on the screen,
 * but not the whole submap.
 *
 * map_frow is map_map top row within the submap.
 *
 * Submaps are stored as arrays of blocks, each block being a 4x4 tile
 * array. map_submaps[].bnum points to the first block of the array.
 *
 * Before a submap can be played, it needs to be expanded from blocks
 * to map_map.
 */

#include "config.h"
#include "env.h"

#include "maps.h"

#include "game.h"
#include "debug.h"
#include "ents.h"
#include "draw.h"
#include "screens.h"
#include "e_sbonus.h"
#include "tiles.h"
#include "fb.h"

#include <vdp.h>

/*
 * global vars
 */
U8 map_map[0x2C][0x20];
U8 map_eflg[0x100];
U16 map_frow;
U16 map_tilesBank;


/*
 * prototypes
 */
static void map_eflg_expand(U16);

/*
 * Fill in map_map with tile numbers by expanding blocks.
 *
 * add map_submaps[].bnum to map_frow to find out where to start from.
 * We need to /4 map_frow to convert from tile rows to block rows, then
 * we need to *8 to convert from block rows to block numbers (there
 * are 8 blocks per block row). This is achieved by *2 then &0xfff8.
 */
void
map_expand(void)
{
  U16 i, j, k, l;
  U16 row, col;
  U16 pbnum;
  int tmpbnum;
  unsigned int nOldBank = nBank;

  SWITCH_IN_BANK12; // map_submaps

  pbnum = map_submaps[env_submap].bnum + ((2 * map_frow) & 0xfff8);
  row = col = 0;

  SWITCH_IN_BANK13; // map_bnums
  tmpbnum = map_bnums[pbnum];
  SWITCH_IN_BANK15; // map_blocks

  for (i = 0; i < 0x0b; i++) {  /* 0x0b rows of blocks */
    for (j = 0; j < 0x08; j++) {  /* 0x08 blocks per row */
      for (k = 0, l = 0; k < 0x04; k++) {  /* expand one block */
	    map_map[row][col++] = map_blocks[tmpbnum][l++];    // map_map is local, ram, map_maps is banked ROM
	    map_map[row][col++] = map_blocks[tmpbnum][l++];
	    map_map[row][col++] = map_blocks[tmpbnum][l++];
	    map_map[row][col]   = map_blocks[tmpbnum][l++];
	    row += 1; col -= 3;
      }
      row -= 4; col += 4;
      pbnum++;
      SWITCH_IN_BANK13; // map_bnums
      tmpbnum = map_bnums[pbnum];
      SWITCH_IN_BANK15; // map_blocks
    }
    row += 4; col = 0;
  }

  SWITCH_IN_BANK(nOldBank);

}


/*
 * Initialize a new submap
 *
 * ASM 0cc3
 */
void
map_init(void)
{
  unsigned int nOldBank = nBank;

  SWITCH_IN_BANK12;

	/*sys_printf("xrick/map_init: map=%#04x submap=%#04x\n", g_map, env_submap);*/
	map_tilesBank = map_submaps[env_submap].page == 1 ? 2 : 1;
	map_eflg_expand((map_submaps[env_submap].page == 1) ? 0x10 : 0x00);

    SWITCH_IN_BANK(nOldBank);

	map_expand();
	ent_reset();

	/* entities that are in the visible part of the map */
	ent_actvis(
		map_frow + MAPS_TOPHEIGHT_TL,
		map_frow + MAPS_TOPHEIGHT_TL+MAPS_VISHEIGHT_TL-1);

	/* entities that are in the hidden top of the map */
	ent_actvis(
		map_frow + 0,
		map_frow + MAPS_TOPHEIGHT_TL-1);

	/* entities that are in the hidden bottom of the map */
	ent_actvis(
		map_frow + MAPS_TOPHEIGHT_TL+MAPS_VISHEIGHT_TL,
		map_frow + MAPS_TOPHEIGHT_TL+MAPS_VISHEIGHT_TL+MAPS_BOTHEIGHT_TL-1);
}


/*
 * Expand entity flags for this map
 *
 * ASM 1117
 */
void
map_eflg_expand(U16 offs)
{
  U16 i, j, k;

  unsigned int nOldBank = nBank;
  SWITCH_IN_BANK14;

  for (i = 0, k = 0; i < 0x10; i++) {
    j = map_eflg_c[offs + i++];
    while (j--) map_eflg[k++] = map_eflg_c[offs + i];
  }

  SWITCH_IN_BANK(nOldBank);
}


/*
 * Chain (sub)maps
 *
 * ASM 0c08
 * return: TRUE/next submap OK, FALSE/map finished
 FIXME should return next submap number, or 0.
 */
U16 map_chain(void)
{
  U16 c, t;
  unsigned int nOldBank;

  env_changeSubmap = 0; /* FIXME but not used?! */
  e_sbonus_counting = FALSE; /* FIXME what? move this out of here!! */

  /* find connection */
  nOldBank = nBank;
  SWITCH_IN_BANK12; // map_submaps, map_connect

  c = map_submaps[env_submap].connect;
  t = 3;

  IFDEBUG_MAPS(
    sys_printf("xrick/maps: chain submap=%#04x frow=%#04x .connect=%#04x %s\n",
	       env_submap, map_frow, c,
	       (game_dir == LEFT ? "-> left" : "-> right"));
  );

  /*
   * look for the first connector with compatible row number. if none
   * found, then panic
   */
  for (c = map_submaps[env_submap].connect; ; c++) {
    if (map_connect[c].dir == 0xff)
      sys_panic("(map_chain) can not find connector\n");
    if (map_connect[c].dir != game_dir) continue;
    t = (ent_ents[1].y >> 3) + map_frow - map_connect[c].rowout;
    if (t < 3) break;
  }

  /* got it */
  IFDEBUG_MAPS(
    sys_printf("xrick/maps: chain frow=%#04x y=%#06x\n",
	       map_frow, ent_ents[1].y);
    sys_printf("xrick/maps: chain connect=%#04x rowout=%#04x - ",
	       c, map_connect[c].rowout);
    );

  if (map_connect[c].submap == 0xff) {
    /* no next submap - request next map */
    
    SWITCH_IN_BANK(nOldBank);
    IFDEBUG_MAPS(
      sys_printf("chain to next map\n");
      );
    return FALSE;
  }
  else  {
    /* next submap */
    IFDEBUG_MAPS(
      sys_printf("chain to submap=%#04x rowin=%#04x\n",
		 map_connect[c].submap, map_connect[c].rowin);
      );
    map_frow = map_frow - map_connect[c].rowout + map_connect[c].rowin;
    env_submap = map_connect[c].submap;
    IFDEBUG_MAPS(
      sys_printf("xrick/maps: chain frow=%#04x\n",
		 map_frow);
      );

    SWITCH_IN_BANK(nOldBank);
    return TRUE;
  }
}


/*
 * Reset all marks, i.e. make them all active again.
 *
 * ASM 0025
 *
 */
void
map_resetMarks(void)
{
  U16 i;
  unsigned int nOldBank = nBank;
  SWITCH_IN_BANK14;
  for (i = 0; i < MAP_NBR_MARKS; i++) {
    map_marks_ent[i] = map_marks[i].ent;
  }
  SWITCH_IN_BANK(nOldBank);
}


/* CLEAN BELOW THIS LINE */

/*
 * maps_paint
 *
 * paints the current map to the frame buffer.
 */
void maps_paint(void)
{
	U16 i;
	int f;
    unsigned int nOldBank = nBank;

    // only reloads if it needs to
	tiles_setBank(map_tilesBank);

    SWITCH_IN_BANK12;

    VDP_INT_DISABLE;

	for (i = 1; i < 24; i++) /* 23 rows, cause we skip the status row 0 */
	{
		f = fb_at(0, i * 8);   // gets VDP offset into gImage
        vdpmemcpy(gImage+f, map_map[i+8], 32);
	}

    VDP_INT_ENABLE;

    SWITCH_IN_BANK(nOldBank);
}



/*
 * maps_paintRect
 *
 * paints a portion of the map at <x>, <y> of size <width>, <height>.
 * <x>, <y> expressed in map/px.
 */
void maps_paintRect(U16 x, U16 y, U16 width, U16 height)
{
	U16 x_fb, y_fb;
	int fb;
	U16 r, c;

	/* align to tiles */
	maps_alignRect(&x, &y, &width, &height);

	/* clip */
	if (maps_clip(x, y, width, height))  /* return if not visible */
		return;

	/* convert to fb/px */
	x_fb = x - MAPS_FB_X;
	y_fb = y - MAPS_FB_Y;

	/* convert map/px to map/tl */
	x >>= 3;
	y >>= 3;
	width >>= 3;
	height >>= 3;

	/* draw */
    VDP_INT_DISABLE;
	for (r = 0; r < height; r++) /* for each tile row */
	{
		fb = fb_at(x_fb, 8 + y_fb + r * 8); /* FIXME +8? */
        vdpmemcpy(fb+gImage, &map_map[y+r][x], width);
	}
    VDP_INT_ENABLE;
}



/*
 * maps_alignRect
 *
 * aligns a rectangle at <x>, <y> of size <width>, <height> to tiles.
 * coordinates expressed in map/px.
 * resulting rectangle might be bigger.
 */
void maps_alignRect(U16 *x, U16 *y, U16 *width, U16 *height)
{
	U16 xa, ya;
	U16 wa, ha;

  	/* align to column and row */
	xa = *x & 0xfff8;
	ya = *y & 0xfff8;

	/* grow width and height to cover tiles */
	*width += *x - xa;
	*height += *y - ya;
	wa = *width + 8 - (*width % 8);
	ha = *height + 8 - (*height % 8);

	*x = xa;
	*y = ya;
	*width = wa;
	*height = ha;
}



/*
 * maps_clip
 *
 * clips a rectangle at <x>, <y> of size <width>, <height>.
 * <x>, <y> expressed in map/px.
 * returns TRUE if fully clipped, FALSE if still (at least partly) visible.
 * NOTE: original would actually clip the sprite. We can't "really" do this,
 * but we can allow a partially visible sprite at the bottom or top - but we do
 * NOT adjust the settings as that moves the sprite, doesn't crop it
 * Since we can't crop anymore, we no longer take pointers for the input parameters
 */
U16 maps_clip(U16 x, U16 y, U16 width, U16 height)
{
	if (x < 0)
	{
		if (x + width < 0)
			return TRUE;
#ifdef ALLOW_CROPS
		else
		{
			*width += *x;
			*x = 0;
		}
#endif
    }
	else
	{
		if (x >= MAPS_WIDTH_PX)
			return TRUE;
#ifdef ALLOW_CROPS
		else
		if (*x + *width > MAPS_WIDTH_PX)
		{
			*width = MAPS_WIDTH_PX - *x;
		}
#endif
    }

	if (y < MAPS_TOPHEIGHT_PX)
	{
		if ((y + height) <= MAPS_TOPHEIGHT_PX)
			return TRUE;
#ifdef ALLOW_CROPS
		else
		{
			*height += *y - MAPS_TOPHEIGHT_PX;
			*y = MAPS_TOPHEIGHT_PX;
		}
#endif
    }
	else
	{
		if (y >= MAPS_TOPHEIGHT_PX+MAPS_VISHEIGHT_PX)
			return TRUE;
#ifdef ALLOW_CROPS
		else
		if (*y + *height > MAPS_TOPHEIGHT_PX+MAPS_VISHEIGHT_PX)
			*height = MAPS_TOPHEIGHT_PX+MAPS_VISHEIGHT_PX - *y;
#endif
    }

	return FALSE;
}




/* eof */
