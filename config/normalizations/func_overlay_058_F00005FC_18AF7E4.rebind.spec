# Bind the state update's private pool references (the seven-entry switch
# table on D_30 and the 0.02f literal that follows it) to the retained overlay
# bytes at data_rodata +0x3D4, rodata-relative +0x104 and +0x120, which the
# shipped text encodes. Only relocation symbol indices change; instructions
# and compiler addends stay untouched.
0x58:.rodata:gOverlay58StatePoolReloc
0x60:.rodata:gOverlay58StatePoolReloc
0x9f8:.rodata:gOverlay58StatePoolReloc
0x9fc:.rodata:gOverlay58StatePoolReloc
