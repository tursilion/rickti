20260815

This is a port of xrick to the TI-99/4A.

Features 15 color F18A backgrounds with 8 color sprites (and 4 sprite palettes), so it's pretty colorful. Based mostly on the Atari ST version in the code.

There are two bugs that I'm aware of. I've had trouble figuring out exactly where to fix the one, and the other I'm not sure if it's an original bug, a port bug, or a compiler bug. One is that certain movable bricks appear a few pixels lower than they should. The other is that sometimes, particularly in level 2, enemies sometimes get stuck in the walls. Neither tends to impact the gameplay much, so don't be too worried if you see them.

The ROM is a 256k combined ROM - this was a tight fit! It will autodetect F18A at startup, and if not present will fall back to standard graphics (though sprites are all white in this mode). It also requires 32k and REQUIRES a joystick.

In game, '4' toggles audio, and 'P' toggles pause. Otherwise, use joystick 1 to move, up to jump, down to crawl. Rick has three tools used by holding fire then pressing a direction:

- Fire+Up - fire gun (bullets are limited and refreshed by crates)
- Fire+Down - lay dynamite (also limited, and back off before it explodes)
- Fire+Left/Right - poke with stick - good for some traps and waking some enemies. Briefly stuns other enemies, but this is not very useful.

In case you are not familiar with Rick Dangerous - expect to die A LOT. This is a memorization game for the most part, and a lot of traps give little to no warning.

A physical cartridge should be available from ArcadeShopper:
https://www.arcadeshopper.com/wp/store/#!/TI-99-4A/c/5051340

ti99iuc has provided improved graphics for the 9918A port.

The original notes are below.

# xrick
Remember Rick Dangerous?

Way before Lara Croft, back in the 1980's and early 1990's, Rick Dangerous was the Indiana Jones of computer games,
running away from rolling rocks, avoiding traps, from South America to a futuristic missile base via Egypt and the
Schwarzendumpf castle.

**xrick** is a clone of Rick Dangerous, produced by carefully reverse-engineering the PC and Atari versions of the
game, and re-coding in C. It has been ported to Windows, Linux, but also BeOs, Amiga, QNX, and all sorts
of gaming console.

You can read more about Rick Dangerous straight from his creator, [Simon Phipps](https://www.simonphipps.com/games/rickdangerous/),
and more about xrick at the original [xrick page](http://www.bigorno.net/xrick). The code for xrick was only available 
as Zip files on that page: the goal of this repository is to release it in a more convenient way.

So far, it contains:
* The "Dec 12th, 2002" release (#021212) which is the last version I published in 2002
* The "May, 2005" release (#050500) which was never released, and is a bit cleaner (?)
* Ported from SDL to SDL2
* With adjustments so it can build with [emscripten](https://emscripten.org/)

