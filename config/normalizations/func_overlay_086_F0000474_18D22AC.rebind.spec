# Bind the compiler's private pool references (two float constants at +0x0
# and +0x4, the five-entry state-switch table at +0x8) to the retained overlay
# pool at data_rodata +0x80, rodata-relative +0x0, which the shipped %hi/%lo
# pairs encode.  Only relocation symbol indices change; instructions and
# addends stay untouched.
0x1f8:.rodata:gOverlay86StatePoolReloc
0x1fc:.rodata:gOverlay86StatePoolReloc
0x208:.rodata:gOverlay86StatePoolReloc
0x210:.rodata:gOverlay86StatePoolReloc
0x224:.rodata:gOverlay86StatePoolReloc
0x22c:.rodata:gOverlay86StatePoolReloc
