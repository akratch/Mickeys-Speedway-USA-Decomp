#include "PR/ultratypes.h"

typedef struct Overlay60SpawnDesc {
    s16 objectId;
    s8 size;
    u8 pad03;
    s16 x;
    s16 y;
    s16 z;
    u8 pad0A;
    u8 alpha;
    u8 pad0C;
} Overlay60SpawnDesc;

typedef struct Overlay60Inner {
    u8 pad00[8];
    s16 mode;
} Overlay60Inner;

typedef struct Overlay60Object {
    u8 pad00[0xC];
    f32 x;
    f32 y;
    u8 pad14[2];
    u8 flags;
    u8 pad17[0x51];
    Overlay60Inner **inner;
} Overlay60Object;

extern u8 *D_8007C0B8;
extern u8 D_800D31C8[];
extern u32 gOverlay60Data00[10];
extern u32 gOverlay60Data28[4];
extern u32 gOverlay60Data38[2];
extern s32 gOverlay60Data40;
extern s32 gOverlay60Data44;
extern s32 gOverlay60Data48;
extern s32 gOverlay60Data4C;
extern s32 gOverlay60Data58;

extern u8 gOverlay60Bss00[];
extern u32 gOverlay60Bss20;
extern u32 gOverlay60Bss40;
extern u32 gOverlay60Bss60;
extern u32 gOverlay60Bss80;
extern Overlay60Object *gOverlay60ObjectC8;
extern Overlay60Object *gOverlay60ObjectCC;
extern Overlay60Object *gOverlay60ObjectCCY;
extern Overlay60Object *gOverlay60ObjectD0;
extern Overlay60Object *gOverlay60ObjectD0Y;
extern Overlay60Object *gOverlay60ObjectD4;
extern Overlay60Object *gOverlay60ObjectD4Y;
extern s16 gOverlay60CoordsD8[];
extern s16 gOverlay60CoordE8;
extern s16 gOverlay60CoordEA;
extern s16 gOverlay60CoordEC;
extern s16 gOverlay60CoordEE;
extern s16 gOverlay60CoordF0;
extern s16 gOverlay60CoordF2;
extern s16 gOverlay60CoordF4;
extern s16 gOverlay60CoordF6;
extern s32 gOverlay60Bss174;
extern u32 gOverlay60Bss144;
extern u32 gOverlay60Bss1E4;
extern u32 gOverlay60Bss1EC;
extern u32 gOverlay60Bss0C;

extern Overlay60Object *func_8000590C(Overlay60SpawnDesc *desc, s32 arg1);
extern void func_8005AD64(Overlay60Object *object, s32 arg1, s32 arg2,
                                    f32 arg3);
extern void loadFrontEndList(void *arg0);
extern s32 frontGetScreenMode(void);
extern void animseqStartPath(u8 index);
extern Overlay60Object *func_800508B4(u8 index);

#define SOURCE(type, offset) (*(type *)(D_8007C0B8 + (offset)))

/* Workbench: mixed structure/schedule/register, 17 words, first +0x108.
 * A fidelity-clean allocator trace could not attribute source webs or stack
 * homes, and coordinate carriers regressed the frame.
 *
 * 2026-09-10, lane c6-close: 21 -> 17 in two independent edits, both of them
 * ordinary C.
 *
 * The first is L59's structural form. The counter and the object cursor were
 * initialised on two statements ahead of a guarded `do`; moving BOTH into one
 * `for` header makes them share a source line, which is the tie-break as1
 * reaches last, and the two hoisted address materialisations then complete
 * before the counter is zeroed as they do in the target. That alone is 21 ->
 * 19. The bound was already register-resident, so this is not the counted-`for`
 * rewrite the earlier closure measured at 28 -- what pays here is the shared
 * line, not the loop shape: `i = 0, objects = ...;` on one statement before an
 * unchanged `do` scores 22, and putting the cursor first in the header scores
 * 27.
 *
 * The second is the descriptor store order, worth 19 -> 17. The residual is
 * schedule-sensitive to it: 23,649 orders of the nine loop-body statements
 * were measured and the floor is 17. The retained order is corroborated
 * independently of the search -- the target stores z, pad0C, alpha, pad0A in
 * that sequence, which is the contiguous block every 17-scoring order carries,
 * and one of objectId/size has to sit last. Nine orders reach 17; this is the
 * one closest to declaration order.
 *
 * 2026-09-11, lane f9-audit: 17 -> 0, byte-exact. The closure above held ONE
 * variable fixed while it varied the rest: the coordinate index was always
 * spelled as an integer add on a `(u32)` cast (or `i * 2` in the indexed
 * form, which is -4 bytes). The ring told the story: with objectId and size
 * stored first the target's coordinate shift takes the ring slot AFTER the one
 * the candidate gives it, so the target spends one pop that emits nothing
 * between the size store and the address. `&gOverlay60CoordsD8[i << 1]` is
 * that pop -- the doubled index folds into the one shift and still costs a
 * temp (L65's phantom, on a shift rather than a mask). With the ring phase
 * right, the named `inner` local takes v1 at save 20 / nocs 1 with no frame
 * growth, which is the "one more coloured pool web" the closure had correctly
 * seen and wrongly refuted: every refutation named `inner` while the phase was
 * still one behind, so the tail shifted and read as a 71-84 word regression.
 * The 0x68 frames it measured came from that same `(u32)` address spelling,
 * not from the sixth declaration. */
