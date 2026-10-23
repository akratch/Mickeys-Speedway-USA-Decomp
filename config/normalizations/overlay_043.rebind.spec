# Overlay 43's compiler .rodata (the image scale and the motion scale) is
# byte-identical to the retained overlay rodata from +0x20, after the two
# "clone.c" trace strings the renderer reaches by placeholder. Until phase 3
# gives the module its rodata, bind both pairs to a base valued 0x20. Each row
# asserts offset, old symbol and new symbol; only relocation symbol indices
# change. Generated from the one-TU object.
0xb24:.rodata:gOverlay43RodataBaseReloc
0xb28:.rodata:gOverlay43RodataBaseReloc
0x1134:.rodata:gOverlay43RodataBaseReloc
0x1138:.rodata:gOverlay43RodataBaseReloc
