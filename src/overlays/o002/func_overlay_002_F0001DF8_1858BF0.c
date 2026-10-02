#include "PR/ultratypes.h"

typedef struct Overlay2RouteState {
    u8 group;
    u8 order;
} Overlay2RouteState;

typedef struct Overlay2ObjectHeader {
    u8 pad00[0xB];
    u8 order;
} Overlay2ObjectHeader;

typedef struct Overlay2RouteObject {
    u8 pad00[0xC];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x24];
    Overlay2ObjectHeader *header;
    u8 pad40[4];
    s16 type;
    u8 pad46[0x1E];
    Overlay2RouteState *route;
    u8 pad68[0x29];
    u8 disabled;
} Overlay2RouteObject;

typedef struct Overlay2RouteInput {
    u8 pad00[0xA];
    u8 group;
    u8 order;
} Overlay2RouteInput;

extern Overlay2RouteObject **func_8000572C(s32 *start, s32 *end);
extern f32 func_8000BCB0(f32 x0, f32 y0, f32 z0, f32 x1, f32 y1,
                         f32 z1);
extern u32 joyGetButtons(s32 controller);

/* Mickey-only reconstruction. The closest permitted reference skeleton is
 * too weak to establish a donor body (masked 4-gram Jaccard 0.077). */
/* NON_MATCHING: the second scan loads lastCandidate from objects[index]
 * and copies it to candidate. 198 masked words at size delta 0, frame
 * 0x888 against 0x880 (one compiler temp cell too many). From 353: the
 * declarations in home order (lastCandidate 0x6C, closest 0x68, previous
 * 0x64, ..., count 0x38), an explicit return after the no-closest arm, the
 * order stores chained (`route->order = input->order = ...`, input stored
 * first), and the tail's start as `count - 1U` (L151: an int 1 lets uopt
 * hoist the joypad branch's `count - 1` and share it with the tail). */
#ifdef NON_MATCHING
void func_overlay_002_F0001DF8_1858BF0(Overlay2RouteObject *object,
                                        Overlay2RouteInput *input) {
    s32 start;
    s32 end;
    s32 index;
    Overlay2RouteObject **objects;
    u16 indices[0x400];
    Overlay2RouteObject *lastCandidate;
    Overlay2RouteObject *closest;
    Overlay2RouteObject *previous;
    Overlay2RouteObject *next;
    Overlay2RouteState *closestRoute;
    Overlay2RouteState *candidateRoute;
    Overlay2RouteState *route;
    Overlay2ObjectHeader *header;
    s32 closestIndex;
    s32 position;
    u32 bestDistance;
    u32 distance;
    u32 previousDistance;
    s32 count;
    Overlay2RouteObject *candidate;

    objects = func_8000572C(&start, &end);
    route = object->route;
    closestIndex = -1;
    bestDistance = (u32)-1;
    route->group = input->group;
    count = 0;
    route->order = input->order;

    if (input->group < 2) {
        for (index = start; index < end; index++) {
            candidate = objects[index];
            lastCandidate = candidate;
            if ((candidate->disabled == 0) && (candidate != object) &&
                (candidate->type == 0x2B)) {
                distance = (u32)func_8000BCB0(
                    object->x, object->y, object->z, candidate->x,
                    candidate->y, candidate->z);
                if (distance < bestDistance) {
                    bestDistance = distance;
                    closestIndex = index;
                }
            }
        }

        if (closestIndex != -1) {
            closest = objects[closestIndex];
            closestRoute = closest->route;
            for (index = start; index < end; index++) {
                lastCandidate = objects[index];
                candidate = lastCandidate;
                candidateRoute = lastCandidate->route;
                if ((lastCandidate->disabled == 0) && (lastCandidate != object) &&
                    (lastCandidate->type == 0x2B) &&
                    (closestRoute->group == candidateRoute->group)) {
                    count++;
                    indices[candidateRoute->order] = (u16)index;
                }
            }
        } else {
            input->group = 1;
            route->group = 1;
            return;
        }

        if (route->group == 0) {
            input->group = closestRoute->group;
            route->group = closestRoute->group;
            if (count == 1) {
                input->order = route->order = 1;
                return;
            }
            if (count == 2) {
                input->order = route->order = 2;
                return;
            }

            if (joyGetButtons(0) & 0x100) {
                header = closest->header;
                position = header->order;
                if (position == 0) {
                    previous = objects[indices[count - 1]];
                    next = objects[indices[1]];
                } else if (count == position + 1) {
                    previous = objects[indices[count - 2]];
                    next = objects[indices[0]];
                } else {
                    previous = objects[indices[position - 1]];
                    next = objects[indices[position + 1]];
                }

                previousDistance = (u32)func_8000BCB0(
                    previous->x, previous->y, previous->z,
                    lastCandidate->x, lastCandidate->y, lastCandidate->z);
                distance = (u32)func_8000BCB0(
                    next->x, next->y, next->z, lastCandidate->x,
                    lastCandidate->y, lastCandidate->z);
                if (distance < previousDistance) {
                    previous = closest;
                }

                candidateRoute = previous->route;
                position = count - 1U;
                while (candidateRoute->order < position) {
                    candidate = objects[indices[position]];
                    candidate->route->order++;
                    candidate->header->order++;
                    position--;
                }
                route->order = input->order = candidateRoute->order + 1;
                return;
            }

            input->order = (u8)count;
            route->order = (u8)count;
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o002/func_overlay_002_F0001DF8_1858BF0/func_overlay_002_F0001DF8_1858BF0.s")
#endif

/* PLATEAU-HANDOFF:func_overlay_002_F0001DF8_1858BF0:start
 * symbol: func_overlay_002_F0001DF8_1858BF0
 * score: 198/460 words
 * frame: 0x888
 * relocations: 5
 * first-mismatch: +0x0
 * summary: Home-order locals, no-closest return, chained order stores, 1U tail start: 353 to 198 at delta 0; frame 0x888 vs 0x880 (one temp cell).
 * PLATEAU-HANDOFF:func_overlay_002_F0001DF8_1858BF0:end
 */
