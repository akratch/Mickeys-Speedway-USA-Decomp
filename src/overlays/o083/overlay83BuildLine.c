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

typedef struct Overlay83Vector {
    f32 x;
    f32 y;
    f32 z;
} Overlay83Vector;

typedef struct Overlay83LineTransform {
    u8 pad00[8];
    f32 halfLength;
    f32 x;
    f32 y;
    f32 z;
} Overlay83LineTransform;

extern void pointListRPY(s32 count, Overlay83LineTransform *transform,
                                    Overlay83Vector *source,
                                    Overlay83Vector *destination);

/*
 * DKR v77/v80 and JFG have generic transformed-vertex builders, but their
 * object scans contain no exact donor for overlay 83.
 */
void overlay83BuildLine(Overlay83Vertex **vertices,
                        Overlay83LineTransform *transform, s32 alpha) {
    Overlay83Vector points[2];

    points[0].x = -transform->halfLength;
    points[0].y = 0.0f;
    points[0].z = 0.0f;
    points[1].x = transform->halfLength;
    points[1].y = 0.0f;
    points[1].z = 0.0f;
    pointListRPY(2, transform, points, points);

    points[0].x += transform->x;
    points[0].y += transform->y;
    points[0].z += transform->z;
    points[1].x += transform->x;
    points[1].y += transform->y;
    points[1].z += transform->z;

    (*vertices)->x = points[0].x;
    (*vertices)->y = points[0].y;
    (*vertices)->z = points[0].z;
    (*vertices)->red = 0xFF;
    (*vertices)->green = 0xFF;
    (*vertices)->blue = 0xFF;
    (*vertices)->alpha = alpha;
    (*vertices)++;

    (*vertices)->x = points[1].x;
    (*vertices)->y = points[1].y;
    (*vertices)->z = points[1].z;
    (*vertices)->red = 0xFF;
    (*vertices)->green = 0xFF;
    (*vertices)->blue = 0xFF;
    (*vertices)->alpha = alpha;
    (*vertices)++;
}
