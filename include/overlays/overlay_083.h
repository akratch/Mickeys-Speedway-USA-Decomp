#ifndef OVERLAY_083_H
#define OVERLAY_083_H

#include "PR/ultratypes.h"
#include "game/lights.h"
#include "n_audio/mbi.h"

/* Overlay 83: a batch of light streaks. Each entry keeps an eight-record
 * history ring and two double-buffered line-vertex buffers. Pinned DKR
 * v77/v80 and JFG object scans found no exact donor. */

typedef struct Overlay83Command {
    u32 w0;
    u32 w1;
} Overlay83Command;

typedef struct Overlay83Vertex {
    s16 x;
    s16 y;
    s16 z;
    u8 red;
    u8 green;
    u8 blue;
    u8 alpha;
} Overlay83Vertex;

typedef struct Overlay83Vector {
    f32 x;
    f32 y;
    f32 z;
} Overlay83Vector;

/* The prefix BuildLine reads from either an entry or a history record. */
typedef struct Overlay83LineTransform {
    u8 pad00[8];
    f32 halfLength;
    f32 x;
    f32 y;
    f32 z;
} Overlay83LineTransform;

/* One history record (0x18 bytes). */
typedef struct Overlay83Record {
    s16 x;
    s16 y;
    s16 z;
    s8 life;
    u8 alpha;
    f32 halfLength;
    f32 worldX;
    f32 worldY;
    f32 worldZ;
} Overlay83Record;

/* Nine lines of two vertices: the head plus eight history records. */
typedef struct Overlay83VertexBuffer {
    Overlay83Vertex vertices[18];
} Overlay83VertexBuffer;

/* One batch entry (0x258 bytes). */
typedef struct Overlay83Entry {
    s16 x;
    s16 y;
    s16 z;
    s16 height;
    f32 halfLength;
    f32 worldX;
    f32 worldY;
    f32 worldZ;
    s16 drawHalfLength;
    s16 velocityX;
    s16 velocityY;
    s16 velocityZ;
    u8 red;
    u8 green;
    u8 blue;
    u8 alpha;
    f32 zero24;
    UnkLight *light;
    u8 firstRecord;
    u8 lastRecord;
    u8 recordCount;
    u8 bufferIndex;
    Overlay83Record records[8];
    Overlay83VertexBuffer vertexBuffers[2];
} Overlay83Entry;

typedef struct Overlay83Batch {
    u8 count;
    u8 pad01;
    s16 alpha;
    Overlay83Entry *entries;
} Overlay83Batch;

typedef struct Overlay83SourceRecord {
    s8 x;
    s8 y;
    s8 z;
    s8 velocityX;
    s8 velocityY;
    s8 velocityZ;
    u8 red;
    u8 green;
    u8 blue;
    u8 scale;
    u8 halfLength;
    u8 alpha;
    u16 flags;
} Overlay83SourceRecord;

typedef struct Overlay83Source {
    u8 pad00[0xA];
    u8 count;
    u8 alpha;
    Overlay83SourceRecord records[1];
} Overlay83Source;

typedef struct Overlay83Nested {
    u8 pad00[0xAC];
    void **resource;
} Overlay83Nested;

typedef struct Overlay83Parent {
    u8 pad00[0xC];
    f32 worldX;
    f32 worldY;
    f32 worldZ;
    u8 pad18[0x24];
    s32 clear3C;
    Overlay83Nested *nested;
    u8 pad44[0x20];
    Overlay83Batch *batch;
    void **setup;
} Overlay83Parent;

extern u8 D_80000000[];
extern f32 gOverlay83ScaleReloc;

extern void *mmAlloc(s32 size, u32 colourTag);
extern void pointListRPY(s32 count, void *transform, void *source,
                         void *destination);
extern void sprSetIA2ColOverride(s32 a0, s32 a1, s32 a2, s32 a3,
                                 s32 a4, s32 a5);
extern void camDoSprite(Overlay83Command **displayList, void *mtx,
                        void *vertices, Overlay83Entry *entry,
                        void *resource, s32 mode, s32 alpha);
extern void sprClearIA2ColOverride(void);
extern void overlay83SubmitSetupReloc(Overlay83Command **displayList,
                                      void *data, s32 mode, s32 extra);

void overlay83BuildBatch(Overlay83Parent *parent, Overlay83Source *source);
void overlay83DrawEntries(Overlay83Parent *parent, s32 updateRate);
void overlay83SubmitAll(Overlay83Command **displayList, void *mtx,
                        void *vertices, Overlay83Parent *parent);

#endif
