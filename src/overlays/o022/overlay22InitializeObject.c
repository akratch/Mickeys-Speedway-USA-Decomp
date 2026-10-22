#include "PR/ultratypes.h"

#define FIELD(base, type, off) (*(type *)((u8 *)(base) + (off)))

typedef struct Vec3f {
    f32 x;
    f32 y;
    f32 z;
} Vec3f;

extern f32 gOverlay22DistanceReloc;
extern u8 D_A7C[];
extern void *D_0[];
extern s32 D_30;
extern void mathOneFloatRPY(void *, Vec3f *);
extern void trackMakePolylist(s32, Vec3f *, Vec3f *, f32 *, void *, s32);
extern s32 func_80010900(Vec3f *, Vec3f *, f32, void *, void *);
extern void partUpdateTriggers(void *, s32);
extern void func_80006EA0(void *);
extern s32 func_8000FAE0(f32, f32, f32);
extern void func_overlay_022_F0000D30_1878E38Reloc(void *, s32, s32 *);

/* One aggregate, not two pointer locals: cfe copy-propagates a declared
 * pointer whose value is a parameter offset or a symbol address, which leaves
 * both values in pooled cells below the declared block, while an array's
 * elements are not promoted and keep the two homes retail spends at the bottom
 * of the block.  Three details are load-bearing and each was measured: the
 * first call reads the address expression rather than collide[1], because
 * retail uses the register it has just computed; collide[1] is stored
 * immediately before that call, because retail's store sits at the end of the
 * argument setup; and the plane table is read through collide[0] at BOTH
 * calls, or it is rematerialised at the second. */
void func_overlay_022_F0000000_1878108(void *object, void *init) {
    void *contact;
    Vec3f position;
    Vec3f offset;
    f32 distance;
    s32 keep;
    void *collide[2];

    contact = FIELD(object, void *, 0x64);
    FIELD(contact, f32, 4) = (f32)FIELD(init, s32, 0xC);
    FIELD(contact, s8, 0) = (s8)FIELD(init, s16, 0xA);
    FIELD(contact, void *, 0x38) = FIELD(init, void *, 0x14);
    FIELD(contact, s16, 2) = 1;
    position.x = FIELD(object, f32, 0xC);
    position.y = FIELD(object, f32, 0x10);
    position.z = FIELD(object, f32, 0x14);
    offset.x = 0.0f;
    offset.y = 0.0f;
    offset.z = FIELD(init, f32, 0x10);
    mathOneFloatRPY(FIELD(contact, void *, 0x38), &offset);
    FIELD(object, f32, 0xC) += offset.x;
    FIELD(object, f32, 0x10) += offset.y;
    FIELD(object, f32, 0x14) += offset.z;
    keep = 1;
    distance = gOverlay22DistanceReloc;
    collide[1] = &FIELD(object, Vec3f, 0xC);
    trackMakePolylist(1, &position, &FIELD(object, Vec3f, 0xC), &distance, 0, 1);
    collide[0] = D_A7C;
    if ((func_80010900(&position, collide[1], distance, object, collide[0]) != 0) &&
        (FIELD(contact, u8, 1) & 4)) {
        FIELD(object, s32, 0x80) |= 2;
        partUpdateTriggers(object, 1);
        func_80006EA0(object);
        keep = 0;
    }
    if (keep != 0) {
        if (FIELD(contact, s8, 0) == 2) {
            if (!(FIELD(contact, u8, 1) & 2)) {
                FIELD(contact, u8, 1) = 0;
                position.x = FIELD(object, f32, 0xC);
                position.y = FIELD(object, f32, 0x10) - 5.0f;
                position.z = FIELD(object, f32, 0x14);
                if (func_80010900(collide[1], &position, distance, object,
                                  collide[0]) == 0) {
                    FIELD(contact, s8, 0) = 1;
                    FIELD(object, s16, 4) = 0x4000;
                }
            }
        } else {
            FIELD(object, f32, 0x24) = -FIELD(contact, f32, 4);
            mathOneFloatRPY(FIELD(contact, void *, 0x38),
                          (Vec3f *)((u8 *)object + 0x1C));
            FIELD(object, s16, 4) = 0x4000;
        }
        FIELD(object, s16, 0x2E) = func_8000FAE0(
            FIELD(object, f32, 0xC), FIELD(object, f32, 0x10),
            FIELD(object, f32, 0x14));
        FIELD(FIELD(object, void *, 0x48), void *, 0x70) =
            FIELD(contact, void *, 0x38);
        FIELD(FIELD(object, void *, 0x48), u16, 6) |= 2;
        FIELD(contact, f32, 0x2C) = FIELD(object, f32, 0xC);
        FIELD(contact, f32, 0x30) = FIELD(object, f32, 0x10);
        FIELD(contact, f32, 0x34) = FIELD(object, f32, 0x14);
        FIELD(contact, s16, 2) = 0;
        if (D_30 < 12) {
            D_0[D_30++] = object;
            return;
        }
        func_overlay_022_F0000D30_1878E38Reloc(D_0[0], 5, &D_30);
        D_0[D_30++] = object;
    }
}
