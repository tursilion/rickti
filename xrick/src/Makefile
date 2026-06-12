# Paths to TMS9900 compilation tools 
# ( Set in environment to override paths )
#TMS9900_DIR?=/home/tursilion/gcc9900/bin
TMS9900_DIR?=/home/tursi/newtms9900-gcc/newgcc9900/bin
ELF2EA5_DIR?=/home/tursi/gcc9900/bin
EA5_SPLIT_DIR?=/home/tursi/gcc9900/bin
CLASSIC99_DSK1?=/mnt/d/classic99/DSK1/
LIBTI99?=/mnt/d/work/ti/libti99/

# Full paths to the executables used
GAS=$(TMS9900_DIR)/tms9900-as
LD=$(TMS9900_DIR)/tms9900-ld
CC=$(TMS9900_DIR)/tms9900-gcc

CP=/usr/bin/cp
NAME=rick

# Path to elf2cart conversion utility
ELF2CART=/mnt/d/work/ti/Elf2BankedCart/elf2bankedcart

# Flags used during linking
#
# Locate the code (.text section) and the data (.data section)
# a linker script is necessary to define the output sections (that will replace the list in the Coleco CRT0)
# That will also let us set the start addresses. The ELF should in theory contain section names which will let
# out custom cartridge builder figure out where things go.
# https://ftp.gnu.org/old-gnu/Manuals/ld-2.9.1/html_node/ld_19.html
LDFLAGS_EA5=-M -T linker.ld

INCPATH=-I/mnt/d/work/libti99ALL -I../include/
LIBPATH=-L/mnt/d/work/libti99ALL/buildti
LIBS=-lti99

# Note: no-function-cse prevents gcc from preferring registers for bl calls, "-minline_rtl -fdump-rtl-expand" for rtl in asm and expand
CFLAGS=-Os -std=c99 -c -s --save-temp -DTI99 -fno-builtin -fno-function-cse
#-minline_rtl

.PHONY: all clean

# crt0 must be first!
OBJECT_LIST = crt0.o vdpint.o control.o dat_ents.o dat_ents2.o dat_maps1.o dat_maps2.o dat_maps3.o dat_maps4.o dat_picscTI.o dat_picspTI.o \
    dat_screens.o dat_tilescTI.o dat_tilespTI.o dat_spritesTI0.o dat_spritesTI1.o dat_spritesTI2.o dat_spritesTI3.o dat_spritesTI4.o \
    draw.o e_bomb.o e_bonus.o e_box.o e_bullet.o e_rick.o e_sbonus.o e_them.o ents.o env.o fb.o \
    game.o img.o maps.o scr_gameover.o scr_getname.o scr_imain.o scr_imap.o scr_pause.o soundbank.o \
    scroller.o sounds.o sprites.o sysarg.o sysevt.o syskbd.o system.o sysvid.o tiles.o \
    util.o xrick.o scratchpad.o CPlayerCommonHandEdit.o CPlayerTIHandEdit.o CPlayerTIHandlers.o

# List of all files needed in executable
PREREQUISITES=$(OBJECT_LIST)

all: $(PREREQUISITES)
	$(LD) $(OBJECT_LIST) $(LIBS) $(LIBPATH) $(LDFLAGS_EA5) -o $(NAME).ea5.elf > ea5.map
	$(ELF2CART) $(NAME).ea5.elf $(NAME)8.bin

clean:
	-rm *.o *.map *.lst *.sym *.ihx *.rom *.i *.s

# libti99 crt0 copied into folder
crt0.o: crt0_bankcart.assembly
	$(GAS) $(AFLAGS) crt0_bankcart.assembly -o crt0.o

# other assembly files
%.o: %.assembly
	$(GAS) $< -o $@

# Recipe to compile all C files
%.o: %.c
	$(CC) -c $< $(CFLAGS) $(INCPATH) -o $@
