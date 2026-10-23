# Overlay 45 owns its .data (the display-list setup, the allocator strings
# and the three motion constants) but the shipped words that address it are
# section-relative, and the glyph renderer's setup-list reference names the
# defined object: bind it to a zero-valued base. The path updater's float
# pool duplicates the three motion constants at data +0x24: bind its pairs to
# a base valued 0x24. Each row asserts offset, old symbol and new symbol;
# only relocation symbol indices change. Generated from the one-TU object.
0x8c0:.rodata:gOverlay45RetainedMotionConstants
0x8c4:.rodata:gOverlay45RetainedMotionConstants
0x8c8:.rodata:gOverlay45RetainedMotionConstants
0x8cc:.rodata:gOverlay45RetainedMotionConstants
0x8d0:.rodata:gOverlay45RetainedMotionConstants
0x8d8:.rodata:gOverlay45RetainedMotionConstants
0x1220:gOverlay45InitializedData:gOverlay45DataBaseReloc
0x122c:gOverlay45InitializedData:gOverlay45DataBaseReloc
