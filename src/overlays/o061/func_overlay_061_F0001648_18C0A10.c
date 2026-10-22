#include "PR/ultratypes.h"

extern u8 gOverlay61SavePathReloc[];
extern s32 packOpen(s32);
extern s32 packOpenFile(s32, void *, s32 *, void **);
extern s32 packFileSize(s32, void *, s32 *);
extern s32 *mmAlloc(s32, s32);
extern s32 packReadFile(s32, void *, void *, s32);
extern s32 overlay68PayloadLimit(s32);
extern void mmFree(void *);
extern void packClose(s32);

/*
 * Exact 92-word match. The one-iteration do/while grouping is semantically
 * inert but preserves IDO's shipped caller-saved allocation; track a cleaner
 * spelling in the cleanup queue.
 */
s32 func_overlay_061_F0001648_18C0A10(s32 *output, s32 device, s32 *slot) {
    s32 copySize;
    s32 result;
    s32 size;
    void *handle;
    s32 *buffer;
    s32 *src;
    s32 *dst;

    result = packOpen(device);
    if (result == 0) {
        result = packOpenFile(
            device, gOverlay61SavePathReloc, slot, &handle);
        if (result == 0) {
            result = packFileSize(device, handle, &size);
            if (size >= 4) {
                buffer = mmAlloc(size, 0x85);
                if (buffer != 0) {
                    result = packReadFile(device, handle, buffer, size);
                    if (result == 0) {
                        if (buffer[0] == 0x43484152) {
                            copySize = size - 4;
                            do {
                                if (overlay68PayloadLimit(copySize) < copySize) {
                                    copySize = overlay68PayloadLimit(copySize);
                                }
                                copySize = (copySize + 3) >> 2;
                                src = buffer + 1;
                                dst = output;
                                while (copySize--) {
                                    *dst++ = *src++;
                                }
                            } while (0);
                            output[3] = (s32)(output + 4);
                        } else {
                            result = 5;
                        }
                    }
                    mmFree(buffer);
                }
            } else {
                result = 6;
            }
        }
    }
    packClose(device);
    return result;
}
