#include "PR/ultratypes.h"

typedef struct Overlay43RotationInput {
    s16 pad00;
    s16 angle;
} Overlay43RotationInput;

typedef struct Overlay43MotionOutput {
    f32 unk00;
    u8 pad04[0x0C];
    f32 unk10;
    f32 unk14;
    f32 unk18;
    u8 pad1C[0x0C];
    f32 unk28;
    u8 pad2C[0x18];
    s32 owner;
} Overlay43MotionOutput;

typedef struct Vec3f {
    f32 x;
    f32 y;
    f32 z;
} Vec3f;

extern void func_80029FE4(Overlay43RotationInput *input, Vec3f *direction);
extern void func_8002A82C(Overlay43MotionOutput *output);

/* Matched by writing the scale as the float literal it is. The shipped load
 * reads the overlay's own constant pool, so the two uses are one value the
 * compiler may keep across the intervening stores; read through a data
 * symbol it has to be carried in a local, and that carrier is what took the
 * divisor's register. The saved direction components are plain locals and
 * one unreferenced leading local supplies the frame cell the carrier held.
 */
void func_overlay_043_F00010A8_188B078(Overlay43RotationInput *input,
                                      s32 owner,
                                      Overlay43MotionOutput *output) {
    s32 pad;
    Vec3f direction;
    f32 x;
    f32 y;
    f32 z;

    if (input->angle < 0) {
        input->angle = 0;
    }
    input->angle -= 0x4000;
    input->angle >>= 1;
    input->angle += 0x4000;

    direction.x = 0.0f;
    direction.y = 0.0f;
    direction.z = -1.0f;
    func_80029FE4(input, &direction);
    x = direction.x;
    y = direction.y;
    z = direction.z;
    output->owner = owner;
    func_8002A82C(output);

    output->unk00 = 1.2f;
    output->unk10 = -(x / y);
    output->unk14 = 0.0f;
    output->unk18 = -(z / y);
    output->unk28 = 1.2f;
}
