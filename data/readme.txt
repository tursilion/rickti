NOTE: not all songs require forcechan 2 - only the ones with a noise channel. But it seems like I can afford the extra bytes, so it was easier to let bestpacker do all the work, and it's not selective.

D:\work\TI\xrick\data>bestpacker -maxsize 8152 -args "-sn -deepdive -forcechan 2" egypt.psg gameover.psg mbase.psg rick1c64.psg rick1victory.psg samerica.psg schwarz.psg bombshht.psg bonus.psg box.psg bullet.psg crawl.psg ent0.psg ent1.psg ent2.psg ent3.psg ent4.psg ent5.psg ent6.psg ent7.psg ent8.psg explode.psg jump.psg novawaa.psg pad.psg sbonus.psg sbonus2.psg stick.psg walk.psg

@rem Output size: 5124 bytes
vgmcomp2 -sn -deepdive -forcechan 2 egypt.psg walk.psg ent6.psg crawl.psg bombshht.psg stick.psg ent8.psg bullet.psg explode.psg ent0.psg box.psg sbonus.psg bonus.psg jump.psg sbonus2.psg pad.psg gameover.psg ent3.psg ent4.psg schwarz.psg ent1.psg ent5.psg rick1victory.psg novawaa.psg ent2.psg mbase.psg ent7.psg samerica.psg rick1c64.psg OutFile00.sbf


00 egypt.psg 
01 walk.psg 
02 ent6.psg 
03 crawl.psg 
04 bombshht.psg 
05 stick.psg 
06 ent8.psg 
07 bullet.psg 
08 explode.psg 
09 ent0.psg 
10 box.psg 
11 sbonus.psg 
12 bonus.psg 
13 jump.psg 
14 sbonus2.psg 
15 pad.psg 
16 gameover.psg 
17 ent3.psg 
18 ent4.psg 
19 schwarz.psg 
20 ent1.psg 
21 ent5.psg 
22 rick1victory.psg 
23 novawaa.psg 
24 ent2.psg 
25 mbase.psg 
26 ent7.psg 
27 samerica.psg 
28 rick1c64.psg

