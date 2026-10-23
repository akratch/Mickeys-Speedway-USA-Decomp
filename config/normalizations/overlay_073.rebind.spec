# Bind the compiler's private pool references (the five-entry mode-switch
# table and the float literals) to the retained overlay pool at data_rodata
# +0xDC, rodata-relative +0xC, which the shipped %hi/%lo pairs encode.
# Offsets are in the one-TU overlay_073.c object, where
# func_overlay_073_F0000190_18CAC50 begins at +0x190. Only relocation
# symbol indices change; instructions and addends stay untouched.
0x1e8:.rodata:gOverlay73UpdatePoolReloc
0x1f0:.rodata:gOverlay73UpdatePoolReloc
0x264:.rodata:gOverlay73UpdatePoolReloc
0x268:.rodata:gOverlay73UpdatePoolReloc
0x394:.rodata:gOverlay73UpdatePoolReloc
0x398:.rodata:gOverlay73UpdatePoolReloc
0x3e0:.rodata:gOverlay73UpdatePoolReloc
0x3e4:.rodata:gOverlay73UpdatePoolReloc
0x550:.rodata:gOverlay73UpdatePoolReloc
0x554:.rodata:gOverlay73UpdatePoolReloc
0x734:.rodata:gOverlay73UpdatePoolReloc
0x738:.rodata:gOverlay73UpdatePoolReloc
0x780:.rodata:gOverlay73UpdatePoolReloc
0x784:.rodata:gOverlay73UpdatePoolReloc
0x810:.rodata:gOverlay73UpdatePoolReloc
0x814:.rodata:gOverlay73UpdatePoolReloc
0x9b8:.rodata:gOverlay73UpdatePoolReloc
0x9d0:.rodata:gOverlay73UpdatePoolReloc
0xaac:.rodata:gOverlay73UpdatePoolReloc
0xab0:.rodata:gOverlay73UpdatePoolReloc
0xad0:.rodata:gOverlay73UpdatePoolReloc
0xad4:.rodata:gOverlay73UpdatePoolReloc
0xb0c:.rodata:gOverlay73UpdatePoolReloc
0xb10:.rodata:gOverlay73UpdatePoolReloc
0xbc4:.rodata:gOverlay73UpdatePoolReloc
0xbcc:.rodata:gOverlay73UpdatePoolReloc
0xbd0:.rodata:gOverlay73UpdatePoolReloc
0xbd4:.rodata:gOverlay73UpdatePoolReloc
0xd3c:.rodata:gOverlay73UpdatePoolReloc
0xd40:.rodata:gOverlay73UpdatePoolReloc
