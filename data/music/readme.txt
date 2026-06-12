Midis from https://rickdangerous.co.uk/sounds.htm
by The Hand of Jimmer

The MIDs might be a bit better if they convert okay. Mapping to the game songs:

tune0 - samerica
tune1 - egypt
tune2 - schwarz
tune3 - mbase
tune4 - rick1victory
tune5 - rick1

Convert mids at 1.2 speed to convert from PAL to NTSC playback speed (my preference).
(note: some tunes were dropping sections unless I did 1.21 instead of 1.2)

To convert a bassy tone channel into pure noise, instead of bass2noise:
- rename the channel to a _noi type
- scalepitch by 0.06666666667 (divide by 15)
- use forcenoisetype to change it to periodic type
- verify no notes drop below 1!

Tunes so far all have only 2-3 channels.

