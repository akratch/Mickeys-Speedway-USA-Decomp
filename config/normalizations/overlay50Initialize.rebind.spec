# func_overlay_050_F0000000_1896970 defines overlay 50's .data and .bss at the
# offsets its runtime records address, so the compiler addresses them through
# this object's own section symbols.  The retained overlay image owns the
# bytes: this object's copies are removed after the rebind, and every site is
# bound to a zero-valued base placeholder so the shipped words stay
# section-relative.  The 0.7f pool's pair is bound to a base valued at the
# literal's retained rodata offset, 0x4.  Each row asserts offset, old symbol
# and new symbol; only relocation symbol indices change.
0xec:.rodata:gOverlay50InitScaleReloc
0xf8:.rodata:gOverlay50InitScaleReloc
0x10:.data:gOverlay50InitDataBaseReloc
0x1c:.data:gOverlay50InitDataBaseReloc
0x20:.data:gOverlay50InitDataBaseReloc
0x28:.data:gOverlay50InitDataBaseReloc
0x38:.bss:gOverlay50InitBssBaseReloc
0x3c:.bss:gOverlay50InitBssBaseReloc
0x48:.data:gOverlay50InitDataBaseReloc
0x50:.data:gOverlay50InitDataBaseReloc
0x54:.data:gOverlay50InitDataBaseReloc
0x5c:.data:gOverlay50InitDataBaseReloc
0x60:.data:gOverlay50InitDataBaseReloc
0x68:.data:gOverlay50InitDataBaseReloc
0x6c:.data:gOverlay50InitDataBaseReloc
0x74:.data:gOverlay50InitDataBaseReloc
0x78:.data:gOverlay50InitDataBaseReloc
0x80:.data:gOverlay50InitDataBaseReloc
0x84:.data:gOverlay50InitDataBaseReloc
0x8c:.data:gOverlay50InitDataBaseReloc
0x90:.data:gOverlay50InitDataBaseReloc
0x98:.data:gOverlay50InitDataBaseReloc
0x9c:.data:gOverlay50InitDataBaseReloc
0xa4:.data:gOverlay50InitDataBaseReloc
0xa8:.data:gOverlay50InitDataBaseReloc
0xb0:.data:gOverlay50InitDataBaseReloc
0xbc:.bss:gOverlay50InitBssBaseReloc
0xc4:.bss:gOverlay50InitBssBaseReloc
0xd0:.bss:gOverlay50InitBssBaseReloc
0xd8:.bss:gOverlay50InitBssBaseReloc
0x100:.bss:gOverlay50InitBssBaseReloc
0x110:.bss:gOverlay50InitBssBaseReloc
0x104:.bss:gOverlay50InitBssBaseReloc
0x10c:.bss:gOverlay50InitBssBaseReloc
0x138:.data:gOverlay50InitDataBaseReloc
0x140:.data:gOverlay50InitDataBaseReloc
0x14c:.bss:gOverlay50InitBssBaseReloc
0x154:.bss:gOverlay50InitBssBaseReloc
0x158:.bss:gOverlay50InitBssBaseReloc
0x160:.bss:gOverlay50InitBssBaseReloc
0x164:.data:gOverlay50InitDataBaseReloc
0x178:.data:gOverlay50InitDataBaseReloc
0x17c:.bss:gOverlay50InitBssBaseReloc
0x1a0:.bss:gOverlay50InitBssBaseReloc
0x168:.data:gOverlay50InitDataBaseReloc
0x174:.data:gOverlay50InitDataBaseReloc
0x19c:.bss:gOverlay50InitBssBaseReloc
0x16c:.data:gOverlay50InitDataBaseReloc
0x170:.data:gOverlay50InitDataBaseReloc
0x198:.bss:gOverlay50InitBssBaseReloc
0x184:.bss:gOverlay50InitBssBaseReloc
0x190:.bss:gOverlay50InitBssBaseReloc
0x180:.data:gOverlay50InitDataBaseReloc
0x194:.data:gOverlay50InitDataBaseReloc
0x188:.data:gOverlay50InitDataBaseReloc
0x18c:.data:gOverlay50InitDataBaseReloc
0x238:.data:gOverlay50InitDataBaseReloc
0x23c:.data:gOverlay50InitDataBaseReloc
0x2a4:.bss:gOverlay50InitBssBaseReloc
0x2a8:.bss:gOverlay50InitBssBaseReloc
0x264:.bss:gOverlay50InitBssBaseReloc
0x268:.bss:gOverlay50InitBssBaseReloc
0x2cc:.bss:gOverlay50InitBssBaseReloc
0x2d0:.bss:gOverlay50InitBssBaseReloc
0x2d4:.bss:gOverlay50InitBssBaseReloc
0x2d8:.bss:gOverlay50InitBssBaseReloc
