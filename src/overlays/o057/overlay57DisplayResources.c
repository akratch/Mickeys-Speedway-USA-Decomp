#include "PR/ultratypes.h"
#include "n_audio/mbi.h"

/* Tier C: a complete eleven-packet GBI state list ends at EndDisplayList,
 * immediately before the independently consumed descriptor-index arrays.
 * No current caller/ORT reference to this list is established. These are
 * actual SDK command fields, not opaque words or invented padding.
 * Tier D: original names/qualifiers are unknown; preserve mutable storage. */
Gfx o57GridDisplayState[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsDPSetTextureLOD(G_TL_TILE),
    gsDPSetTextureLUT(G_TT_NONE),
    gsDPSetTextureDetail(G_TD_CLAMP),
    gsDPSetTexturePersp(G_TP_NONE),
    gsDPSetTextureFilter(G_TF_BILERP),
    gsDPSetTextureConvert(G_TC_FILT),
    gsDPSetAlphaCompare(G_AC_NONE),
    gsDPSetRenderMode(G_RM_CLD_SURF, G_RM_CLD_SURF2),
    gsSPEndDisplayList(),
};

/* Tier B: the initializer's two nine-iteration loops read signed halfword
 * indices into the resident front-end resource table. */
s16 o57LeftDescriptorResources[] = {54, 55, 56, 57, 58, 59, 60, 61, 10};
s16 o57RightDescriptorResources[] = {54, 55, 56, 57, 58, 63, 61, 59, 10};
