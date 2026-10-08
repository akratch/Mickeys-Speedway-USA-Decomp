#include "PR/ultratypes.h"

/* IDO 5.3 -O2 -mips2 reproduces 0x110 bytes; -mips1 grows them to 0x11C. */

typedef struct Overlay61RecordHeader {
    u32 magic;
    u8 pad4[4];
    s8 field8;
    u8 pad9;
    s16 fieldA;
    s16 fieldC;
} Overlay61RecordHeader;

extern u8 gOverlay61RecordPathReloc[];
extern s32 overlay61StorageReadyReloc(void);
extern s32 overlay61OpenReloc(s32 device, void *path, s32 arg2, void **handle);
extern s32 overlay61GetSizeReloc(s32 device, void *handle, s32 *size);
extern void *overlay61AllocReloc(s32 size, s32 tag);
extern s32 overlay61ReadReloc(s32 device, void *handle, void *data, s32 size);
extern void overlay61FreeReloc(void *data);
extern void overlay61CloseReloc(s32 device);

s32 overlay61ReadCharacter(
    s32 device, s32 slot, s32 *out0, s32 *out1, s32 *out2) {
    s32 result;
    s32 size;
    void *handle;
    Overlay61RecordHeader *buffer;
    volatile s32 padding[2];

    result = overlay61StorageReadyReloc();
    if (result == 0) {
        result = overlay61OpenReloc(
            device, gOverlay61RecordPathReloc, slot, &handle);
        if (result == 0) {
            result = overlay61GetSizeReloc(device, handle, &size);
            if (size >= 0x40) {
                buffer = overlay61AllocReloc(0x40, 0x85);
                if (buffer != 0) {
                    result = overlay61ReadReloc(device, handle, buffer, 0x40);
                    if (result == 0) {
                        if (buffer->magic == 0x43484152) {
                            *out0 = buffer->field8;
                            *out1 = buffer->fieldA;
                            *out2 = buffer->fieldC;
                        } else {
                            result = 5;
                        }
                    }
                    overlay61FreeReloc(buffer);
                }
            } else {
                result = 6;
            }
        }
    }
    overlay61CloseReloc(device);
    return result;
}
