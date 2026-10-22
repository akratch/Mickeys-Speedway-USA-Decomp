#include "PR/ultratypes.h"
#include "n_audio/mbi.h"

typedef struct Overlay47ResourceRecord {
    s16 value[8];
} Overlay47ResourceRecord;

typedef struct Overlay47StateData {
    s32 minimumX;
    s32 minimumY;
    s32 mode;
    s32 count;
    s32 reserved0;
    f32 scale;
    s32 selector;
    s32 reserved1;
    s32 reserved2;
    s32 modeValue;
    s32 reserved3;
    s32 reserved4;
} Overlay47StateData;

typedef struct Overlay47ResourceTail {
    char status[4];
    f32 allocationScale;
    f32 movementScale;
    u32 resourceOffset[6];
    f32 fadeScale;
    f32 fadeStep;
    f32 colorScale;
    f32 depthScale;
    u32 reserved[3];
} Overlay47ResourceTail;

typedef struct Overlay47InitializedData {
    Overlay47ResourceRecord records[42];
    Gfx setup[23];
    u16 resourceSelectors[28];
    u8 selectorPadding[0x20];
    u16 colorPairs[24];
    u8 colorFlags[16];
    f32 motionScales[16];
    u8 colorRamps[0x50];
    u16 glyphMetrics[56];
    u8 metricPadding[0x20];
    u16 textureSelectors[8];
    u8 selectorOrder[16];
    Overlay47StateData state;
    Overlay47ResourceTail tail;
} Overlay47InitializedData;

#define O47_GFX(w0, w1) {{ w0, w1 }}

/* Overlay 47's resource tables are owned by the release TU in ROM order. */
Overlay47InitializedData gOverlay47InitializedData = {
    {
        {{ -23, -23, 0, -1, -1, -11, -23, 0 }},
        {{ -1, -1, -23, -20, 0, -1, -1, -20 }},
        {{ -20, 0, -1, -1, -11, -20, 0, -1 }},
        {{ -1, -23, -11, 0, -1, -1, -20, -11 }},
        {{ 0, -1, -1, 23, -23, 0, -1, -1 }},
        {{ 11, -23, 0, -1, -1, 23, -20, 0 }},
        {{ -1, -1, 20, -20, 0, -1, -1, 11 }},
        {{ -20, 0, -1, -1, 23, -11, 0, -1 }},
        {{ -1, 20, -11, 0, -1, -1, -23, 23 }},
        {{ 0, -1, -1, -11, 23, 0, -1, -1 }},
        {{ -23, 20, 0, -1, -1, -20, 20, 0 }},
        {{ -1, -1, -11, 20, 0, -1, -1, -23 }},
        {{ 11, 0, -1, -1, -20, 11, 0, -1 }},
        {{ -1, 23, 23, 0, -1, -1, 11, 23 }},
        {{ 0, -1, -1, 23, 20, 0, -1, -1 }},
        {{ 20, 20, 0, -1, -1, 11, 20, 0 }},
        {{ -1, -1, 23, 11, 0, -1, -1, 20 }},
        {{ 11, 0, -1, -1, 16384, 258, 0, 0 }},
        {{ 0, 0, 0, 0, 16385, 1026, 0, 0 }},
        {{ 0, 0, 0, 0, 16386, 773, 0, 0 }},
        {{ 0, 0, 0, 0, 16387, 1541, 0, 0 }},
        {{ 0, 0, 0, 0, 16392, 1801, 0, 0 }},
        {{ 0, 0, 0, 0, 16392, 2315, 0, 0 }},
        {{ 0, 0, 0, 0, 16394, 2316, 0, 0 }},
        {{ 0, 0, 0, 0, 16394, 3085, 0, 0 }},
        {{ 0, 0, 0, 0, -21, -20, 0, -1 }},
        {{ -1, 21, -20, 0, -1, -1, -21, 20 }},
        {{ 0, -1, -1, 21, 20, 0, -1, -1 }},
        {{ 16384, 258, 0, 0, 0, 0, 0, 0 }},
        {{ 16385, 770, 0, 0, 0, 0, 0, 0 }},
        {{ -8, -24, 0, -1, -1, -8, -24, 0 }},
        {{ -1, -1, -8, -32, 0, -1, -1, -8 }},
        {{ -32, 0, -1, -1, 16384, 258, 0, 0 }},
        {{ 512, 0, 0, 512, 16385, 770, 512, 0 }},
        {{ 512, 512, 0, 512, -160, -40, 0, 0 }},
        {{ 0, 160, -40, 0, 0, 0, -160, -48 }},
        {{ 0, 0, 92, 160, -48, 0, 0, 92 }},
        {{ -160, -120, 0, 0, 92, 160, -120, 0 }},
        {{ 0, 92, 0, 0, 16384, 258, 0, 0 }},
        {{ 0, 0, 0, 0, 16385, 770, 0, 0 }},
        {{ 0, 0, 0, 0, 16386, 772, 0, 0 }},
        {{ 0, 0, 0, 0, 16387, 1284, 0, 0 }},
    },
    {
        O47_GFX(0, 0),
        O47_GFX(0xE7000000, 0),
        O47_GFX(0xBA001402, 0),
        O47_GFX(0xBA001001, 0),
        O47_GFX(0xBA000E02, 0),
        O47_GFX(0xBA001102, 0),
        O47_GFX(0xBA001301, 0),
        O47_GFX(0xBA000C02, 0x00002000),
        O47_GFX(0xBA000903, 0x00000C00),
        O47_GFX(0xB9000002, 0),
        O47_GFX(0xB900031D, 0x00442048),
        O47_GFX(0xB8000000, 0),
        O47_GFX(0xE7000000, 0),
        O47_GFX(0xBA001402, 0),
        O47_GFX(0xBA001001, 0),
        O47_GFX(0xBA000E02, 0),
        O47_GFX(0xBA001102, 0),
        O47_GFX(0xBA001301, 0),
        O47_GFX(0xBA000C02, 0x00002000),
        O47_GFX(0xBA000903, 0x00000C00),
        O47_GFX(0xB9000002, 0),
        O47_GFX(0xB900031D, 0x005041C8),
        O47_GFX(0xB8000000, 0),
    },
    { 3, 4, 5, 11, 124, 12, 13, 14, 15, 16, -1, 0, 6, -1,
      64, 68, 136, 137, 66, 67, 175, 176, 177, 178, -1, 0, 0, 0 },
    { 0 },
    { 0, 0, 76, 131, 140, 131, 204, 131, 76, 179, 140, 179,
      204, 179, 1023, -1, 516, -1, 259, 1535, 2, 1030, -256, 255 },
    { 0, 0, 255, 255, 0, 255, 0, 255, 255, 255, 0, 255, 0, 160,
      160, 0 },
    { 0.0105F, 0.0065F, 0.015F, 0.0105F, 0.0105F, 0.022F,
      1.16F, 1.16F, 1.16F, 1.16F, 1.16F, 1.16F,
      1.0F, 0.9F, 0.8F, 0.7F },
    { 16, 0, 248, 0, 12, 0, 240, 0, 4, 0, 244, 0, 16, 0, 248, 0,
      12, 0, 240, 0, 156, 156, 156, 156, 156, 156, 150, 150,
      147, 144, 0, 0, 40, 40, 40, 40, 40, 40, 35, 30, 27, 25, 0, 0,
      182, 182, 182, 182, 182, 182, 185, 188, 189, 192, 0, 0, 0, 0,
      0, 0, 0, 0, 246, 241, 238, 233, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 83, 148, 201, 498, 364, 430, 778, 682,
      770, 602, 85, 149, 202, 499, 365, 432, 779, 775, 784, 603,
      86, 150, 203, 500, 366, 433, 728, 683, 771, 604, 771, 771,
      516, 1026, 771, 771, 516, 1026, 1026, 516, 1026, 516, 771, 516,
      516, 1282, 517, 1027, 1281, 261 },
    { 0 },
    { 64, 68, 136, 137, 66, 67, 175, 176 },
    { 0, 177, 0, 178, 0, 3, 4, 5, 1, 2, 6, 7, 8, 9, 0, 0 },
    { -35, -36, 3, 2, 0, 0.04F, 13, 0, 0, 4, 0, 0 },
    { "OK", 0.015F, 0.3F, { 6696, 6468, 6880, 6632, 6632, 6792 },
      0.65F, 0.02F, 0.8F, 1.16F, { 0, 0, 0 } },
};

