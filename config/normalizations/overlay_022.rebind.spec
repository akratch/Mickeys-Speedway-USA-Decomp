# Bind the update's private pool references (14.4f, 0.8f and 0.03f; offsets
# are within overlay_022.c's .text, the update starting at +0x2B0) to the
# retained overlay pool at rodata-relative +0x4, which the shipped %hi/%lo
# pairs encode (overlay_022.c's .data owns those bytes). Only
# relocation symbol indices change; instructions and addends stay untouched.
0x2cc:.rodata:gOverlay22UpdatePoolReloc
0x2d0:.rodata:gOverlay22UpdatePoolReloc
0x658:.rodata:gOverlay22UpdatePoolReloc
0x678:.rodata:gOverlay22UpdatePoolReloc
0x930:.rodata:gOverlay22UpdatePoolReloc
0x958:.rodata:gOverlay22UpdatePoolReloc
