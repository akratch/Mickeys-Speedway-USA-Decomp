#include "PR/ultratypes.h"

/* Pinned DKR v77/v80 and JFG scans found no exact volume-builder donor. */

typedef struct Vec3f {
    f32 x;
    f32 y;
    f32 z;
} Vec3f;

typedef struct Overlay96Vertex {
    s16 x;
    s16 y;
    s16 z;
    u8 flag;
    u8 red;
    u8 green;
    u8 blue;
} Overlay96Vertex;

typedef struct Overlay96Plane {
    f32 x;
    f32 y;
    f32 z;
    f32 distance;
} Overlay96Plane;

typedef struct Overlay96Volume {
    Overlay96Vertex vertices[8];
    Overlay96Plane planes[6];
    u8 attributes[16];
} Overlay96Volume;

typedef struct Overlay96Object {
    u8 pad00[0x64];
    Overlay96Volume *volume;
} Overlay96Object;

typedef struct Overlay96Definition {
    u8 pad00[4];
    s16 centerX;
    s16 centerY;
    s16 centerZ;
    s16 extentX;
    s16 extentY;
    s16 extentZ;
    u8 angleX;
    u8 angleY;
    u8 attributes[16];
} Overlay96Definition;

typedef struct Overlay96FaceDefinition {
    u8 pad00;
    u8 first;
    u8 second;
    u8 third;
    u8 pad04[0x1C];
} Overlay96FaceDefinition;

extern Overlay96FaceDefinition gO96FaceDefinitionsReloc[6];
extern void pointListRPY(s32 count, s16 *angles, Vec3f *source,
                                          Vec3f *destination);
extern f32 sqrtf(f32 value);
extern void overlay96Register(Overlay96Volume *volume);

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
