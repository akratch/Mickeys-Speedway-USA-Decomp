#include "PR/ultratypes.h"

/* Tier B: each leading label pointer selects a distinct NUL-terminated text
 * buffer used by the presentation/font path. The renderer temporarily edits
 * and restores bytes, so these are writable objects. Tier C: their spelling
 * follows actual byte-string use. IDO supplies four-byte object alignment;
 * no extra array capacity or encoded payload is introduced. */

/* Initialized rodata +0x0. */
u8 o101Label000[] = "Mickey Visits Pluto";

/* Initialized rodata +0x14. */
u8 o101Label001[] = "Hi ya, Pluto!";

/* Initialized rodata +0x24. */
u8 o101Label002[] = "Look what I have for you here...";

/* Initialized rodata +0x48. */
u8 o101Label003[] = "Mickey Finds a Note";

/* Initialized rodata +0x5C. */
u8 o101Label004[] = "Pluto? Hey, what's this?";

/* Initialized rodata +0x78. */
u8 o101Label005[] = "The Weasel's Note";

/* Initialized rodata +0x8C. */
u8 o101Label006[] = "Mickey";

/* Initialized rodata +0x94. */
u8 o101Label007[] = "Hey, everybody, the Weasels took Pluto.";

/* Initialized rodata +0xBC. */
u8 o101Label008[] = "Meet me at Ludwig's shop. We have to get";

/* Initialized rodata +0xE8. */
u8 o101Label009[] = "race cars and go after him.";

/* Initialized rodata +0x104. */
u8 o101Label010[] = "Daisy";

/* Initialized rodata +0x10C. */
u8 o101Label011[] = "That's awful!";

/* Initialized rodata +0x11C. */
u8 o101Label012[] = "Donald";

/* Initialized rodata +0x124. */
u8 o101Label013[] = "Wait for me.";

/* Initialized rodata +0x134. */
u8 o101Label014[] = "Minnie";

/* Initialized rodata +0x13C. */
u8 o101Label015[] = "Oh, My!";

/* Initialized rodata +0x144. */
u8 o101Label016[] = "Pete";

/* Initialized rodata +0x14C. */
u8 o101Label017[] = "Weasels, ha-ha!";

/* Initialized rodata +0x15C. */
u8 o101Label018[] = "Goofy";

/* Initialized rodata +0x164. */
u8 o101Label019[] = "Don't worry, Mickey.";

/* Initialized rodata +0x17C. */
u8 o101Label020[] = "We'll get him back.";

/* Initialized rodata +0x190. */
u8 o101Label021[] = "Ludwig Von Drake";

/* Initialized rodata +0x1A4. */
u8 o101Label022[] = "Now remember, everybody, I'll be";

/* Initialized rodata +0x1C8. */
u8 o101Label023[] = "with you to give you extra power";

/* Initialized rodata +0x1EC. */
u8 o101Label024[] = "for your cars.";

/* Initialized rodata +0x1FC. */
u8 o101Label025[] = "GOOD LUCK!";

/* Initialized rodata +0x208. */
u8 o101Label026[] = "Engine";

/* Initialized rodata +0x210. */
u8 o101Label027[] = "Ludwig Von Drake";

/* Initialized rodata +0x224. */
u8 o101Label028[] = "WELL DONE!";

/* Initialized rodata +0x230. */
u8 o101Label029[] = "I have filled you up with gas";

/* Initialized rodata +0x250. */
u8 o101Label030[] = "and changed your engine oil.";

/* Initialized rodata +0x270. */
u8 o101Label031[] = "Keep up the good work!";

/* Initialized rodata +0x288. */
u8 o101Label032[] = "Try for the Third Trophy";

/* Initialized rodata +0x2A4. */
u8 o101Label033[] = "Ludwig Von Drake";

/* Initialized rodata +0x2B8. */
u8 o101Label034[] = "YOU'RE DOING GREAT!";

/* Initialized rodata +0x2CC. */
u8 o101Label035[] = "Keep this up and you'll catch";

