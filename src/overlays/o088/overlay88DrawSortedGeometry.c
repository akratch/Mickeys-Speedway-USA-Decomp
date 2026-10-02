/*
 * Overlay 88 and overlay 69 ship the same renderer body.  Keep the public
 * symbols overlay-local while sharing the reviewed implementation spelling.
 */
#define SHARED_DRAW_FUNCTION overlay88DrawSortedGeometry
#define SHARED_FIXED_RESOURCE_RELOC overlay88DrawFixedResourceReloc
#define SHARED_METRIC_RELOC overlay88MetricReloc
#define SHARED_TRANSFORM_RELOC overlay88PrepareTransformReloc
#define SHARED_DYNAMIC_SUBMIT_RELOC overlay88SubmitDynamicReloc
#define SHARED_FIXED_SUBMIT_RELOC overlay88DrawConeReloc

/* Plateau: see the shared body's notes in overlay 69. */
#ifdef NON_MATCHING
#include "src/overlays/o069/overlay69DrawSortedGeometry.c"
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o088/overlay88DrawSortedGeometry/func_overlay_088_F00001A4_18D3C2C.s")
#endif
