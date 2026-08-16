/* 
 128k banked cartridge with 32k RAM support
 Fixed bank is copied into 32k by the CRT0.
 The cartridge builder deal with how that works,
 and we will assume a maximum size of 24k (the
 entire high memory bank). All other pages here
 become 8k
*/

OUTPUT_FORMAT("elf32-tms9900", "elf32-tms9900",
              "elf32-tms9900")
OUTPUT_ARCH(tms9900)
ENTRY(_start)
SEARCH_DIR("/home/tursi/gcc9900/tms9900/lib");

/* banked offset of 46 bytes to account for header space */
/* We also make the 24k block smaller by 46*3 so we can store it across 3 banks */
/* Although this script gives 24k, note we lose an extra */
/* 92 bytes for the headers in page 2 and 3 (46 bytes each) */

MEMORY
{
    INIT(xr) : ORIGIN = 0x6000, LENGTH = 512

    FIXED(xrw) : ORIGIN = 0xA000, LENGTH = 24K
    STACK(xrw) : ORIGIN = 0x2000, LENGTH = 8K
    SCRATCH(rw) : ORIGIN = 0x8340, LENGTH = 48
    CART1(XR) : ORIGIN = 0x6028, LENGTH = 8152
    CART2(XR) : ORIGIN = 0x6028, LENGTH = 8152
    CART3(XR) : ORIGIN = 0x6028, LENGTH = 8152
    CART4(XR) : ORIGIN = 0x6028, LENGTH = 8152
    CART5(XR) : ORIGIN = 0x6028, LENGTH = 8152
    CART6(XR) : ORIGIN = 0x6028, LENGTH = 8152
    CART7(XR) : ORIGIN = 0x6028, LENGTH = 8152
    CART8(XR) : ORIGIN = 0x6028, LENGTH = 8152
    CART9(XR) : ORIGIN = 0x6028, LENGTH = 8152
    CART10(XR) : ORIGIN = 0x6028, LENGTH = 8152
    CART11(XR) : ORIGIN = 0x6028, LENGTH = 8152
    CART12(XR) : ORIGIN = 0x6028, LENGTH = 8152
    CART13(XR) : ORIGIN = 0x6028, LENGTH = 8152
    CART14(XR) : ORIGIN = 0x6028, LENGTH = 8152
    CART15(XR) : ORIGIN = 0x6028, LENGTH = 8152
    
    FLASH(XR) : ORIGIN = 0, LENGTH = 256K
}

/* scratchpad usage (inferred and investigated)
   TI Interrupt configured to minimum usage

8300-831F   C workspace
8320-833F   C interrupt workspace (sound will use all of it)
8340-836F   free (though floating point will use from 834A on)
8370        VDP RAM top (we don't care)
8374        KSCAN mode
8375        Key
8376        Joystick Y
8377        Joystick X
8378        Rnd seed
8379        VDP Interrupt timer
837B        VDP status byte mirror
837C        GPL Status (may be important)
837E-83BF   free
83C0-83DF   Interrupt workspace, includes flags, VDPR1 mirror, screen timeout and interrupt hook
83E0-83FF   GPLWS (unplanned peripheral interrupts could use any of it)

*/

