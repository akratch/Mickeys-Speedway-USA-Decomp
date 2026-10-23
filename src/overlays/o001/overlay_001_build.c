#include "overlays/overlay_001.h"
#include "game/math.h"

/* ---- overlay1BuildPointRecord ---- */


/*
 * Overlay 1 text +0x7B0..+0xBD4.
 *
 * The shipped object was compiled with four-way loop unrolling and the R4300
 * multiply scheduling workaround.  The two imported angle-component calls
 * occupy distinct runtime relocation records even though both unresolved JAL
 * fields are zero in the stored overlay image.
 */

typedef struct Overlay1PointTemplate {
    u8 unknown0[4];
    s16 base0;
    s16 base1;
    s16 base2;
    s16 angle;
    u16 headerValue;
    s8 radial[8];
    s8 metadata[8];
    u16 flags;
} Overlay1PointTemplate;

typedef struct Overlay1GeneratedPoint {
    f32 component0;
    f32 component1;
    f32 component2;
    s8 metadata;
    u8 untouched[3];
} Overlay1GeneratedPoint;

typedef struct Overlay1PointRecord {
    f32 base0;
    f32 base1;
    f32 base2;
    s16 angle;
    u16 headerValue;
    u16 flags;
    u8 untouched12[2];
    Overlay1GeneratedPoint points[8];
} Overlay1PointRecord;


void overlay1BuildPointRecord(Overlay1PointRecord *output,
                              const Overlay1PointTemplate *source,
                              s32 unused) {
    f32 angleComponent0;
    f32 angleComponent1;
    f32 scale;
    s32 i;
    Overlay1GeneratedPoint *point;
    const Overlay1PointTemplate *sourceCursor;

    (void)unused;

    angleComponent0 = func_8002A8C0(source->angle);
    angleComponent1 = func_8002A8BC(source->angle);

    if (source->flags & 2) {
        scale = -16.0f;
    } else {
        scale = -8.0f;
    }

    output->angle = source->angle;
    output->headerValue = source->headerValue;
    output->flags = source->flags;
    output->base0 = source->base0;
    output->base1 = source->base1;
    output->base2 = source->base2;

    i = 0;
    point = output->points;
    sourceCursor = source;
    do {
        point->component0 =
            ((f32)sourceCursor->radial[0] * angleComponent0) * scale +
            (f32)source->base0;
        point->component1 =
            ((f32)sourceCursor->radial[0] * angleComponent1) * scale +
            (f32)source->base2;
        point->component2 = 0.0f;
        point->metadata = sourceCursor->metadata[0];
        point++;
        sourceCursor =
            (const Overlay1PointTemplate *)((const u8 *)sourceCursor + 1);
        i++;
    } while (i != 8);
}
