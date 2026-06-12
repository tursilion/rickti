/*
 * xrick/include/sounds.h
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

#ifndef _SOUNDS_H
#define _SOUNDS_H

#define RICK1_SND 28
#define SAMERICA_SND 27
#define EGYPT_SND 0
#define SCHWARZ_SND 19
#define MBASE_SND 25
#define GAMEOVER_SND 16
#define RICK1VICTORY_SND 22

// sound effects

#define WALK_SND 1
#define CRAWL_SND 3
#define JUMP_SND 13
#define STICK_SND 5
#define BULLET_SND 7
#define BOMBSHHT_SND 4
#define EXPLODE_SND 8
#define DIE_SND 23

#define PAD_SND 15
#define BOX_SND 10
#define BONUS_SND 12
#define SBONUS_SND 11
#define SBONUS2_SND 14

#define ENT0_SND 9
#define ENT1_SND 20
#define ENT2_SND 24
#define ENT3_SND 17
#define ENT4_SND 18
#define ENT5_SND 21
#define ENT6_SND 2
#define ENT7_SND 26
#define ENT8_SND 6

#ifdef ENABLE_SOUND

#include "TISNPlay.h"
#include <sound.h>

// Indexes into the SBF sound bank
// Sounds are in BANK12 right now. About 5k.
// music

// start a sound effect
extern void sounds_play(U16 idx);

// stop sounds and mute
// note: no pause needed, we don't play
// music during the time 'pause' is valid, so we'll just let
// whatever sound effect finish playing out.
extern void sounds_stop(void);

// check if sound is playing
#define sounds_playing() (isSNPlaying)

extern void sounds_toggleMute();
extern U16 isMuted;

extern const U16 WAV_ENTITY[9];

#endif /* ENABLE_SOUND */

#endif /* _SOUNDS_H */

/* eof */


