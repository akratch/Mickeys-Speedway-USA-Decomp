#include "overlays/overlay_096.h"

void overlay96Register(void *volume) {
    s32 index;

    index = D_80078F84;
    while (index--) {
        if (gO96EntriesReloc[index] == volume) {
            return;
        }
    }
    if (D_80078F84 < 16) {
        gO96EntriesReloc[D_80078F84] = volume;
        D_80078F84++;
    }
}

/* Matched 2026-09-17 (lane w7-o096), 25 -> 0 masked words at delta 0,
 * frameless, six relocations, unforced. L160: delete the walking entry
 * pointer and subscript gO96EntriesReloc[index] in a while so IDO generates
 * the cursor. Lever 45: repeating the count global buys the retained base
 * and the count-1 copy. Declaration census (lever 46) is the sibling's
 * single index local. */
void overlay96Unregister(void *volume) {
    s32 index;

    index = D_80078F84;
    while (index--) {
        if (gO96EntriesReloc[index] == volume) {
            D_80078F84--;
            while (index < D_80078F84) {
                gO96EntriesReloc[index] = gO96EntriesReloc[index + 1];
                index++;
            }
            return;
        }
    }
}

void overlay96BuildVolume(Overlay96Object *object,
                          Overlay96Definition *definition) {
    f32 extentZ;
    f32 normalX;
    f32 normalY;
    f32 normalZ;
    f32 ax;
    f32 ay;
    f32 az;
    f32 bx;
    f32 by;
    f32 bz;
    f32 cx;
    f32 cy;
    f32 cz;
    f32 magnitudeSquared;
    f32 scale;
    Overlay96Volume *volume;
    Vec3f *point;
    s16 angles[3];
    Vec3f points[8];
    Overlay96Vertex *vertex;
    Overlay96Plane *plane;
    Overlay96FaceDefinition *face;
    s32 count;

    volume = object->volume;
    ax = (f32)definition->extentX;
    ay = (f32)definition->extentY;
    extentZ = (f32)definition->extentZ;

    points[0].x = -ax;
    points[0].y = -extentZ;
    points[0].z = -ay;
    points[1].x = ax;
    points[1].y = -extentZ;
    points[1].z = -ay;
    points[2].x = -ax;
    points[2].y = -extentZ;
    points[2].z = ay;
    points[3].x = ax;
    points[3].y = -extentZ;
    points[3].z = ay;
    points[4].x = -ax;
    points[4].y = extentZ;
    points[4].z = -ay;
    points[5].x = ax;
    points[5].y = extentZ;
    points[5].z = -ay;
    points[6].x = -ax;
    points[6].y = extentZ;
    points[6].z = ay;
    points[7].x = ax;
    points[7].y = extentZ;
    points[7].z = ay;

    angles[0] = definition->angleX << 8;
    angles[1] = definition->angleY << 6;
    angles[2] = 0;
    pointListRPY(8, angles, points, points);

    az = (f32)definition->centerX;
    ay = (f32)definition->centerY;
    ax = (f32)definition->centerZ;
    point = points;
    vertex = volume->vertices;
    count = 7;
    do {
        point->x += az;
        point->y += ay;
        point->z += ax;
        vertex->x = point->x;
        vertex->y = point->y;
        vertex->z = point->z;
        vertex->flag = 0;
        vertex->red = 0x40;
        vertex->green = 0xFF;
        vertex->blue = 0x80;
        point++;
        vertex++;
    } while (count--);

    plane = volume->planes;
    face = gO96FaceDefinitionsReloc;
    count = 5;
    do {
        ax = points[face->first].x;
        ay = points[face->first].y;
        az = points[face->first].z;
        bx = points[face->second].x;
        by = points[face->second].y;
        bz = points[face->second].z;
        cx = points[face->third].x;
        cy = points[face->third].y;
        cz = points[face->third].z;

        normalX = (by - ay) * (cz - bz) - (bz - az) * (cy - by);
        normalY = (bz - az) * (cx - bx) - (bx - ax) * (cz - bz);
        normalZ = (bx - ax) * (cy - by) - (by - ay) * (cx - bx);
        magnitudeSquared = normalX * normalX + normalY * normalY +
                           normalZ * normalZ;
        scale = magnitudeSquared;
        if (scale > 0.0f) {
            scale = 1.0f / sqrtf(scale);
        }
        plane->x = normalX * scale;
        plane->y = normalY * scale;
        plane->z = normalZ * scale;
        plane->distance = -(plane->x * ax + ay * plane->y + az * plane->z);
        face++;
        plane++;
    } while (count--);

    count = 15;
    do {
        volume->attributes[count] = definition->attributes[count];
    } while (count--);
    overlay96Register(volume);
}

