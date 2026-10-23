# Bind the update's private pool references (offsets within overlay_029.c's
# .text, the update at +0x5C4) (-0.1f, -0.2f, FLT_MAX, 750.0f
# and 0.4f) to the retained overlay pool at data_rodata +0x0, rodata-relative
# +0x0, which the shipped %hi/%lo pairs encode. Only relocation symbol
# indices change; instructions and addends stay untouched.
0x664:.rodata:gOverlay29UpdatePoolReloc
0x670:.rodata:gOverlay29UpdatePoolReloc
0x680:.rodata:gOverlay29UpdatePoolReloc
0x684:.rodata:gOverlay29UpdatePoolReloc
0xb08:.rodata:gOverlay29UpdatePoolReloc
0xb60:.rodata:gOverlay29UpdatePoolReloc
0xb68:.rodata:gOverlay29UpdatePoolReloc
0xb6c:.rodata:gOverlay29UpdatePoolReloc
0xbc8:.rodata:gOverlay29UpdatePoolReloc
0xbcc:.rodata:gOverlay29UpdatePoolReloc
