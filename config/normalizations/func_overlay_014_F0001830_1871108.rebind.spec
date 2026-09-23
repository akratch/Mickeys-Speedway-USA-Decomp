# Bind the compiler's private jump-table references (the seven-case command
# switch) to the retained overlay table at data_rodata +0x174, rodata-relative
# +0x54, which the shipped text encodes.
# Only relocation symbol indices change; instructions stay untouched.
0x114:.rodata:gOverlay14DrawJumpTableReloc
0x11C:.rodata:gOverlay14DrawJumpTableReloc
