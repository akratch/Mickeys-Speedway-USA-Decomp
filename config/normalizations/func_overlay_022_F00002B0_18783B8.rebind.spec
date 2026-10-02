# Bind the compiler's private pool references (14.4f, 0.8f and 0.03f) to the
# retained overlay pool at rodata-relative +0x4, which the shipped %hi/%lo
# pairs encode (overlay22RemoveObject's .data owns those bytes). Only
# relocation symbol indices change; instructions and addends stay untouched.
0x1c:.rodata:gOverlay22UpdatePoolReloc
0x20:.rodata:gOverlay22UpdatePoolReloc
0x3a8:.rodata:gOverlay22UpdatePoolReloc
0x3c8:.rodata:gOverlay22UpdatePoolReloc
0x680:.rodata:gOverlay22UpdatePoolReloc
0x6a8:.rodata:gOverlay22UpdatePoolReloc
