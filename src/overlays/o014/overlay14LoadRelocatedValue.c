#include "PR/ultratypes.h"

extern u32 *gOverlay14OffsetsF4;
extern void *mmAlloc(s32 size, s32 tag);
extern void piRomLoadSection(s32 source, void *destination, s32 start, s32 size);

/* Tier A: linked ROM identity with untouched IDO instruction output.
 * The result carrier keeps both relocation operations in one register.
 * Read through cursor before advancing it; command retains the store address.
 * Declare the two spilled offsets first so their homes bound the frame. */
void *overlay14LoadRelocatedValue(s32 index, s32 source) {
    s32 start;
    s32 size;
    u8 *buffer;
    u8 *cursor;
    u32 value;

    start = gOverlay14OffsetsF4[index];
    size = gOverlay14OffsetsF4[index + 1] - start;
    buffer = mmAlloc(size, 0x85);
    if (buffer != NULL) {
        piRomLoadSection(source, buffer, start, size);
        cursor = buffer;
        while (*cursor != 0) {
            u8 *command;

            command = cursor;
            switch (*cursor) {
                case 1:
                    cursor += 4;
                    break;
                case 2:
                    value =
                        ((*(u32 *)cursor & 0xFFFFFF) | 0x80000000);
                    cursor += 4;
                    value += (u32)buffer;
                    *(u32 *)command = value;
                    *command = 2;
                    break;
                case 3:
                    value =
                        ((*(u32 *)cursor & 0xFFFFFF) | 0x80000000);
                    cursor += 4;
                    value += (u32)buffer;
                    *(u32 *)command = value;
                    *command = 3;
                    break;
                case 4:
                    value =
                        ((*(u32 *)cursor & 0xFFFFFF) | 0x80000000);
                    cursor += 4;
                    value += (u32)buffer;
                    *(u32 *)command = value;
                    *command = 4;
                    break;
                case 5:
                    value =
                        ((*(u32 *)cursor & 0xFFFFFF) | 0x80000000);
                    cursor += 4;
                    value += (u32)buffer;
                    *(u32 *)command = value;
                    *command = 5;
                    break;
                case 6:
                    cursor += 8;
                    break;
                case 7:
                    cursor += 8;
                    break;
            }
        }
    }
    return buffer;
}