SECTIONS {
    PROVIDE (__executable_start = 0xA000); 

    PROVIDE (stext = .);
    
    . = 0x6000;
    
    loader :
    {
        crt0.o
    } > INIT AT> FLASH
    
    . = 0xA000;
    fixed :
    {
        vdpint.o
        control.o(.text .rodata)
        dat_screens.o(.text .rodata)
        draw.o(.text .rodata)
        e_bomb.o(.text .rodata)
        e_bonus.o(.text .rodata)
        e_box.o(.text .rodata)
        e_bullet.o(.text .rodata)
        e_rick.o(.text .rodata)
        e_sbonus.o(.text .rodata)
        e_them.o(.text .rodata)
        ents.o(.text .rodata)
        env.o(.text .rodata)
        fb.o(.text .rodata)
        game.o(.text .rodata)
        img.o(.text .rodata)
        maps.o(.text .rodata)
        scr_pause.o(.text .rodata)  /* too small to be worth the cost of banking */
        sounds.o(.text .rodata)     /* leave in RAM */
        sprites.o(.text .rodata)    /* leave in RAM */
        sysarg.o(.text .rodata)
        syskbd.o(.text .rodata)
        system.o(.text .rodata)
        sysvid.o(.text .rodata)
        tiles.o(.text .rodata)
        util.o(.text .rodata)
        xrick.o(.text .rodata)
        CPlayerCommonHandEdit.o(.text .rodata)
        CPlayerTIHandEdit.o(.text .rodata)
        CPlayerTIHandlers.o(.text .rodata)
        scroller.o(.text .rodata)
        scratchpad.o(.text .rodata) /* keep associated with scroller.o */
        *libti99.a:*(.text .rodata)
    } > FIXED AT> FLASH
    
    PROVIDE (etext = .);

    /* bank0, 1 and 2 are used for the 24k of fixed code */
    
    bank3 :
    {
        /* must start at beginning of bank, index is hard coded in sprites.c sprites_paint */
        dat_spritesTI0.o(.text .rodata)
        /* --NOTHING BUT THE SPRITES BEFORE THIS-- */
        scr_getname.o(.text .rodata)
        PROVIDE(ebank3a = .);
    } > CART3 AT> FLASH

    bank4 :
    {
        /* must start at beginning of bank and be sequential after 0 */
        dat_spritesTI1.o(.text .rodata)
        /* --NOTHING BUT THE SPRITES BEFORE THIS-- */
        scr_imain.o(.text .rodata)
        sysevt.o(.text .rodata)
        PROVIDE(ebank4a = .);
    } > CART4 AT> FLASH

    bank5 :
    {
        /* must start at beginning of bank and be sequential after 1 */
        dat_spritesTI2.o(.text .rodata)
        /* --NOTHING BUT THE SPRITES BEFORE THIS-- */
        tigrom.o(.text .rodata)
        PROVIDE(ebank5a = .);
    } > CART5 AT> FLASH

    bank6 :
    {
        /* must start at beginning of bank and be sequential after 2 */
        dat_spritesTI3.o(.text .rodata)
        /* --NOTHING BUT THE SPRITES BEFORE THIS-- */
        PROVIDE(ebank6a = .);
    } > CART6 AT> FLASH

    bank7 :
    {
        /* must start at beginning of bank and be sequential after 3 */
        dat_spritesTI4.o(.text .rodata)
        /* --NOTHING BUT THE SPRITES BEFORE THIS-- */
        dat_ents2.o(.text .rodata)
        PROVIDE(ebank7a = .);
    } > CART7 AT> FLASH

    bank8 :
    {
        dat_tilespTI.o(.text .rodata)
        PROVIDE(ebank8a = .);
    } > CART8 AT> FLASH

    bank9 :
    {
        dat_tilescTI.o(.text .rodata)
        PROVIDE(ebank9a = .);
    } > CART9 AT> FLASH

    bank10 :
    {
        scr_gameover.o(.text .rodata)
        dat_picspTI.o(.text .rodata)
        PROVIDE(ebank10a = .);
    } > CART10 AT> FLASH

    bank11 :
    {
        dat_picscTI.o(.text .rodata)
        PROVIDE(ebank11a = .);
    } > CART11 AT> FLASH

    bank12 :
    {
        dat_maps1.o(.text .rodata)
        soundbank.o(.text .rodata)
        PROVIDE(ebank12a = .);
    } > CART12 AT> FLASH

    bank13 :
    {
        dat_maps2.o(.text .rodata)
        PROVIDE(ebank13a = .);
    } > CART13 AT> FLASH

    bank14 :
    {
        dat_maps3.o(.text .rodata)
        dat_ents.o(.text .rodata)
        PROVIDE(ebank14a = .);
    } > CART14 AT> FLASH

    bank15 :
    {
        dat_maps4.o(.text .rodata)
        scr_imap.o(.text .rodata)
        PROVIDE(ebank15a = .);
    } > CART15 AT> FLASH

    . = 0x2080;
    PROVIDE(sdata = .);
    .data : 
    { 
        *(.data) 
    } > STACK AT> FLASH
    PROVIDE(edata = .);
    
    PROVIDE(sbss = .);
    .bss :  
    { 
        *(.bss) 
        libti99.a:*(.bss)  
    } > STACK AT> FLASH

    PROVIDE(ebss = .);

    songDatVars : 
    { 
        *(.songDatVars) 
    } > STACK AT> FLASH
    
    _end = .; 
    PROVIDE (end = .);
}
