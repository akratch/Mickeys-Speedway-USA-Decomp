# Overlay 46's compiler .rodata (the two captions, the sequence switch table
# and the four particle factors) is byte-identical to the retained overlay
# rodata from +0xC (the leading title string is reached by extern). Until
# phase 3 gives the module its rodata, bind every site to a base valued 0xC.
# Each row asserts offset, old symbol and new symbol; only relocation symbol
# indices change. Generated from the one-TU object.
0x144:.rodata:gOverlay46RodataBaseReloc
0x14c:.rodata:gOverlay46RodataBaseReloc
0x6a4:.rodata:gOverlay46RodataBaseReloc
0x6a8:.rodata:gOverlay46RodataBaseReloc
0x928:.rodata:gOverlay46RodataBaseReloc
0x92c:.rodata:gOverlay46RodataBaseReloc
0xa44:.rodata:gOverlay46RodataBaseReloc
0xa48:.rodata:gOverlay46RodataBaseReloc
0xca8:.rodata:gOverlay46RodataBaseReloc
0xcac:.rodata:gOverlay46RodataBaseReloc
0xe20:.rodata:gOverlay46RodataBaseReloc
0xe2c:.rodata:gOverlay46RodataBaseReloc
0xe64:.rodata:gOverlay46RodataBaseReloc
0xe70:.rodata:gOverlay46RodataBaseReloc
