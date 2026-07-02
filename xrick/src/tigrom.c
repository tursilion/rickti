/*
 * xrick/src/tigrom.c
 * GRAM load/save routines for high scores
 */

#include "tigrom.h"
#include "game.h"
#include <kscan.h>
#include <grom.h>

static int hasGrom = 0;
extern hscore_t game_hscores[8];

// return true if there's a high score ubergrom attached
int checkHighScores() {
    // just check the configuration bits. If bytes 0 and 1
    // are not inverted copies of each other, assume no ubergrom
    // config space is always mapped, so we can just go ahead and read
    GROM_SET_ADDRESS(UBERGROM_CHECK);
    unsigned char a = UBERGROM_RD;
    unsigned char b = UBERGROM_RD;
    // this is really important. If we do all this without casting, then (~b) is promoted
    // to an unsigned int (with the MSB set to 00). This forces a to be promoted as
    // well, and the inversion causes a failure to match because of the MSB.
    // It also works if you do the inversion separately ( ie: b = ~b; if (a != b) return 0; )
    // we can also use signed chars, which causes the promotions to sign extend and
    // accidentally give us the correct results.
    if (a != (unsigned char)(~b)) return 0;
    return 1;
}

// destroys the magic byte so high scores will be reset
void clearHighScores() {
    // unlock the eeprom
	GromWriteData(0xffff, 15, 0x55);
	GromWriteData(0xffff, 15, 0xaa);
	GromWriteData(0xffff, 15, 0x5a);

    // now write the data to corrupt the magic
    GromWriteData(UBERGROM_WRITE, 15, 0);

    // and relock the eeprom
	GromWriteData(0xffff, 15, 0);
}

// write the high scores out - we assume that the device was already checked for
// - it's safe for us to just do it, the UBERGROM will check for changes. IIRC.
void savegrom() {
    if (!checkHighScores()) {
        return;
    }

    // unlock the eeprom
	GromWriteData(0xffff, 15, 0x55);
	GromWriteData(0xffff, 15, 0xaa);
	GromWriteData(0xffff, 15, 0x5a);

    // write the magic
    GromWriteData(UBERGROM_WRITE, 15, 0xaa);
    GromWriteData(UBERGROM_WRITE+1, 15, 0x55);

    // now write the data
    unsigned char *pDat = (unsigned char*)game_hscores;
    for (int idx=UBERGROM_WRITE+2; idx<UBERGROM_WRITE+sizeof(game_hscores)+2; ++idx) {
        // A bit slower this way, setting the address each time
        GromWriteData(idx, 15, *(pDat++));
    }

    // and relock the eeprom
	GromWriteData(0xffff, 15, 0);
}

void loadgrom() {
    if (!checkHighScores()) {
        return;
    }
    // if joystick fire is held, then clear the high scores
    // this is only called once at startup
    kscanfast(1);
    if (KSCAN_KEY == JOY_FIRE) {
        clearHighScores();
        return;
    }

    // we think the device exists, so suck in the high scores
    // we don't bother assume the cache is valid - we re-read each time
    GROM_SET_ADDRESS(UBERGROM_WRITE);

    // check magic (autoincrement!)
    if (UBERGROM_RD != 0xaa) {
        return;
    }
    if (UBERGROM_RD != 0x55) {
        return;
    }

    // load the scores (autoincrement)
    unsigned char *pDat = (unsigned char*)game_hscores;
    for (int idx=0; idx<sizeof(game_hscores); ++idx) {
        *(pDat++) = UBERGROM_RD;
    }
}
