#ifndef OVERLAY_096_H
#define OVERLAY_096_H

#include "PR/ultratypes.h"

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

/* Also read by overlay96DrawObject as its vertex buffer. */
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

typedef struct Overlay96Command {
    u32 w0;
    u32 w1;
} Overlay96Command;

typedef struct Overlay96DrawLocals {
    void *vertices;
} Overlay96DrawLocals;

extern s32 D_80078F84;
extern Overlay96Volume *gO96EntriesReloc[16];
extern Overlay96FaceDefinition gO96FaceDefinitionsReloc[6];
extern u8 D_80000000[];

extern void pointListRPY(s32 count, s16 *angles, Vec3f *source,
                         Vec3f *destination);
extern f32 sqrtf(f32 value);
extern void texDPTextureX(Overlay96Command **commands, void *resource,
                          s32 mode, s32 flags);

void overlay96Register(void *volume);
void overlay96Unregister(void *volume);
void overlay96BuildVolume(Overlay96Object *object,
                          Overlay96Definition *definition);
Overlay96Volume *overlay96FindVolume(f32 x, f32 y, f32 z);
s32 overlay96TestBit(u8 *record, s32 index);
void overlay96DrawObject(Overlay96Command **commands, s32 unused1,
                         s32 unused2, Overlay96Object *object);

#endif
