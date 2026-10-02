# Bind the compiler's private pool references (-0.1f, -0.2f, FLT_MAX, 750.0f
# and 0.4f) to the retained overlay pool at data_rodata +0x0, rodata-relative
# +0x0, which the shipped %hi/%lo pairs encode. Only relocation symbol
# indices change; instructions and addends stay untouched.
0xa0:.rodata:gOverlay29UpdatePoolReloc
0xac:.rodata:gOverlay29UpdatePoolReloc
0xbc:.rodata:gOverlay29UpdatePoolReloc
0xc0:.rodata:gOverlay29UpdatePoolReloc
0x544:.rodata:gOverlay29UpdatePoolReloc
0x59c:.rodata:gOverlay29UpdatePoolReloc
0x5a4:.rodata:gOverlay29UpdatePoolReloc
0x5a8:.rodata:gOverlay29UpdatePoolReloc
0x604:.rodata:gOverlay29UpdatePoolReloc
0x608:.rodata:gOverlay29UpdatePoolReloc
