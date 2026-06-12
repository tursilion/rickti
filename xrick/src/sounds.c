/*
 * xrick/src/sounds.c
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
#include "sounds.h"

#ifdef ENABLE_SOUND

#define BIN2INC_HEADER_ONLY
#include "soundbank.c"

// whether we are muted
U16 isMuted = 0;

// entity to musical index
const U16 WAV_ENTITY[9] = {
    ENT0_SND, ENT1_SND, ENT2_SND, ENT3_SND, ENT4_SND, ENT5_SND, ENT6_SND, ENT7_SND, ENT8_SND
};

// a simple priority system - higher number is more important
// the crazy order is because that's how the sound bank compressed best
static const U16 SND_PRIORITY[29] = {
/* EGYPT_SND 0          */ 5,
/* WALK_SND 1           */ 0,
/* ENT6_SND 2           */ 3,
/* CRAWL_SND 3          */ 0,
/* BOMBSHHT_SND 4       */ 4,
/* STICK_SND 5          */ 1,
/* ENT8_SND 6           */ 3,
/* BULLET_SND 7         */ 2,
/* EXPLODE_SND 8        */ 4,
/* ENT0_SND 9           */ 3,
/* BOX_SND 10           */ 4,
/* SBONUS_SND 11        */ 4,
/* BONUS_SND 12         */ 4,
/* JUMP_SND 13          */ 2,
/* SBONUS2_SND 14       */ 4,
/* PAD_SND 15           */ 2,
/* GAMEOVER_SND 16      */ 5,
/* ENT3_SND 17          */ 3,
/* ENT4_SND 18          */ 3,
/* SCHWARZ_SND 19       */ 5,
/* ENT1_SND 20          */ 3,
/* ENT5_SND 21          */ 3,
/* RICK1VICTORY_SND 22  */ 5,
/* DIE_SND 23           */ 4,
/* ENT2_SND 24          */ 3,
/* MBASE_SND 25         */ 5,
/* ENT7_SND 26          */ 3,
/* SAMERICA_SND 27      */ 5,
/* RICK1_SND 28         */ 5
};
U16 currentPri = 0;

/*
 * sounds_play
 *
 * play a music or a sound effect (we run them all the same here)
 * Note - we don't handle looping here. The title is the only song
 * in the game that loops, so we just handle looping there.
 * 
 */
void sounds_play(U16 idx)
{
    unsigned int nOldBank = nBank;
    unsigned int nPri = SND_PRIORITY[idx];

    if (isMuted) return;

    if (sounds_playing()) {
        if (nPri < currentPri) {
            return;
        }
        sounds_stop();
    }
    currentPri = nPri;

    SWITCH_IN_BANK12;
    StartSong(SOUNDBANK, idx);
    SWITCH_IN_BANK(nOldBank);
}

void sounds_stop(void) {
    StopSong();
    MUTE_SOUND();
    currentPri = 0;
}

void sounds_toggleMute(void) {
    if (isMuted) {
        isMuted = 0;
        // next sound will start when it starts
    } else {
        sounds_stop();
        isMuted = 1;
    }
}

#endif /* ENABLE_SOUND */

 /* eof */
