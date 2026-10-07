/*
 * Overlay 88 and overlay 69 ship the same renderer body.  Keep the public
 * function symbols overlay-local while sharing the implementation spelling.
 * The object recipes supply distinct runtime-relocation names for the callees.
 */
#define overlay69DrawSortedGeometry overlay88DrawSortedGeometry

#include "src/overlays/o069/overlay69DrawSortedGeometry.c"