/* Pinned DKR v77/v80 and JFG scans found no exact donor. */
Overlay96Volume *overlay96FindVolume(f32 x, f32 y, f32 z) {
    s32 count = D_80078F84;
    s32 planeIndex;
    s32 inside;
    Overlay96Volume *volume;
    Overlay96Plane *plane;
    f32 distance;

    while (count--) {
        volume = gO96EntriesReloc[count];
        plane = volume->planes;
        inside = 1;
        planeIndex = 5;
        do {
            distance = plane->x * x + y * plane->y +
                       z * plane->z + plane->distance;
            plane++;
            if (distance < 0.0f) {
                inside = 0;
                break;
            }
        } while (planeIndex--);
        if (inside != 0) {
            return volume;
        }
    }
    return NULL;
}

/* DKR source/object scans found no corresponding bit-table query. */
s32 overlay96TestBit(u8 *record, s32 index) {
    u8 *byte;

    if ((index >= 0) && (index < 0x80) && (record != NULL)) {
        byte = record;
        byte += index >> 3;
        if (byte[0xB0] & (1 << (index & 7))) {
            return -1;
        }
    }
    return 0;
}

#define OVERLAY96_SHIFTL(value, shift, width) \
    (((u32)(value) & ((1U << (width)) - 1U)) << (shift))

#define OVERLAY96_PIPE_SYNC(packet)                                      \
    {                                                                    \
        Overlay96Command *macroCommand = (Overlay96Command *)(packet);    \
        macroCommand->w0 = 0xE7000000;                                   \
        macroCommand->w1 = 0;                                            \
    }

#define OVERLAY96_PRIM_WHITE(packet)                                     \
    {                                                                    \
        Overlay96Command *macroCommand = (Overlay96Command *)(packet);    \
        macroCommand->w0 = 0xFA000000;                                   \
        macroCommand->w1 = 0xFFFFFFFF;                                   \
    }

#define OVERLAY96_ENV_WHITE(packet)                                      \
    {                                                                    \
        Overlay96Command *macroCommand = (Overlay96Command *)(packet);    \
        macroCommand->w0 = 0xFB000000;                                   \
        macroCommand->w1 = 0xFFFFFF00;                                   \
    }

#define OVERLAY96_LOAD_VERTICES(packet, address)                         \
    {                                                                    \
        Overlay96Command *macroCommand = (Overlay96Command *)(packet);    \
        void *vertexAddress = (void *)(address);                          \
        macroCommand->w0 =                                                \
            ((((((u32)vertexAddress & 6U) | 0x40U) & 0xFFU) << 16) |     \
             0x04000000U) |                                              \
            0x58U;                                                       \
        macroCommand->w1 = (u32)vertexAddress;                            \
    }

#define OVERLAY96_DRAW_TRIANGLES(packet, address, count, textured)       \
    {                                                                    \
        Overlay96Command *macroCommand = (Overlay96Command *)(packet);    \
        macroCommand->w0 =                                                \
            OVERLAY96_SHIFTL((((count) - 1) << 4) | (textured), 16, 8) | \
            OVERLAY96_SHIFTL(5, 24, 8) |                                 \
            OVERLAY96_SHIFTL((count) * 16, 0, 16);                        \
        macroCommand->w1 = (u32)(address);                                \
    }

/* DKR v77/v80 and JFG have generic display-list relatives, but no donor. */
void overlay96DrawObject(Overlay96Command **commands, s32 unused1,
                         s32 unused2, Overlay96Object *object) {
    Overlay96DrawLocals locals;

    locals.vertices = object->volume;
    OVERLAY96_PIPE_SYNC((*commands)++);
    OVERLAY96_PRIM_WHITE((*commands)++);
    OVERLAY96_ENV_WHITE((*commands)++);
    texDPTextureX(commands, NULL, 0x16, 0);
    OVERLAY96_LOAD_VERTICES(
        (*commands)++, (void *)((u32)locals.vertices + 0x80000000U));
    OVERLAY96_DRAW_TRIANGLES((*commands)++, D_80000000, 12, 0);
    OVERLAY96_PIPE_SYNC((*commands)++);
}
