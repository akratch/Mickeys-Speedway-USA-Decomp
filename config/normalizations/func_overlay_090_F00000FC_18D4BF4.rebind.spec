# Bind the compiler's private pool references (six float constants, the
# seven-entry state-switch table and the 0.003f constant) to the retained
# overlay pool at data_rodata +0x8, rodata-relative +0x8 after
# overlay90Initialize's two data words, which the shipped %hi/%lo pairs
# encode. Only relocation symbol indices change; instructions and addends
# stay untouched.
0x178:.rodata:gOverlay90StatePoolReloc
0x17c:.rodata:gOverlay90StatePoolReloc
0x1a8:.rodata:gOverlay90StatePoolReloc
0x1ac:.rodata:gOverlay90StatePoolReloc
0x1dc:.rodata:gOverlay90StatePoolReloc
0x1e0:.rodata:gOverlay90StatePoolReloc
0x1e4:.rodata:gOverlay90StatePoolReloc
0x1e8:.rodata:gOverlay90StatePoolReloc
0x1ec:.rodata:gOverlay90StatePoolReloc
0x1f0:.rodata:gOverlay90StatePoolReloc
0x1f4:.rodata:gOverlay90StatePoolReloc
0x1fc:.rodata:gOverlay90StatePoolReloc
0x21c:.rodata:gOverlay90StatePoolReloc
0x224:.rodata:gOverlay90StatePoolReloc
0x2f8:.rodata:gOverlay90StatePoolReloc
0x2fc:.rodata:gOverlay90StatePoolReloc
