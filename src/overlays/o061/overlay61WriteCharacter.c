#include "PR/ultratypes.h"

extern s32 overlay61RecordSize(void);
extern void *mmAlloc(s32 size, s32 tag);
extern s32 packWriteFile(
    s32 arg0, s32 arg1, void *format, s32 arg3, void *data, s32 size);
extern void mmFree(void *data);
extern u8 gOverlay61RecordFormatReloc[];

s32 overlay61WriteCharacter(s32 *source, s32 destination, s32 value) {
    s32 size;
    s32 words;
    s32 result;
    s32 *buffer;

    size = overlay61RecordSize() + 4;
    if (size % 32 != 0) {
        size = (size - (size % 32)) + 32;
    }

    buffer = mmAlloc(size, 0x85);
    if (buffer != 0) {
        s32 count;
        s32 *src;
        s32 *dst;

        buffer[0] = 0x43484152;
        words = (size - 1) >> 2;
        src = source;
        dst = buffer + 1;
        count = words - 1;
        if (words != 0) {
            do {
                *dst++ = *src++;
            } while (count--);
        }
        result = packWriteFile(
            destination, -1, gOverlay61RecordFormatReloc, value, buffer, size);
        mmFree(buffer);
    } else {
        result = 6;
    }
    return result;
}
