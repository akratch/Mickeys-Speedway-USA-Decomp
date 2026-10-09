#include "PR/ultratypes.h"

typedef struct Overlay40Color {
    u8 red;
    u8 green;
    u8 blue;
} Overlay40Color;

typedef struct Overlay40ColorSource {
    Overlay40Color *colors;
    s16 *groups;
} Overlay40ColorSource;

typedef struct Overlay40Vertex {
    u8 pad00[6];
    s8 red;
    s8 green;
    s8 blue;
    u8 pad09;
} Overlay40Vertex;

typedef struct Overlay40FadeRecord {
    Overlay40Vertex *vertices;
    u8 pad04[0x1C];
    s16 count;
    u8 pad22[0xC];
    u8 dirty;
    u8 pad2F;
    Overlay40ColorSource *source;
    u8 pad34[0xC];
} Overlay40FadeRecord;

typedef struct Overlay40FadeContext {
    u8 pad00[4];
    Overlay40FadeRecord *records;
    u8 pad08[0x12];
    s16 count;
} Overlay40FadeContext;

extern s16 gOverlay40BlendTimer;
extern s16 gOverlay40BlendCurrent;
extern s16 gOverlay40BlendTarget;
extern s16 gOverlay40BlendDuration;
extern s16 gOverlay40BlendOutput;

/* The blend base is read twice, not copied into a second local. The target
 * wants the load itself in v1 and the running output in v0; with two named
 * locals both take pool colours in first-definition order, so whichever name
 * the load defines takes v0 and the pair can only be swapped, never separated
 * (98/101 either way round). Spelling the second use as the global read again
 * makes the loaded value a CSE temporary, which uopt numbers after every
 * named local, so it takes v1 and `output` -- now defined by the copy -- takes
 * v0. Keeping `current` declared *and* assigned from the global costs a second
 * %hi materialisation (102 words); dropping the local is what collapses it.
 * The object emits all ten runtime HI16/LO16 roles: D_800D6C4C(timer),
 * D_800D6C52(current), D_800D6C50(target), D_800D6C4E(duration), and
 * D_800D6C54(output).
 * Owned Overlay 40 +0x690..+0x824 / ROM 0x1886F40..0x18870D4 excludes separate
 * +0x824..+0x830 padding. ORT 1314 and resident runtime record 156 at
 * trackUpdateLighting+0x130 authenticate the sole inbound trap site. Mickey-only;
 * exact pinned DKR v77/v80/JFG scans are negative. */
void overlay40FadeRecords(register s32 *enabled, Overlay40FadeContext *context,
                          s32 amount) {
    Overlay40FadeRecord *record;
    Overlay40ColorSource *source;
    Overlay40Color *color;
    Overlay40Vertex *vertex;
    s16 *group;
    s32 output;
    s32 timer;
    s32 remaining;
    s32 groupRemaining;
    s32 vertexRemaining;

    output = gOverlay40BlendCurrent;
    timer = gOverlay40BlendTimer;
    if (timer != 0) {
        if (amount < timer) {
            gOverlay40BlendTimer = timer - amount;
            output += ((gOverlay40BlendTarget - gOverlay40BlendCurrent) *
                      gOverlay40BlendTimer) / gOverlay40BlendDuration;
        } else {
            gOverlay40BlendTimer = 0;
        }
        gOverlay40BlendOutput = output;
    }

    record = context->records;
    remaining = context->count;
    while (remaining--) {
        if (*enabled != 0) {
            source = record->source;
            if (source != 0) {
                group = source->groups;
                groupRemaining = (record->count + 0xF) >> 4;
                while (groupRemaining--) {
                    *group++ = 0;
                }

                color = source->colors;
                vertexRemaining = record->count;
                vertex = record->vertices;
                while (vertexRemaining--) {
                    vertex->red = (color->red * output) >> 8;
                    vertex->green = (color->green * output) >> 8;
                    vertex->blue = (color->blue * output) >> 8;
                    vertex++;
                    color++;
                }
                record->dirty = 0;
            }
        }
        enabled++;
        record++;
    }
}
