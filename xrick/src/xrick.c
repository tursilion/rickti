/*
 * XRICK
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
#include "ricksystem.h"
#include "sysvid.h"
#include "game.h"
#include "fb.h"
#include "sprites.h"
#include "scroller.h"
#include "tigrom.h"

#include <vdp.h>

#ifdef F18A
#include <f18a.h>
extern void vsetchar_base(int pAddr, unsigned char ch);
#endif

// timer updated by VDP interrupt - making it 32 bit so it can represent milliseconds better
// without wrapping very often. Note we are doing milliseconds*10 (so we can count by 16.6ms)
// TODO: this is wrong for PAL, obviously, which counts at 20ms
#ifndef CLASSIC99
volatile U16 vdpCountH, vdpCountL;
#else
// warning: there is a chance of race here between the two bytes...
// we'll see if we can get away without vdpCountH
U16 getVdpCount() { return ReadByteFromClassic99(0xa102)*256 + ReadByteFromClassic99(0xa103); }
void setVdpCount(U16 val) { WriteWordToClassic99(0xa102, val); }
#endif

// current bank, used so we can switch and switch back
unsigned int nBank;

// reset time at startup
void sys_resettime() {
#ifndef CLASSIC99
    VDP_INT_DISABLE;
    vdpCountL = 0;
    vdpCountH = 0;
    VDP_INT_ENABLE;
#else
    setVdpCount(0);
#endif
}

// gcc doesn't support naked functions, so we need to write the interrupt handler in assembly...
extern void sys_vdpint();

/*
 * Sets a console, if possible
 */
static void setConsole()
{
	// NOTE: does not handle parent process console being redirected
	// eg "./xrick > stdout.txt" still writes to the actual console

#ifdef __WIN32__
	// try to attach to parent process console
	if (AttachConsole(-1))
	{
		// reopen (SDL2 closed everything)
		freopen("CONIN$", "r", stdin);
		freopen("CONOUT$", "w", stdout);
		freopen("CONOUT$", "w", stderr);
	}
#endif
}

 /*
  * Initialize system
  */
void
sys_init(int argc, char** argv)
{
    (void)argc;
    (void)argv;

    // make sure interrupts are off during init
    VDP_INT_DISABLE;

    // get banking into a known state
    SWITCH_IN_BANK0;

    VDP_SET_REGISTER(VDP_REG_COL, COLOR_BLACK);

#ifdef F18A
    // VRAM Map (ingame)
    // 0000    Pattern (only 2k cause of masking working)
    // 0800    5K of Rick sprites
    // 1C00    screen image table
    // 1F00    sprite attribute table
    // 1F80    -- free 128 bytes --
    // 2000    Color (also 2k)
    // 2800    Sprite colors (2k table) bit 0    
    // 3000    Sprite colors bit 1
    // 3800    Sprite colors bit 2
    // 4000    Rick sprite cache .5k (ish)
    // 4300    GPU program
    // 4???    -- free ?? bytes --

    // detect also unlocks for us
    if (!detect_f18a()) {
        // the loader loaded the wrong program!
        sys_panic("F18A required!");
    }

    // disable sprite flicker
    VDP_SET_REGISTER(F18A_REG_MAXSPR, 32);

#else
    // VRAM map
    //      0000    pattern
    //      1800    image table
    //      1B00    sprite
    //      1B80    -- free --
    //      2000    color
    //      3800    sprite patterns (192 chars)
    //      3E00    -- free --
#endif

    set_bitmap(VDP_SPR_16x16);
#ifdef F18A
    // we need to move a few tables around
    VDP_SET_REGISTER(VDP_REG_SIT, 7);
    gImage = 0x1c00;
    VDP_SET_REGISTER(VDP_REG_SAL, 62);
    gSprite = 0x1f00;
    VDP_SET_REGISTER(VDP_REG_SDT, 5);
    gSpritePat = 0x2800;
#else
	VDP_SET_REGISTER(VDP_REG_SDT, 7);	// remap sprite pattern table to not overlap the SIT
    gSpritePat = 0x3800;
    vdpmemset(gSpritePat, 0, 0x800);    // make sure it's zeroed
#endif

    // get a blank screen up by initializing all three char 0 to blank and then writing all zeros to the SIT
    fb_clear();

    // load high scores if we have GRAM
    SWITCH_IN_BANK5;
    loadgrom();
    SWITCH_IN_BANK0;

    // load scroll assembly to scratchpad
    scroll_init();

    // reset timing and interrupt system
    sys_resettime();
#ifndef CLASSIC99
    VDP_INT_CTRL = VDP_INT_CTRL_DISABLE_ALL;
    setUserIntHook(sys_vdpint);
#else
    WriteByteToClassic99(0x83C2, VDP_INT_CTRL_DISABLE_ALL);
    // We NEED the interrupt routine for this to work
    // So first we need to load one
/*
   1            * some hacks for Classic99 debug:
   2                AORG >a080
   3            * use a dedicated workspace, but use the main stack pointer
   4  A080 02E0     lwpi >8320
   4  A082 8320  
   5  A084 C2A0     mov @>8314,R10      * get the real stack pointer
   5  A086 8314  
   6            
   7  A088 0200     li r0,166           * ms*10
   7  A08A 00A6  
   8  A08C A800     a r0,@>A102      * hacked so vdpCount is at A102
   8  A08E A102  
   9            
  10            * Can't do this right now - the sprite table is in PC RAM, not Classic99
  11            *    li r3,>80
  12            *    li r2,sprite_table
  13            *    mov @gSprite,r1
  14            *    bl @vdpmemcpy
  15            
  16  A090 02E0     lwpi >83E0
  16  A092 83E0  
  17  A094 045B     b *r11
*/
    WriteWordToClassic99(0xA080 , 0x02E0);
    WriteWordToClassic99(0xA082 , 0x8320);
    WriteWordToClassic99(0xA084 , 0xC2A0);
    WriteWordToClassic99(0xA086 , 0x8314);
    WriteWordToClassic99(0xA088 , 0x0200);
    WriteWordToClassic99(0xA08A , 0x00A6);
    WriteWordToClassic99(0xA08C , 0xA800);
    WriteWordToClassic99(0xA08E , 0xA102);
    WriteWordToClassic99(0xA090 , 0x02E0);
    WriteWordToClassic99(0xA092 , 0x83E0);
    WriteWordToClassic99(0xA094 , 0x045B);

    // set the screen timeout to an odd value so it never expires
    WriteByteToClassic99(0x83d7, 1);

    // Load the pointer
    WriteWordToClassic99(0x83c4, 0xa080);
#endif                            
    VDP_CLEAR_VBLANK;             
                                  
    // Note: we run with interrupts enabled, but we expect no VDP activity during blank
    VDP_INT_ENABLE;
}



/*
 * Shutdown system
 */
void
sys_shutdown(void)
{
#ifdef F18A
    reset_f18a();
#endif
}


/*
 * main
 */
#ifdef CLASSIC99
extern void classic99_main();
#endif

int
main()
{
#ifdef CLASSIC99
    // this is a stupid hack and causes a little recursion, but it's just for test
    static int first = 1;
    if (first) {
        first = 0;
        classic99_main();
    }
#endif

	sys_init(0, 0);

	game_run(0);

	sys_shutdown();
	return 0;
}

/* eof */
