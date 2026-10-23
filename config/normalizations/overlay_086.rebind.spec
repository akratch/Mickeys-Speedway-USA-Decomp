# Bind the compiler's private pool references (two float constants at +0x0
# and +0x4, the five-entry state-switch table at +0x8) to the retained overlay
# pool at data_rodata +0x80, rodata-relative +0x0, which the shipped %hi/%lo
# pairs encode. Offsets are in the one-TU overlay_086.c object, where
# func_overlay_086_F0000474_18D22AC begins at +0x474. Only relocation symbol
# indices change; instructions and addends stay untouched.
0x66c:.rodata:gOverlay86StatePoolReloc
0x670:.rodata:gOverlay86StatePoolReloc
0x67c:.rodata:gOverlay86StatePoolReloc
0x684:.rodata:gOverlay86StatePoolReloc
0x698:.rodata:gOverlay86StatePoolReloc
0x6a0:.rodata:gOverlay86StatePoolReloc
