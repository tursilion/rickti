/*
 * xrick/src/tigrom.h
 */

#define UBERGROM_RD *((volatile unsigned char*)0x983c)
#define UBERGROM_WD *((volatile unsigned char*)0x9C3c)
#define UBERGROM_CHECK 0xf800
#define UBERGROM_WRITE 0xfa00
#define UBERGROM_UNLOCK 0xffff

void loadgrom();
void savegrom();
