# Bind the compiler's private jump-table references (the 25-case selector
# switch) to the retained overlay table at data_rodata +0x5CFC,
# rodata-relative +0xEAC, which the shipped text encodes.
# Only relocation symbol indices change; instructions stay untouched.
0x78:.rodata:gO101TailC6E8SwitchTableReloc
0x80:.rodata:gO101TailC6E8SwitchTableReloc
