/*
 * Overlay 88 and overlay 69 ship the same renderer body.  Keep the public
 * function symbols overlay-local while sharing the implementation spelling.
 * The object recipes supply distinct runtime-relocation names for the callees.
 */
#define overlay69DrawSortedGeometry overlay88DrawSortedGeometry

/* Plateau: see the shared body's notes in overlay 69. */
#ifdef NON_MATCHING
#include "src/overlays/o069/overlay69DrawSortedGeometry.c"
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o088/overlay88DrawSortedGeometry/func_overlay_088_F00001A4_18D3C2C.s")
#endif