#undef O47_GFX

typedef struct Overlay47Entry {
    u8 pad00[0x24];
    void *handle;
    u8 pad28[2];
    s8 flag;
    u8 pad2B[9];
} Overlay47Entry;

extern Overlay47Entry D_D0;
extern Overlay47Entry D_0_entries[];
extern void *D_30C;
extern void *D_314;
extern void *D_318;
extern void *D_31C;
extern void *D_320;
extern void *D_38C;
extern void *D_3B4;
extern u8 D_358[];
extern u8 D_status0;
extern u8 D_status1;
extern u8 D_status2;
extern u8 D_status3;
extern u8 D_status4;

extern void overlay45ReleaseDescriptor(void *descriptor);
extern void func_80006EA0(void *handle);
extern void freeFrontEndList(s16 *assetGroup);

/* The four flags are the same field in consecutive entries. Reading the first
 * status byte in every assignment keeps the source symmetric; IDO folds its
 * known-zero value in the first branch while retaining the temp-ring pop. */
void func_overlay_047_F00009D0_18917E8(void) {
    Overlay47Entry *entry;
    void **slot;

    overlay45ReleaseDescriptor(D_30C);
    overlay45ReleaseDescriptor(D_314);
    overlay45ReleaseDescriptor(D_318);
    overlay45ReleaseDescriptor(D_31C);
    overlay45ReleaseDescriptor(D_320);

    entry = D_0_entries; do {
        if (entry->handle != NULL) {
            func_80006EA0(entry->handle);
            entry->handle = NULL;
        }
        entry++;
    } while (entry < &D_D0);

    slot = &D_38C; do {
        if (*slot != NULL) {
            func_80006EA0(*slot);
            *slot = NULL;
        }
        slot++;
    } while (slot < &D_3B4);

    freeFrontEndList((s16 *) D_358);
    D_status0 = 0;
    if (D_0_entries[0].flag != 0) {
        D_status1 = D_status0 | 1;
    }
    if (D_0_entries[1].flag != 0) {
        D_status2 = D_status0 | 2;
    }
    if (D_0_entries[2].flag != 0) {
        D_status3 = D_status0 | 4;
    }
    if (D_0_entries[3].flag != 0) {
        D_status4 = D_status0 | 8;
    }
}
