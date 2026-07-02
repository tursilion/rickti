/* 
 256k banked cartridge with 32k RAM support
 Fixed bank is copied into 32k by the CRT0.
 The cartridge builder deal with how that works,
 and we will assume a maximum size of 24k (the
 entire high memory bank). All other pages here
 become 8k. Note: anything shared with the 9918A
 version must be linked to the same page!
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
    
    /* f18a only */
    CART16(XR) : ORIGIN = 0x6028, LENGTH = 8152
    CART17(XR) : ORIGIN = 0x6028, LENGTH = 8152
    CART18(XR) : ORIGIN = 0x6028, LENGTH = 8152
    CART19(XR) : ORIGIN = 0x6028, LENGTH = 8152
    CART20(XR) : ORIGIN = 0x6028, LENGTH = 8152
    CART21(XR) : ORIGIN = 0x6028, LENGTH = 8152
    CART22(XR) : ORIGIN = 0x6028, LENGTH = 8152
    CART23(XR) : ORIGIN = 0x6028, LENGTH = 8152
    CART24(XR) : ORIGIN = 0x6028, LENGTH = 8152
    CART25(XR) : ORIGIN = 0x6028, LENGTH = 8152
    CART26(XR) : ORIGIN = 0x6028, LENGTH = 8152
    CART27(XR) : ORIGIN = 0x6028, LENGTH = 8152
    CART28(XR) : ORIGIN = 0x6028, LENGTH = 8152
    CART29(XR) : ORIGIN = 0x6028, LENGTH = 8152
    CART30(XR) : ORIGIN = 0x6028, LENGTH = 8152
    CART31(XR) : ORIGIN = 0x6028, LENGTH = 8152
    
    FLASH(XR) : ORIGIN = 0, LENGTH = 256K
}

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
        /* here this is the F18A build */
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
        /* these three do NOT contain F18A code, should be okay to share */

        /* must start at beginning of bank, index is hard coded in sprites.c sprites_paint */
        /* 9918A data */
        /* dat_spritesTI0.o(.text .rodata) - exactly 6k */
        /* --NOTHING BUT THE SPRITES BEFORE THIS-- */
        /*scr_getname.o(.text .rodata)*/
        /*sysevt.o(.text .rodata)*/
        
        FILL(0xFF);
        . = . + 8019;  /* size of sprite data and base code */
        BYTE(0xFF);

        PROVIDE(ebank3a = .);
    } > CART3 AT> FLASH

    bank4 :
    {
        /* scr_imain.o contains F18A code, need a separate one. But keeping sysevt is */
        /* made more complicated, so we'll just drop the this bank and replace both */
        /* 9918A data */
        /* dat_spritesTI1.o(.text .rodata) */
        /* --NOTHING BUT THE SPRITES BEFORE THIS-- */
        /* scr_imain.o(.text .rodata) */
        /* sysevt.o(.text .rodata) */

        FILL(0xFF);
        . = . + 8083;
        BYTE(0xFF);
        
        PROVIDE(ebank4a = .);
    } > CART4 AT> FLASH 

    bank5 :
    {
        /* only has the sprite data - if needed we COULD put F18A stuff here */
        /*dat_spritesTI2.o(.text .rodata)*/

        FILL(0xFF);
        . = . + 6144;
        tigrom.o(.text .rodata)
   
        PROVIDE(ebank5a = .);
    } > CART5 AT> FLASH

    bank6 :
    {
        /* only has the sprite data - if needed we COULD put F18A stuff here */
        /*dat_spritesTI3.o(.text .rodata)*/

        FILL(0xFF);
        . = . + 6143;
        BYTE(0xFF);

        PROVIDE(ebank6a = .);
    } > CART6 AT> FLASH

    bank7 :
    {
        /* no f18a - okay to share */

        /* must start at beginning of bank and be sequential after 3 */
        /*dat_spritesTI4.o(.text .rodata)*/

        FILL(0xFF);
        . = . + 2688;     /* size of sprite data (note smaller size!) */
        dat_ents2.o(.text .rodata)

        PROVIDE(ebank7a = .);
    } > CART7 AT> FLASH

    bank8 :
    {
        /* only has the tile data - if needed we COULD put F18A stuff here */
        /*dat_tilespTI.o(.text .rodata)*/

        FILL(0xFF);
        . = . + 6143;
        BYTE(0xFF);

        PROVIDE(ebank8a = .);
    } > CART8 AT> FLASH

    bank9 :
    {
        /* only has the tile data - if needed we COULD put F18A stuff here */
        /*dat_tilescTI.o(.text .rodata)*/

        FILL(0xFF);
        . = . + 6143;
        BYTE(0xFF);

        PROVIDE(ebank9a = .);
    } > CART9 AT> FLASH

    bank10 :
    {
        /* we can't reuse the title or tile memory, but code is safe */
        /*scr_gameover.o(.text .rodata)*/
        /*dat_picspTI.o(.text .rodata)*/
        FILL(0xFF);
        . = . + 7202;   /* note larger size for pic */
        BYTE(0xFF);
        
        PROVIDE(ebank10a = .);
    } > CART10 AT> FLASH

    bank11 :
    {
        /* only has the image data - if needed we COULD put F18A stuff here */
        /*dat_picscTI.o(.text .rodata)*/

        FILL(0xFF);
        . = . + 6911;   /* note larger size */
        BYTE(0xFF);
        
        PROVIDE(ebank11a = .);
    } > CART11 AT> FLASH

    bank12 :
    {
        /* safe to reuse */
        
        dat_maps1.o(.text .rodata)
        soundbank.o(.text .rodata)
        PROVIDE(ebank12a = .);
    } > CART12 AT> FLASH

    bank13 :
    {
        /* safe to reuse */
        
        dat_maps2.o(.text .rodata)
        PROVIDE(ebank13a = .);
    } > CART13 AT> FLASH

    bank14 :
    {
        /* safe to reuse */
        
        dat_maps3.o(.text .rodata)
        dat_ents.o(.text .rodata)
        PROVIDE(ebank14a = .);
    } > CART14 AT> FLASH

    bank15 :
    {
        /* safe to reuse maps, but not imap, so just declare the bank full */
        dat_maps4.o(.text .rodata)

        /*scr_imap.o(.text .rodata)*/
        FILL(0xFF);
        . = . + 2371;
        BYTE(0xFF);
        
        PROVIDE(ebank15a = .);
    } > CART15 AT> FLASH


    /* below here is specific to the F18A build */

    bank16 :
    {
        sprf00.o(.text .rodata)
        scr_imain.o(.text .rodata)
        scr_gameover.o(.text .rodata)

        PROVIDE(ebank16a = .);
    } > CART16 AT> FLASH

    bank17 :
    {
        sprf10.o(.text .rodata)
        splashf18_splitcol1.o(.text .rodata)

        PROVIDE(ebank17a = .);
    } > CART17 AT> FLASH

    bank18 :
    {
        sprf20.o(.text .rodata)
        splashf18_splitcol2.o(.text .rodata)

        PROVIDE(ebank18a = .);
    } > CART18 AT> FLASH

    bank19 :
    {
        sprf30.o(.text .rodata)

        splashf18_splitcol3.o(.text .rodata)
        PROVIDE(ebank19a = .);
    } > CART19 AT> FLASH

    bank20 :
    {
        sprf40.o(.text .rodata)
        tilesf18_splitcol1.o(.text .rodata)
        tilesf18_splitpat1.o(.text .rodata)

        PROVIDE(ebank20a = .);
    } > CART20 AT> FLASH

    bank21 :
    {
        sprf01.o(.text .rodata)
        dat_hafcTI.o(.text .rodata)
        PROVIDE(ebank21a = .);
    } > CART21 AT> FLASH

    bank22 :
    {
        sprf11.o(.text .rodata)
        dat_hafpTI.o(.text .rodata)
        PROVIDE(ebank22a = .);
    } > CART22 AT> FLASH

    bank23 :
    {
        sprf21.o(.text .rodata)
        sysevt.o(.text .rodata)
        splashf18_splitcol4.o(.text .rodata)
        PROVIDE(ebank23a = .);
    } > CART23 AT> FLASH

    bank24 :
    {
        sprf31.o(.text .rodata)
        splashf18_splitpat1.o(.text .rodata)
        PROVIDE(ebank24a = .);
    } > CART24 AT> FLASH

    bank25 :
    {
        sprf41.o(.text .rodata)
        tilesf18_splitcol2.o(.text .rodata)
        tilesf18_splitpat2.o(.text .rodata)
        scr_getname.o(.text .rodata)        

        PROVIDE(ebank25a = .);
    } > CART25 AT> FLASH

    bank26 :
    {
        sprf02.o(.text .rodata)
        splashf18_splitpat2.o(.text .rodata)
        PROVIDE(ebank26a = .);
    } > CART26 AT> FLASH

    bank27 :
    {
        sprf12.o(.text .rodata)
        splashf18_splitpat3.o(.text .rodata)
        PROVIDE(ebank27a = .);
    } > CART27 AT> FLASH

    bank28 :
    {
        sprf22.o(.text .rodata)
        splashf18_splitpat4.o(.text .rodata)
        PROVIDE(ebank28a = .);
    } > CART28 AT> FLASH

    bank29 :
    {
        sprf32.o(.text .rodata)
        PROVIDE(ebank29a = .);
    } > CART29 AT> FLASH

    bank30 :
    {
        sprf42.o(.text .rodata)
        scr_imap.o(.text .rodata)
        PROVIDE(ebank30a = .);
    } > CART30 AT> FLASH

    bank31 :
    {
        tilesf18_splitcol3.o(.text .rodata)
        tilesf18_splitpat3.o(.text .rodata)

        PROVIDE(ebank31a = .);
    } > CART31 AT> FLASH

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
