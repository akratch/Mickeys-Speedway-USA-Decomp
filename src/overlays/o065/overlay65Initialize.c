#include "PR/ultratypes.h"

typedef struct Overlay65Record {
    u8 pad00[0x1B];
    u8 flag1B;
    u8 pad1C[0x1B];
    u8 flag37;
    u8 pad38[0x1B];
    u8 flag53;
    u8 pad54[0x1B];
    u8 flag6F;
} Overlay65Record;

extern u8 gOverlay65InitFlag0;
extern u8 gOverlay65InitFlag1;
extern Overlay65Record gOverlay65Records[];
extern Overlay65Record gOverlay65RecordsEnd;
extern void *gOverlay65Handle;
extern u8 *gOverlay65Buffers[2];

void *overlay65AllocateReloc();
void overlay65ResetSlots(void);

/* DKR v77/v80 and JFG contain no exact donor for this fixed-pool initializer. */
void overlay65Initialize(void) {
    u8 *end;
    u8 *record;
    u8 *buffer;
    u8 *bufferEnd;

    gOverlay65InitFlag0 = 0;
    gOverlay65InitFlag1 = 0; record = (u8 *)gOverlay65Records; end = (u8 *)&gOverlay65RecordsEnd;
    do {
        record[0x37] = 0;
        record[0x6F] = (record[0x53] = 0);
        record[0x1B] = 0;
        record += 0x70;
    } while (record != end);
    gOverlay65Handle = overlay65AllocateReloc(0x91);
    overlay65ResetSlots();
    buffer = overlay65AllocateReloc(0x7530, 0x87);
    bufferEnd = buffer + 0x3A98;
    gOverlay65Buffers[0] = buffer;
    gOverlay65Buffers[1] = bufferEnd;
}
