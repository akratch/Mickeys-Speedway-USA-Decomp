#include "PR/ultratypes.h"

typedef struct Vec3f {
    f32 x;
    f32 y;
    f32 z;
} Vec3f;

typedef struct Overlay29SourceState {
    u8 pad000[0x1A8];
    u16 flags;
    u8 pad1AA[0x1D2];
    s16 recordIndex;
    u8 selector0;
    u8 selector1;
    u8 selector2;
} Overlay29SourceState;

typedef struct Overlay29Entity {
    u8 pad00[6];
    u16 flags;
    u8 pad08[0x68];
    void *owner;
} Overlay29Entity;

typedef struct Overlay29Contact {
    void *owner;
    s16 recordIndex;
    u8 pad06[2];
    u8 selector0;
    u8 selector1;
    u8 selector2;
    u8 pad0B[0xF];
    u8 flags;
} Overlay29Contact;

typedef struct Overlay29Object {
    s16 angle0;
    s16 angle1;
    s16 angle2;
    u8 pad06[6];
    Vec3f position;
    u8 pad18[0x30];
    Overlay29Entity *entity;
    u8 pad4C[0x18];
    Overlay29Contact *contact;
    u8 pad68[0x18];
    s32 active;
} Overlay29Object;

typedef struct Overlay29Init {
    u8 pad00[0xA];
    s16 angle0;
    s16 angle1;
    u8 pad0E[2];
    Overlay29Object *object;
} Overlay29Init;

extern u8 func_overlay_029_F0000EE0_187E190[];
extern void mathOneFloatRPY(void *, Vec3f *);
extern void trackMakePolylist(s32, Vec3f *, Vec3f *, f32 *, void *, s32);
extern s32 func_80010900(Vec3f *, Vec3f *, f32, void *, void *);
/* This call site reaches the overlay's own +0x10C4 entry through the module's
 * ROM relocation surface: the record at +0xF0 is a SYMBOL R_MIPS_26 naming
 * overlay 29 offset 0x10C4, so the shipped word is the 0xF0000000 addend and
 * the C must not name the in-module definition. Its two neighbours at +0x16C
 * and +0x178 are JUMP records against the same symbol index and do store
 * offset >> 2, so they keep the bare names -- same module is not the test,
 * the record's operation is. See CLAUDE.md. */
extern void func_overlay_029_F00010C4_187E374Reloc(void *, s32);
extern void func_overlay_029_F00001C4_187D474(void *);
extern void func_overlay_029_F000023C_187D4EC(void *, void *);

/* Matched 2026-09-12, lane p9-tight. Fifteen words to zero, two edits.
 *
 * 1. The `object` field was `volatile`, which gives the reloads AND as1's
 *    scheduling edges [L144]; the target needs only the reloads. Taking the
 *    address at each of the four reads keeps all four loads -- the count is
 *    observable, the ROM has four -- and drops the edges, and the entry block
 *    then collapses from a mixed structure/schedule/register residual (15) to
 *    a pure schedule one (9, category schedule-only, the instruction multiset
 *    already exact). Every prior probe on this function held the `volatile`
 *    fixed and asked which statement order would reach the target's stream;
 *    with the edges in place no order can, which is why eight positions of the
 *    source assignment measured 15 to 22 and never below.
 * 2. With the edges gone the order is the lever, and it is the natural one:
 *    the two pointer producers first, then position x/y/z, then offset x/y/z.
 *    Measured over all 1,120 interleavings that keep position and offset each
 *    in x/y/z order, with the contact and source assignments at every
 *    position: exactly two score 0, this one and the mirror with `source`
 *    ahead of `contact`; the next best is 2 and the median is 12.
 *
 * The prior closure's named decision variable -- uopt's web-creation order in
 * the entry block -- was the wrong level. The block is one basic block, so
 * every one of those forms produced the same webs; what differed was as1's
 * scheduling, and the volatile edges were holding it fixed.
 */
void func_overlay_029_F000042C_187D6DC(Overlay29Object *object,
                                       Overlay29Init *init) {
    Overlay29Contact *contact;
    Overlay29SourceState *source;
    Vec3f position;
    Vec3f offset;
    f32 distance;

    contact = object->contact;
    source = (Overlay29SourceState *)(*(Overlay29Object **)&init->object)->contact;
    position.x = object->position.x;
    position.y = object->position.y;
    position.z = object->position.z;
    offset.x = 0.0f;
    offset.y = 0.0f;
    offset.z = -60.0f;

    mathOneFloatRPY((*(Overlay29Object **)&init->object), &offset);
    object->position.x += offset.x;
    object->position.y += offset.y;
    object->position.z += offset.z;

    distance = 8.0f;
    trackMakePolylist(1, &position, &object->position, &distance, NULL, 0);
    if (func_80010900(&position, &object->position, distance, object, func_overlay_029_F0000EE0_187E190) != 0) {
        func_overlay_029_F00010C4_187E374Reloc(object, 5);
        return;
    }

    object->entity->flags |= 2;
    object->entity->owner = (*(Overlay29Object **)&init->object);
    object->angle0 = init->angle0;
    object->angle1 = init->angle1;
    object->angle2 = 0;
    object->active = 1;
    contact->owner = (*(Overlay29Object **)&init->object);
    contact->recordIndex = source->recordIndex;
    contact->selector0 = source->selector0;
    contact->selector1 = source->selector1;
    contact->selector2 = source->selector2;
    contact->flags = source->flags & 8;
    func_overlay_029_F00001C4_187D474(object);
    func_overlay_029_F000023C_187D4EC(object, contact);
}
