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
 * too weak to establish a donor body (masked 4-gram Jaccard 0.077).
 *
 * Matched from 198 masked words by writing it with fourteen plain locals and
 * early exits. One `candidate` is the scan variable of both loops and of the
 * renumbering loop (the earlier split carrier cost a fifteenth frame cell);
 * the second scan rejects with one `continue`, which is what orders index
 * after the renumbering cursor and the indices base ahead of the unroller's
 * bound; `closestIndex` is reused for the closest object's slot,
 * `closestRoute` and `candidateRoute` for the renumbering loop's two routes;
 * the nonzero group returns early, so the chained group copy reloads route;
 * and the renumbering start sits in the for-init, which keeps the copy after
 * the cursor setup. */
void func_overlay_002_F0001DF8_1858BF0(Overlay2RouteObject *object,
                                        Overlay2RouteInput *input) {
    s32 start;
    s32 end;
    s32 index;
    Overlay2RouteObject **objects;
    u16 indices[0x400];
    Overlay2RouteObject *candidate;
    Overlay2RouteObject *closest;
    Overlay2RouteObject *previous;
    Overlay2RouteObject *next;
    Overlay2RouteState *closestRoute;
    Overlay2RouteState *candidateRoute;
    Overlay2RouteState *route;
    Overlay2ObjectHeader *header;
    s32 closestIndex;
    s32 unused;
    u32 bestDistance;
    u32 distance;
    u32 previousDistance;
    s32 count;

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
                candidate = objects[index];
                candidateRoute = candidate->route;
                if ((candidate->disabled != 0) || (candidate == object) ||
                    (candidate->type != 0x2B)) {
                    continue;
                }
                if (candidateRoute->group == closestRoute->group) {
                    count++;
                    indices[candidateRoute->order] = (u16)index;
                }
            }
        } else {
            input->group = 1;
            route->group = 1;
            return;
        }

        if (route->group != 0) {
            return;
        }

        route->group = input->group = closestRoute->group;
        if (count == 1) {
            route->order = input->order = 1;
            return;
        }
        if (count == 2) {
            route->order = input->order = 2;
            return;
        }

        if (joyGetButtons(0) & 0x100) {
            /* The closest object's slot in the group's order. */
            closestIndex = closest->header->order;
            if (closestIndex == 0) {
                previous = objects[indices[count - 1]];
                next = objects[indices[1]];
            } else if (closestIndex == count - 1) {
                previous = objects[indices[count - 2]];
                next = objects[indices[0]];
            } else {
                previous = objects[indices[closestIndex - 1]];
                next = objects[indices[closestIndex + 1]];
            }

            previousDistance = (u32)func_8000BCB0(
                previous->x, previous->y, previous->z, candidate->x,
                candidate->y, candidate->z);
            distance = (u32)func_8000BCB0(
                next->x, next->y, next->z, candidate->x, candidate->y,
                candidate->z);
            if (distance < previousDistance) {
                previous = closest;
            }

            closestRoute = previous->route;
            for (index = count - 1; closestRoute->order < index; index--) {
                candidate = objects[indices[index]];
                candidateRoute = candidate->route;
                header = candidate->header;
                candidateRoute->order++;
                header->order++;
            }
            route->order = input->order = closestRoute->order + 1;
            return;
        }

        input->order = (u8)count;
        route->order = (u8)count;
    }
}