/* Initialized rodata +0x2EC. */
u8 o101Label036[] = "those crafty Weasels in no time.";

/* Initialized rodata +0x310. */
u8 o101Label037[] = "Now try the other Race Series!";

/* Initialized rodata +0x330. */
u8 o101Label038[] = "Engine";

/* Initialized rodata +0x338. */
u8 o101Label039[] = "Ludwig Von Drake";

/* Initialized rodata +0x34C. */
u8 o101Label040[] = "EXCELLENT RACING!";

/* Initialized rodata +0x360. */
u8 o101Label041[] = "You've forced the Weasels to";

/* Initialized rodata +0x380. */
u8 o101Label042[] = "flee farther across the country.";

/* Initialized rodata +0x3A4. */
u8 o101Label043[] = "Now try Victory Vehicles!";

/* Initialized rodata +0x3C0. */
u8 o101Label044[] = "Engine";

/* Initialized rodata +0x3C8. */
u8 o101Label045[] = "Ludwig Von Drake";

/* Initialized rodata +0x3DC. */
u8 o101Label046[] = "OH, HOW WONDERFUL!";

/* Initialized rodata +0x3F0. */
u8 o101Label047[] = "You're almost there. One more";

/* Initialized rodata +0x410. */
u8 o101Label048[] = "victory and Pluto will be saved.";

/* Initialized rodata +0x434. */
u8 o101Label049[] = "Bring those Weasels to Justice!";

/* Initialized rodata +0x454. */
u8 o101Label050[] = "Pluto is Reunited With His Friends";

/* Initialized rodata +0x478. */
u8 o101Label051[] = "We're so glad to see you! We had to";

/* Initialized rodata +0x49C. */
u8 o101Label052[] = "race awful fast to catch up with you.";

/* Initialized rodata +0x4C4. */
u8 o101Label053[] = "Pluto's Collar";

/* Initialized rodata +0x4D4. */
u8 o101Label054[] = "Mickey Speaks";

/* Initialized rodata +0x4E4. */
u8 o101Label055[] = "I don't know why those Weasels";

/* Initialized rodata +0x504. */
u8 o101Label056[] = "wanted your collar, Pluto.";

/* Initialized rodata +0x520. */
u8 o101Label057[] = "The Weasels Can't Hear the Story";

/* Initialized rodata +0x544. */
u8 o101Label058[] = "Minnie just made it with the glass";

/* Initialized rodata +0x568. */
u8 o101Label059[] = "beads from one of her old necklaces!";

/* Initialized rodata +0x590. */
u8 o101Label060[] = "Everyone Heads for Home";

/* Initialized rodata +0x5A8. */
u8 o101Label061[] = "Oh well. Let's go home, boy.";

/* Initialized rodata +0x5C8. */
u8 o101Label062[] = "Stolen Car Parts";

/* Initialized rodata +0x5DC. */
u8 o101Label063[] = "Excellent! You've found a stolen car part.";

/* Initialized rodata +0x608. */
u8 o101Label064[] = "The Weasels took four of these and hid them.";

/* Initialized rodata +0x638. */
u8 o101Label065[] = "Find them all to help you get Pluto back!";

/* Initialized rodata +0x664. */
u8 o101Label066[] = "My word, another of my stolen car parts.";

/* Initialized rodata +0x690. */
u8 o101Label067[] = "This is super. If you find just two more,";

/* Initialized rodata +0x6BC. */
u8 o101Label068[] = "then I can help you reach the final races!";

/* Initialized rodata +0x6E8. */
u8 o101Label069[] = "This is beautiful. You're doing a great job!";

/* Initialized rodata +0x718. */
u8 o101Label070[] = "One more stolen part - then I got some fixin'";

/* Initialized rodata +0x748. */
u8 o101Label071[] = "up to do so you can have some more racing!";

/* Initialized rodata +0x774. */
u8 o101Label072[] = "Marvelous! You've found all the parts.";

/* Initialized rodata +0x79C. */
u8 o101Label073[] = "I'll now use these to service your car";

