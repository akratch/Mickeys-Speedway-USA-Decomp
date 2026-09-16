# Bind the compiler's private jump-table references (the mode switch on
# D_o058_5E94, thirteen entries) to the retained overlay table at
# data_rodata +0x3F4, rodata-relative +0x124, which the shipped text encodes.
# Only relocation symbol indices change; instructions stay untouched.
0x110:.rodata:gOverlay58ModeJumpTableReloc
0x118:.rodata:gOverlay58ModeJumpTableReloc
