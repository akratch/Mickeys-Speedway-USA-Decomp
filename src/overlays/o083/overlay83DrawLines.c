#include "PR/ultratypes.h"

typedef struct Overlay83Vertex {
    s16 x;
    s16 y;
    s16 z;
    u8 red;
    u8 green;
    u8 blue;
    u8 alpha;
} Overlay83Vertex;

typedef struct Overlay83LineRecord {
    u8 pad00[7];
    u8 alpha;
    f32 halfLength;
    f32 x;
    f32 y;
    f32 z;
} Overlay83LineRecord;

typedef struct Overlay83LineObject {
    u8 pad00[8];
    f32 halfLength;
    f32 x;
    f32 y;
    f32 z;
    s16 drawHalfLength;
    u8 pad1A[9];
    u8 alpha;
    u8 pad24[8];
    u8 firstRecord;
    u8 lastRecord;
    u8 active;
    u8 bufferIndex;
    Overlay83LineRecord records[8];
    u8 padF0[0x168];
} Overlay83LineObject;

extern void overlay83BuildLine(Overlay83Vertex **vertices, void *transform,
                                    s32 alpha);

/*
 * DKR v77/v80 and JFG contain generic transformed-line and debug-geometry
 * code, but their object scans contain no exact donor for this history-line
 * renderer.
 */
void overlay83DrawLines(Overlay83LineObject *object) {
    if (object->active != 0) {
        s32 recordIndex;
        f32 savedHalfLength;
        Overlay83Vertex *vertices;

        object->bufferIndex ^= 1;
        vertices = (Overlay83Vertex *)
            ((u8 *)object + object->bufferIndex * 0xB4 + 0xF0);
        savedHalfLength = object->halfLength;
        object->halfLength = object->drawHalfLength;
        overlay83BuildLine(&vertices, object, object->alpha);
        object->halfLength = savedHalfLength;

        recordIndex = object->lastRecord;
        do {
            recordIndex--;
            if (recordIndex < 0) {
                recordIndex = 7;
            }
            overlay83BuildLine(&vertices, &object->records[recordIndex],
                                    object->records[recordIndex].alpha);
        } while (recordIndex != object->firstRecord);
    }
}
