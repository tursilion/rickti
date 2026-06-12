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

    if (isMuted) return;

    if (sounds_playing()) {
        StopSong();
    }
    SWITCH_IN_BANK12;
    StartSong(SOUNDBANK, idx);
    SWITCH_IN_BANK(nOldBank);
}

void sounds_stop(void) {
    StopSong();
    MUTE_SOUND();
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