void func_overlay_060_F0000000_18B9DD8(void) {
    Overlay60Object **objects;
    Overlay60SpawnDesc desc;
    Overlay60Object *object;
    s16 *coordinate;
    Overlay60Inner *inner;
    s32 i;

    gOverlay60Data00[0] = SOURCE(u32, 0x44);
    gOverlay60Data00[1] = SOURCE(u32, 0x1C8);
    gOverlay60Data00[2] = SOURCE(u32, 0x1E4);
    gOverlay60Data00[3] = SOURCE(u32, 0x48);
    gOverlay60Data00[4] = SOURCE(u32, 0x40);
    gOverlay60Data00[5] = SOURCE(u32, 0x3C);
    gOverlay60Data00[6] = SOURCE(u32, 0x4C);
    gOverlay60Data00[7] = SOURCE(u32, 0x278);
    gOverlay60Data00[8] = SOURCE(u32, 0x1CC);
    gOverlay60Data00[9] = SOURCE(u32, 0x1D0);

    gOverlay60Data28[0] = SOURCE(u32, 0x00);
    gOverlay60Data28[1] = SOURCE(u32, 0x04);
    gOverlay60Data28[2] = SOURCE(u32, 0x08);
    gOverlay60Data28[3] = SOURCE(u32, 0x0C);
    gOverlay60Data38[0] = SOURCE(u32, 0x1C);
    gOverlay60Data38[1] = SOURCE(u32, 0x18);

    gOverlay60Data40 = 0;
    gOverlay60Data44 = 0;
    gOverlay60Data48 = 0x9B;
    gOverlay60Data4C = -1;

    for (i = 0, objects = &gOverlay60ObjectC8; i < 4; i++, objects++) {
        desc.objectId = 0x138;
        desc.size = 0xE;
        coordinate = &gOverlay60CoordsD8[i << 1];
        desc.x = coordinate[0];
        desc.y = coordinate[1];
        desc.z = 0;
        desc.pad0C = 0;
        desc.alpha = 0x80;
        desc.pad0A = 0;
        object = func_8000590C(&desc, 0);
        *objects = object;
        inner = *object->inner;
        inner->mode = 2;
        func_8005AD64(*objects, 0, 0, 0.0f);
    }

    loadFrontEndList(gOverlay60Bss00);
    gOverlay60Bss20 = *(u32 *)(D_800D31C8 + 0x1E4);
    gOverlay60Bss40 = *(u32 *)(D_800D31C8 + 0x0C);
    gOverlay60Bss60 = *(u32 *)(D_800D31C8 + 0x144);
    gOverlay60Bss80 = *(u32 *)(D_800D31C8 + 0x1EC);

    i = frontGetScreenMode();
    if (i == 1) {
        gOverlay60Data58 = 4;
        gOverlay60ObjectC8->x = (f32)gOverlay60CoordE8;
        gOverlay60ObjectC8->y = (f32)gOverlay60CoordEA;
        gOverlay60ObjectCC->x = (f32)gOverlay60CoordEC;
        gOverlay60ObjectCCY->y = (f32)gOverlay60CoordEE;
        gOverlay60ObjectD0->x = (f32)gOverlay60CoordF0;
        gOverlay60ObjectD0Y->y = (f32)gOverlay60CoordF2;
        gOverlay60ObjectD4->x = (f32)gOverlay60CoordF4;
        gOverlay60ObjectD4Y->y = (f32)gOverlay60CoordF6;
    } else {
        gOverlay60Data58 = 0;
    }

    animseqStartPath(((u8 *)&gOverlay60Data58)[3]);
    object = func_800508B4(((u8 *)&gOverlay60Data58)[3]);
    if (object != 0) {
        object->flags |= 2;
    }
    gOverlay60Bss174 = 0;
}

