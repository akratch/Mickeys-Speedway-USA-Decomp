# Bind the overlay 26 update's ten literal-pool references (a hi/lo pair per
# float literal; offsets within overlay_026.c's .text, the update at +0x1A0) to the retained Overlay 26 constants at rodata-relative 0x0
# through 0x10.
0x244:.rodata:gOverlay26UpdatePoolReloc
0x248:.rodata:gOverlay26UpdatePoolReloc
0x250:.rodata:gOverlay26UpdatePoolReloc
0x258:.rodata:gOverlay26UpdatePoolReloc
0x7c0:.rodata:gOverlay26UpdatePoolReloc
0x830:.rodata:gOverlay26UpdatePoolReloc
0xa94:.rodata:gOverlay26UpdatePoolReloc
0xa98:.rodata:gOverlay26UpdatePoolReloc
0xad0:.rodata:gOverlay26UpdatePoolReloc
0xad4:.rodata:gOverlay26UpdatePoolReloc
