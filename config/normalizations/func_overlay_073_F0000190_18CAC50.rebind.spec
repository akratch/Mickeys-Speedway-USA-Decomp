# Bind the compiler's private pool references (the five-entry mode-switch
# table and the float literals) to the retained overlay pool at data_rodata
# +0xDC, rodata-relative +0xC, which the shipped %hi/%lo pairs encode. Only
# relocation symbol indices change; instructions and addends stay untouched.
0x58:.rodata:gOverlay73UpdatePoolReloc
0x60:.rodata:gOverlay73UpdatePoolReloc
0xd4:.rodata:gOverlay73UpdatePoolReloc
0xd8:.rodata:gOverlay73UpdatePoolReloc
0x204:.rodata:gOverlay73UpdatePoolReloc
0x208:.rodata:gOverlay73UpdatePoolReloc
0x250:.rodata:gOverlay73UpdatePoolReloc
0x254:.rodata:gOverlay73UpdatePoolReloc
0x3c0:.rodata:gOverlay73UpdatePoolReloc
0x3c4:.rodata:gOverlay73UpdatePoolReloc
0x5a4:.rodata:gOverlay73UpdatePoolReloc
0x5a8:.rodata:gOverlay73UpdatePoolReloc
0x5f0:.rodata:gOverlay73UpdatePoolReloc
0x5f4:.rodata:gOverlay73UpdatePoolReloc
0x680:.rodata:gOverlay73UpdatePoolReloc
0x684:.rodata:gOverlay73UpdatePoolReloc
0x828:.rodata:gOverlay73UpdatePoolReloc
0x840:.rodata:gOverlay73UpdatePoolReloc
0x91c:.rodata:gOverlay73UpdatePoolReloc
0x920:.rodata:gOverlay73UpdatePoolReloc
0x940:.rodata:gOverlay73UpdatePoolReloc
0x944:.rodata:gOverlay73UpdatePoolReloc
0x97c:.rodata:gOverlay73UpdatePoolReloc
0x980:.rodata:gOverlay73UpdatePoolReloc
0xa34:.rodata:gOverlay73UpdatePoolReloc
0xa3c:.rodata:gOverlay73UpdatePoolReloc
0xa40:.rodata:gOverlay73UpdatePoolReloc
0xa44:.rodata:gOverlay73UpdatePoolReloc
0xbac:.rodata:gOverlay73UpdatePoolReloc
0xbb0:.rodata:gOverlay73UpdatePoolReloc
