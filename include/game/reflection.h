#ifndef GAME_REFLECTION_H
#define GAME_REFLECTION_H

#include "PR/ultratypes.h"
#include "n_audio/gbi.h"

struct TrackVertex;

/* The reflection renderer advances the caller's fixed-matrix cursor. */
void overlay98RenderReflections(Gfx **commands, Mtx **matrices,
                                struct TrackVertex **vertices);

#endif