/* Initialized rodata +0x7C4. */
u8 o101Label074[] = "so you can race the Frantic Finale races!";

/* Initialized rodata +0x7F0. */
u8 o101Label075[] = "Huey Joins the Race";

/* Initialized rodata +0x804. */
u8 o101Label076[] = "Hey, can I come with you?";

/* Initialized rodata +0x820. */
u8 o101Label077[] = "Louie Joins the Race";

/* Initialized rodata +0x838. */
u8 o101Label078[] = "Wait for me, you guys.";

/* Initialized rodata +0x850. */
u8 o101Label079[] = "Dewey Joins the Race";

/* Initialized rodata +0x868. */
u8 o101Label080[] = "Hey, I want to race too!";

/* Initialized rodata +0x884. */
u8 o101Label081[] = "Ludwig Joins the Race";

/* Initialized rodata +0x89C. */
u8 o101Label082[] = "Time to test my WonderCar!";

/* Initialized rodata +0x8B8. */
u8 o101Label083[] = "Electronic Mail";

/* Initialized rodata +0x8C8. */
u8 o101Label084[] = "It looks like you have received";

/* Initialized rodata +0x8E8. */
u8 o101Label085[] = "a postcard from the Weasels!";

/* Initialized rodata +0x908. */
u8 o101Label086[] = "Welcome to Time Trial";

/* Initialized rodata +0x920. */
u8 o101Label087[] = "HELLO THERE!";

/* Initialized rodata +0x930. */
u8 o101Label088[] = "You can use the Time Trial to practice";

/* Initialized rodata +0x958. */
u8 o101Label089[] = "the tracks and record your fastest times.";

/* Initialized rodata +0x984. */
u8 o101Label090[] = "Also, for each track there are special";

/* Initialized rodata +0x9AC. */
u8 o101Label091[] = "challenge times. If you beat the fastest";

/* Initialized rodata +0x9D8. */
u8 o101Label092[] = "challenge time on a track, you will";

/* Initialized rodata +0x9FC. */
u8 o101Label093[] = "receive a Gold Challenge Token.";

/* Initialized rodata +0xA1C. */
u8 o101Label094[] = "To help you beat these times, set the";

/* Initialized rodata +0xA44. */
u8 o101Label095[] = "Staff Ghosts setting on the Options";

/* Initialized rodata +0xA68. */
u8 o101Label096[] = "page to ON.";

/* Initialized rodata +0xA74. */
u8 o101Label097[] = "Secret Feature Opened";

/* Initialized rodata +0xA8C. */
u8 o101Label098[] = "NICE WORK!";

/* Initialized rodata +0xA98. */
u8 o101Label099[] = "You have unlocked a secret feature.";

/* Initialized rodata +0xABC. */
u8 o101Label100[] = "To find out what it is, go to the";

/* Initialized rodata +0xAE0. */
u8 o101Label101[] = "Options page and select 'Cheats'.";

/* Initialized rodata +0xB04. */
u8 o101Label102[] = "Trophy";

/* Initialized rodata +0xB0C. */
u8 o101Label103[] = "CONGRATULATIONS!";

/* Initialized rodata +0xB20. */
u8 o101Label104[] = "You've won a Race Series!";

/* Initialized rodata +0xB3C. */
u8 o101Label105[] = "Win them all at this difficulty,";

/* Initialized rodata +0xB60. */
u8 o101Label106[] = "then try the harder levels. The";

/* Initialized rodata +0xB80. */
u8 o101Label107[] = "Weasels await you on Professional.";

/* Initialized rodata +0xBA4. */
u8 o101Label108[] = "WOW, YOU'RE A GREAT DRIVER!";

/* Initialized rodata +0xBC0. */
u8 o101Label109[] = "Now try the mirrored races, if";

/* Initialized rodata +0xBE0. */
u8 o101Label110[] = "you feel like a new challenge.";

/* Initialized rodata +0xC00. */
u8 o101Label111[] = "Who knows, you may learn some";
