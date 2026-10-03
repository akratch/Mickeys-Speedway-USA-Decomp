# func_overlay_009_F0000000_1866678 reaches overlay 9's parameter block at
# .data +0x2D0 through this object's own .data symbol, because as1 shares a
# high half between two of its stores only for a symbol the TU defines.  The
# retained overlay image owns the bytes: the object's .data is removed after
# the rebind, and every site is bound to a zero-valued base placeholder so
# the shipped words stay section-relative.  Each row asserts offset, old
# symbol and new symbol; only relocation symbol indices change.
0x1a8:.data:gOverlay9DataBaseReloc
0x1ac:.data:gOverlay9DataBaseReloc
0x1bc:.data:gOverlay9DataBaseReloc
0x1c0:.data:gOverlay9DataBaseReloc
0x1f8:.data:gOverlay9DataBaseReloc
0x1fc:.data:gOverlay9DataBaseReloc
0x20c:.data:gOverlay9DataBaseReloc
0x210:.data:gOverlay9DataBaseReloc
0x238:.data:gOverlay9DataBaseReloc
0x240:.data:gOverlay9DataBaseReloc
0x22c:.data:gOverlay9DataBaseReloc
0x230:.data:gOverlay9DataBaseReloc
0x248:.data:gOverlay9DataBaseReloc
0x274:.data:gOverlay9DataBaseReloc
0x268:.data:gOverlay9DataBaseReloc
0x290:.data:gOverlay9DataBaseReloc
0x27c:.data:gOverlay9DataBaseReloc
0x2a8:.data:gOverlay9DataBaseReloc
0x29c:.data:gOverlay9DataBaseReloc
0x2b4:.data:gOverlay9DataBaseReloc
0x2b8:.data:gOverlay9DataBaseReloc
0x2e8:.data:gOverlay9DataBaseReloc
0x2dc:.data:gOverlay9DataBaseReloc
0x308:.data:gOverlay9DataBaseReloc
0x2f8:.data:gOverlay9DataBaseReloc
0x320:.data:gOverlay9DataBaseReloc
0x328:.data:gOverlay9DataBaseReloc
0x3f4:.data:gOverlay9DataBaseReloc
0x3f8:.data:gOverlay9DataBaseReloc
0x414:.data:gOverlay9DataBaseReloc
0x418:.data:gOverlay9DataBaseReloc
0x36c:.data:gOverlay9DataBaseReloc
0x41c:.data:gOverlay9DataBaseReloc
0x428:.data:gOverlay9DataBaseReloc
0x42c:.data:gOverlay9DataBaseReloc
