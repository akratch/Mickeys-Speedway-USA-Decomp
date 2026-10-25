#include "overlays/overlay_047.h"

/* The module owns its initialized tables, compiler pool and 0x340-byte BSS.
 * The flag loop in the release routine is unrolled by IDO; it uses one
 * resident status byte, replacing the earlier same-address extern aliases.
 */
/* Initialized values retained from the existing C owner; fields follow their consumers. */
static Overlay47Vertex gOverlay47InitializedData[14] = {
    { -23, -23, 0, 255, 255, 255, 255 },
    { -11, -23, 0, 255, 255, 255, 255 },
    { -23, -20, 0, 255, 255, 255, 255 },
    { -20, -20, 0, 255, 255, 255, 255 },
    { -11, -20, 0, 255, 255, 255, 255 },
    { -23, -11, 0, 255, 255, 255, 255 },
    { -20, -11, 0, 255, 255, 255, 255 },
    { 23, -23, 0, 255, 255, 255, 255 },
    { 11, -23, 0, 255, 255, 255, 255 },
    { 23, -20, 0, 255, 255, 255, 255 },
    { 20, -20, 0, 255, 255, 255, 255 },
    { 11, -20, 0, 255, 255, 255, 255 },
    { 23, -11, 0, 255, 255, 255, 255 },
    { 20, -11, 0, 255, 255, 255, 255 },
};
static Overlay47Vertex ov47Data_8C[14] = {
    { -23, 23, 0, 255, 255, 255, 255 },
    { -11, 23, 0, 255, 255, 255, 255 },
    { -23, 20, 0, 255, 255, 255, 255 },
    { -20, 20, 0, 255, 255, 255, 255 },
    { -11, 20, 0, 255, 255, 255, 255 },
    { -23, 11, 0, 255, 255, 255, 255 },
    { -20, 11, 0, 255, 255, 255, 255 },
    { 23, 23, 0, 255, 255, 255, 255 },
    { 11, 23, 0, 255, 255, 255, 255 },
    { 23, 20, 0, 255, 255, 255, 255 },
    { 20, 20, 0, 255, 255, 255, 255 },
    { 11, 20, 0, 255, 255, 255, 255 },
    { 23, 11, 0, 255, 255, 255, 255 },
    { 20, 11, 0, 255, 255, 255, 255 },
};
static Overlay47Triangle ov47Data_118[8] = {
    { 64, 0, 1, 2, 0, 0, 0, 0, 0, 0 },
    { 64, 1, 4, 2, 0, 0, 0, 0, 0, 0 },
    { 64, 2, 3, 5, 0, 0, 0, 0, 0, 0 },
    { 64, 3, 6, 5, 0, 0, 0, 0, 0, 0 },
    { 64, 8, 7, 9, 0, 0, 0, 0, 0, 0 },
    { 64, 8, 9, 11, 0, 0, 0, 0, 0, 0 },
    { 64, 10, 9, 12, 0, 0, 0, 0, 0, 0 },
    { 64, 10, 12, 13, 0, 0, 0, 0, 0, 0 },
};
static Overlay47Vertex ov47Data_198[4] = {
    { -21, -20, 0, 255, 255, 255, 255 },
    { 21, -20, 0, 255, 255, 255, 255 },
    { -21, 20, 0, 255, 255, 255, 255 },
    { 21, 20, 0, 255, 255, 255, 255 },
};
static Overlay47Triangle ov47Data_1C0[2] = {
    { 64, 0, 1, 2, 0, 0, 0, 0, 0, 0 },
    { 64, 1, 3, 2, 0, 0, 0, 0, 0, 0 },
};
static s16 sUnreferencedGeometry[36] = {
    -8, -24, 0, -1, -1, -8, -24, 0, -1, -1, -8, -32,
    0, -1, -1, -8, -32, 0, -1, -1, 16384, 258, 0, 0,
    512, 0, 0, 512, 16385, 770, 512, 0, 512, 512, 0, 512
};
static Overlay47VertexSet6 ov47Data_228 = { {
    { -160, -40, 0, 0, 0, 0, 0 },
    { 160, -40, 0, 0, 0, 0, 0 },
    { -160, -48, 0, 0, 0, 0, 92 },
    { 160, -48, 0, 0, 0, 0, 92 },
    { -160, -120, 0, 0, 0, 0, 92 },
    { 160, -120, 0, 0, 0, 0, 92 },
}, { 0 } };
static Overlay47Triangle ov47Data_268[4] = {
    { 64, 0, 1, 2, 0, 0, 0, 0, 0, 0 },
    { 64, 1, 3, 2, 0, 0, 0, 0, 0, 0 },
    { 64, 2, 3, 4, 0, 0, 0, 0, 0, 0 },
    { 64, 3, 5, 4, 0, 0, 0, 0, 0, 0 },
};
static Overlay47Command ov47Data_2A8[11] = {
    {{ 0xe7000000, 0x0 }},
    {{ 0xba001402, 0x0 }},
    {{ 0xba001001, 0x0 }},
    {{ 0xba000e02, 0x0 }},
    {{ 0xba001102, 0x0 }},
    {{ 0xba001301, 0x0 }},
    {{ 0xba000c02, 0x2000 }},
    {{ 0xba000903, 0xc00 }},
    {{ 0xb9000002, 0x0 }},
    {{ 0xb900031d, 0x442048 }},
    {{ 0xb8000000, 0x0 }},
};
static Overlay47Command ov47Data_300[11] = {
    {{ 0xe7000000, 0x0 }},
    {{ 0xba001402, 0x0 }},
    {{ 0xba001001, 0x0 }},
    {{ 0xba000e02, 0x0 }},
    {{ 0xba001102, 0x0 }},
    {{ 0xba001301, 0x0 }},
    {{ 0xba000c02, 0x2000 }},
    {{ 0xba000903, 0xc00 }},
    {{ 0xb9000002, 0x0 }},
    {{ 0xb900031d, 0x5041c8 }},
    {{ 0xb8000000, 0x0 }},
};
static s16 D_358[12] = {
    3, 4, 5, 11, 124, 12, 13, 14, 15, 16, -1, 0
};
static s16 D_370[2] = {
    6, -1
};
static s16 D_374[12] = {
    64, 68, 136, 137, 66, 67, 175, 176, 177, 178, -1, 0
};
static void * D_38C[10] = { NULL };
static s16 sScreenPositions[6][2] = {
    { 76, 131 },
    { 140, 131 },
    { 204, 131 },
    { 76, 179 },
    { 140, 179 },
    { 204, 179 },
};
static s8 ov47Data_3CC[4][4] = {
    { 3, -1, -1, -1 },
    { 2, 4, -1, -1 },
    { 1, 3, 5, -1 },
    { 0, 2, 4, 6 },
};
static s32 ov47Data_3DC[5] = { 0xff0000ff, 0xffff, 0xff00ff, 0xffff00ff, 0xa0a000 };
static f32 ov47Data_3F0[6] = { 0.0105F, 0.0065F, 0.015F, 0.0105F, 0.0105F, 0.022F };
static f32 sIconScales[10] = { 1.16F, 1.16F, 1.16F, 1.16F, 1.16F, 1.16F, 1.0F, 0.9F, 0.8F, 0.7F };
static s8 sLayoutPrefix[20] = { 16, 0, -8, 0, 12, 0, -16, 0, 4, 0, -12, 0, 16, 0, -8, 0, 12, 0, -16, 0 };
static s8 sIconStartX[10] = { -100, -100, -100, -100, -100, -100, -106, -106, -109, -112 };
static s8 sIconStepX[10] = { 40, 40, 40, 40, 40, 40, 35, 30, 27, 25 };
static s8 sIconStartY[10] = { -74, -74, -74, -74, -74, -74, -71, -68, -67, -64 };
static s8 sIconStepY[10] = { 0, 0, 0, 0, 0, 0, -10, -15, -18, -23 };
static s8 ov47Data_474[12] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
static s8 ov47Data_480[12] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
static u16 ov47Data_48C[10] = {
    83, 148, 201, 498, 364, 430, 778, 682, 770, 602
};
static u16 ov47Data_4A0[10] = {
    85, 149, 202, 499, 365, 432, 779, 775, 784, 603
};
static u16 ov47Data_4B4[10] = {
    86, 150, 203, 500, 366, 433, 728, 683, 771, 604
};
static s8 ov47Data_4C8[10][4] = {
    { 3, 3, 3, 3 },
    { 2, 4, 4, 2 },
    { 3, 3, 3, 3 },
    { 2, 4, 4, 2 },
    { 4, 2, 2, 4 },
    { 4, 2, 2, 4 },
    { 3, 3, 2, 4 },
    { 2, 4, 5, 2 },
    { 2, 5, 4, 3 },
    { 5, 1, 1, 5 },
};
/* The renderer walks nodes until the null-texture terminator. */
static Overlay47TextureNode sTextureNodes[2] = {
    { NULL, NULL, 0, 0, 0 },
    { NULL, NULL, 0, 0, 0 },
};
static s16 ov47Data_510[10] = {
    64, 68, 136, 137, 66, 67, 175, 176, 177, 178
};
static s8 ov47Data_524[12] = {
    0, 3, 4, 5, 1, 2, 6, 7, 8, 9, 0, 0
};
static s32 ov47Data_530[4] = {
    -35, -36, 3, 2
};
static f32 ov47Data_540 = 0.0F;
static f32 ov47Data_544 = 0.04F;
static s32 ov47Data_548 = 13;
static f32 ov47Data_54C = 0.0F;
static s32 ov47Data_550 = 0;
static s32 ov47Data_554 = 4;

static s8 ov47Bss_0;
static Overlay47Icon ov47Bss_8[10];
static Overlay47TextureNode ov47Bss_1C0[5];
static Overlay47Position ov47Bss_210[7];
static Overlay47Position ov47Bss_280[7];
static Overlay47Position ov47Bss_2F0;
static s8 ov47Bss_300[10];
static s8 ov47Bss_30A;
static s8 ov47Bss_30B;
static void *ov47Bss_30C;
static void *ov47Bss_310;
static Overlay45ResourceDescriptor *ov47Bss_314;
static Overlay45ResourceDescriptor *ov47Bss_318;
static Overlay45ResourceDescriptor *ov47Bss_31C;
static Overlay45ResourceDescriptor *ov47Bss_320;
static s32 ov47Bss_324;
static s32 ov47Bss_328[4];
static s32 ov47Bss_338;


static void func_overlay_047_F0002D10_1893B28(Overlay47Player *player);

#define O47_COMMAND(a, b) { \
    Overlay47Command *command = D_800D3140++; \
    command->words.w0 = (u32)(a); \
    command->words.w1 = (u32)(b); \
}
/* N64 ABI representation contract: pointer and signed word are 32 bits.
 * The sign-bit XOR is modulo-32 physical encoding, without signed overflow.
 * This helper is used consistently for relocated module geometry. Matrix
 * packets use GBI definitions and the existing command lvalue view with
 * unsigned runtime-pointer conversion. */
typedef char Overlay47AddressWordWidth[(sizeof(void *) == 4 && sizeof(s32) == 4) ? 1 : -1];
#define O47_GEOMETRY_ADDRESS(p) ((geometryAddress.pointer=(const void *)(p)), (u32)(geometryAddress.bits ^ (-2147483647-1)))
#define O47_MATRIX(p) O47_COMMAND((G_MTX << 24) | ((G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH) << 16) | sizeof(Mtx), ((u32)(p) + 0x80000000U))
#define O47_VERTICES(p, n) \
    O47_COMMAND(0x04000000 | (((((n) << 3) | (O47_GEOMETRY_ADDRESS(p) & 6)) & 0xFF) << 16) | ((n) * 10 + 8), O47_GEOMETRY_ADDRESS(p))
#define O47_RECTANGLE(x, y) \
    O47_COMMAND(0xF6000000 | ((((x) + 6) & 0x3FF) << 14) | ((((y) + 6) & 0x3FF) << 2), \
                (((x) & 0x3FF) << 14) | (((y) & 0x3FF) << 2))

void func_overlay_047_F0000000_1890E18(void) {
    Overlay47SpawnPacket packet;
    Overlay47PathObject *pathObject;
    Overlay47PathNode *node;
    s32 i;
    s32 j;
    s32 x;
    s32 y;
    s32 yStep;
    Overlay47Player *entry;
    Overlay47Icon *choice;

    loadFrontEndList(D_358);
    setupFrontEndList(D_370);
    fontUseFont(3);
    fontColour(0xFF, 0xFF, 0xFF, 0, 0xFF);
    ov47Bss_30C = overlay45CreateDescriptor(D_8007C0B8[98], 0xA0, 0x18, 4);
    ov47Bss_314 = overlay45CreateDescriptor("OK", 0xA0, 0x110, 4);
    ov47Bss_318 = overlay45CreateDescriptor(D_8007C0B8[99], 0xA0, -0x28, 4);
    ov47Bss_31C = overlay45CreateDescriptor(D_8007C0B8[22], 0x78, 0x104, 4);
    ov47Bss_320 = overlay45CreateDescriptor(D_8007C0B8[23], 0xC8, 0x104, 4);
    joyResetMap();

    packet.type = 0xE;
    packet.x = 0;
    packet.y = 0;
    packet.z = 0;
    packet.state = 0;
    packet.byte0B = 0x4B;
    packet.byte0A = 0;
    i = 0;
    while (D_374[i] != -1) {
        packet.selector = D_374[i];
        D_38C[i] = func_8000590C(&packet, 0);
        i++;
    }

    for (i = 0; i < 4; i++) {
        ov47Bss_1C0[i].alternate = 0;
        ov47Bss_1C0[i].packedOffset = 0;
        ov47Bss_1C0[i].y = 0x96;
    }
    ov47Bss_1C0[4].texture = 0;
    ov47Bss_1C0[4].alternate = 0;

    pathObject = func_800508B4(0x48);
    if (pathObject != NULL) {
        node = pathObject->path;
        for (i = 0; i < 7; i++) {
            ov47Bss_210[i].x = node->x;
            ov47Bss_210[i].y = node->y;
            ov47Bss_210[i].z = node->z;
            ov47Bss_210[i].rotation = node->selector;
            node = node->next;
        }
    }

    pathObject = func_800508B4(0x52);
    if (pathObject != NULL) {
        node = pathObject->path;
        for (i = 0; i < 7; i++) {
            ov47Bss_280[i].x = node->x;
            ov47Bss_280[i].y = node->y;
            ov47Bss_280[i].z = node->z;
            ov47Bss_280[i].rotation = node->selector;
            node = node->next;
        }
    }

    pathObject = func_800508B4(0x49);
    if (pathObject != NULL) {
        node = pathObject->path;
        ov47Bss_2F0.x = node->x;
        ov47Bss_2F0.y = node->y;
        ov47Bss_2F0.z = node->z;
        ov47Bss_2F0.rotation = node->selector;
    }

    for (i = 0; i < 4; i++) {
        D_800D3058[i].actor = NULL;
        D_800D3058[i].sound = 0;
        D_800D3058[i].x = ov47Bss_2F0.x;
        D_800D3058[i].y = ov47Bss_2F0.y;
        D_800D3058[i].z = ov47Bss_2F0.z;
        D_800D3058[i].rotation = ov47Bss_2F0.rotation;
        D_800D3058[i].ready = 0;
        D_800D3058[i].leaving = 0;
    }

    for (i = 0; i < 10; i++) {
        ov47Bss_300[i] = 0;
    }

    for (i = 0; i < 6; i++) {
        ov47Bss_8[i].selector = i;
    }

    ov47Bss_0 = 6;
    if (D_800D3128.unlocked) {
        ov47Bss_300[6] = 0;
        ov47Bss_8[ov47Bss_0].selector = 6.0f;
        ov47Bss_0++;
    } else {
        ov47Bss_300[6] = 1;
    }

    if (((D_800D3128.configuration[0] & 7) >= 3) &&
        ((D_800D3128.configuration[1] & 7) >= 3) &&
        ((D_800D3128.configuration[2] & 7) >= 3)) {
        ov47Bss_300[7] = 0;
        ov47Bss_8[ov47Bss_0].selector = 7.0f;
        ov47Bss_0++;
    } else {
        ov47Bss_300[7] = 1;
    }

    if ((((D_800D3128.configuration[0] & 0x38) >> 3) >= 3) &&
        (((D_800D3128.configuration[1] & 0x38) >> 3) >= 3) &&
        (((D_800D3128.configuration[2] & 0x38) >> 3) >= 3)) {
        ov47Bss_300[8] = 0;
        ov47Bss_8[ov47Bss_0].selector = 8.0f;
        ov47Bss_0++;
    } else {
        ov47Bss_300[8] = 1;
    }

    if ((((D_800D3128.configuration[0] & 0x1C0) >> 6) == 4) &&
        (((D_800D3128.configuration[1] & 0x1C0) >> 6) == 4) &&
        (((D_800D3128.configuration[2] & 0x1C0) >> 6) == 4) &&
        (((D_800D3128.configuration[3] & 0x1C0) >> 6) == 4) &&
        (((D_800D3128.field10 & 0x1C0) >> 6) == 4)) {
        ov47Bss_300[9] = 0;
        ov47Bss_8[ov47Bss_0].selector = 9.0f;
        ov47Bss_0++;
    } else {
        ov47Bss_300[9] = 1;
    }

    if (frontGetMode() == 9) {
        D_8007C1A0 = 1;
        ov47Bss_30A = 1;
        for (i = 0; i < 4; i++) {
            D_800D3058[i].selector = 0;
            D_800D3058[i].active = 0;
        }
        func_8003A754();
        D_800D3058[0].active = 1;
        ov47Bss_300[0] = 1;
        D_8007BF68 = 1;
    } else {
        ov47Bss_30A = D_8007C1A0;
        for (i = 0; i < 4; i++) {
            if (D_800D3058[i].active != 0) {
                ov47Bss_300[D_800D3058[i].selector] = 1;
            }
        }
    }

    animseqStartPath(0x4A);
    pathObject = func_800508B4(0x4A);
    if (pathObject != NULL) {
        pathObject->flags |= 2;
    }
    amSndPlay(0x19, NULL);

    for (i = 0; i < 4; i++) {
        D_800D3058[i].ready = 0;
    }
    ov47Bss_310 = 0;
    ov47Bss_324 = 0;
    sTextureNodes[0].texture = D_800D31C8[4];

    x = sIconStartX[ov47Bss_0 - 1];
    y = sIconStartY[ov47Bss_0 - 1];
    yStep = sIconStepY[ov47Bss_0 - 1];
    for (i = 0; i < 10; i++) {
        ov47Bss_8[i].z = 0.0f;
        ov47Bss_8[i].rotationX = 0;
        ov47Bss_8[i].x = x;
        ov47Bss_8[i].rotationY = 0;
        ov47Bss_8[i].rotationZ = 0;
        ov47Bss_8[i].scale = sIconScales[ov47Bss_0 - 1];
        x += sIconStepX[ov47Bss_0 - 1];
        ov47Bss_8[i].y = y;
        y += yStep;
        yStep = -yStep;
    }

    entry = D_800D3058;
    for (i = 0; i < 4; i++, entry++) {
        for (choice = ov47Bss_8, j = 0; j < ov47Bss_0; j++, choice++) {
            /* `(*p).f`, not `p->f`: the arrow form swaps the c.eq.s operands. */
            if (((*choice).selector == (*entry).selector) && ((*entry).active != 0)) {
                ov47Bss_328[i] = ((s32)choice->x + 0xA0) << 4;
            }
        }
    }
    ov47Bss_338 = 0;
}

void func_overlay_047_F00009D0_18917E8(void) {
    Overlay47Player *entry;
    void **slot;
    s32 i;

    overlay45ReleaseDescriptor(ov47Bss_30C);
    overlay45ReleaseDescriptor(ov47Bss_314);
    overlay45ReleaseDescriptor(ov47Bss_318);
    overlay45ReleaseDescriptor(ov47Bss_31C);
    overlay45ReleaseDescriptor(ov47Bss_320);

    entry = D_800D3058; do {
        if (entry->actor != NULL) {
            func_80006EA0(entry->actor);
            entry->actor = NULL;
        }
        entry++;
    } while (entry < &D_800D3058[4]);

    slot = D_38C; do {
        if (*slot != NULL) {
            func_80006EA0(*slot);
            *slot = NULL;
        }
        slot++;
    } while (slot < &D_38C[10]);

    freeFrontEndList((s16 *) D_358);
    D_800D31B0 = 0;
    for (i = 0; i < 4; i++) {
        if (D_800D3058[i].ready != 0) {
            D_800D31B0 |= 1 << i;
        }
    }
}

void func_overlay_047_F0000B30_1891948(s32 updateRate) {
    s32 j;
    s32 controller;
    s32 i;
    s32 slot;
    s32 selected;
    s32 unready;
    s32 colourIndex;
    s32 activeCount;
    s32 speed;
    s32 allReady;
    s32 count;
    s32 start;
    s32 rotationStep;
    s32 labelCount;

    s32 barX;
    s32 stat;
    s32 bars, field;
    union { const void *pointer; s32 bits; } geometryAddress;

    s32 x, y;
    s32 red, green, blue;
    s32 showMode;
    f32 movement;
    s32 back;
    s32 padD; /* unused: holds the frame */
    f32 oldFrame;
    f32 frame; /* unused: holds the frame (17 words without it) */
    Overlay47Player *p2;
    f32 scale;
    f32 rate;
    f32 oldFov;
    MtxF localMatrix;
    MtxF cameraMatrix;
    MtxF resultMatrix;
    Mtx *savedMatrix;
    f32 oldSelector;
    f32 **height;
    Overlay47Player *player;
    Overlay47TextureScroll *scroll;
    Overlay47Icon *icon;
    /* The label column has a local of its own, declared last: its cell is
     * the one between the declared homes and the spill temps (lane v-4
     * had it as an unused pad, 67 -> 59), and as its own web it takes s6
     * where the ready flag keeps fp (lane x-047). */
    s32 labelX;

    rate = updateRate;
    ov47Bss_30B = 0;
    showMode = 0;
    allReady = 1;
    start = 0;
    activeCount = 0;
    back = 0;
    /* Both player-array loops outside the main loop are indexed by
     * controller and spelled `(D_800D3058 + controller)->`: the two loops'
     * cursors are one strength-reduced fp web (lane v-4, 54 -> 47; on the
     * if/else body subscripts in both loops are 20 against 11, either loop
     * alone 382 to 394 at +4, lane x-047). */
    for (controller = 0; controller < 4; controller++) {
        if ((D_800D3058 + controller)->active != 0) {
            /* Counted after the ready test (lane x-047, 4 -> 2: the inner
             * branch keeps its nop rather than a likely copy of the step). */
            if ((D_800D3058 + controller)->ready == 0) {
                allReady = 0;
                ov47Bss_338 = 0;
            }
            activeCount++;
        }
    }
    slot = 0;
    if (allReady && ((activeCount == 1) || (activeCount == 4))) {
        ov47Bss_338 = 1;
    }
    player = D_800D3058;
    for (controller = 0; controller < 4; controller++) {
        /* The icon cursor is reset per controller (lane v-4, 59 -> 54; still
         * 11 against 16 on the if/else body, lane x-047). */
        icon = ov47Bss_8;
        if (!player->active) {
            if ((joyGetPressed(controller) & 0x9000) && !player->leaving &&
                !ov47Bss_324 && !start) {
                player->active = 1;
                player->x = ov47Bss_2F0.x;
                player->y = ov47Bss_2F0.y;
                player->z = ov47Bss_2F0.z;
                player->rotation = ov47Bss_2F0.rotation;
                while (ov47Bss_300[player->selector]) {
                    player->selector++;
                    if (player->selector >= 10) player->selector = 0;
                }
                allReady = 0;
                ov47Bss_300[player->selector] = 1;
                D_8007C1A0++;
                ov47Bss_30A++;
                amSndPlay(12, NULL);
                amSndPlay(25, NULL);
                if (player->actor != NULL) {
                    if (player->actor->kind != ov47Data_510[player->selector]) {
                        func_80006EA0(player->actor);
                        func_overlay_047_F0002D10_1893B28(player);
                    } else {
                        func_8005AD64(player->actor, 1, -1, 0.0f);
                    }
                }
                /* One counted loop on i, as shipped (the target's s4 is
                 * i here and in the later loops); the inherited j copy
                 * and its > 0 guard were m2c artefacts. 455 -> 335
                 * aligned at size 0 (lane r-4). */
                for (i = 0; i < ov47Bss_0; i++) {
                    if ((f32)player->selector == icon->selector) {
                        ov47Bss_328[controller] = ((s32)icon->x + 160) << 4;
                    }
                    icon++;
                }
                icon = ov47Bss_8;
            }
        } else {
            if (allReady && !ov47Bss_338) {
                showMode = 1;
                if (D_800D3190[controller] < -16) {
                    if (D_8007BF74 != 0) {
                        amSndPlay(14, NULL);
                    } else {
                        ov47Bss_30B = 1;
                        D_8007BF74 = 1;
                    }
                }
                if (D_800D3190[controller] >= 17) {
                    if (D_8007BF74 == 0) {
                        amSndPlay(14, NULL);
                    } else {
                        ov47Bss_30B = 1;
                        D_8007BF74 = 0;
                    }
                }
            }
            if ((joyGetPressed(controller) & 0x9000) && !player->leaving &&
                player->actor != NULL && !ov47Bss_324) {
                if (allReady) {
                    if (!ov47Bss_338) {
                        ov47Bss_338 = 1;
                        amSndPlay(12, NULL);
                    } else {
                        start = 1;
                        if (player->sound != NULL) amSndStop(player->sound);
                        amSndPlay(ov47Data_4B4[ov47Data_524[player->selector]], &player->sound);
                    }
                } else if (!player->ready) {
                    player->ready = 1;
                    if (player->sound != NULL) amSndStop(player->sound);
                    amSndPlay(ov47Data_48C[ov47Data_524[player->selector]], &player->sound);
                }
                func_8005AD64(player->actor, 2, -1, 0.0f);
            } else if ((joyGetPressed(controller) & 0x4000) && !player->leaving) {
                if (player->actor != NULL && !ov47Bss_324 && !start) {
                    if (player->ready) {
                        player->ready = 0;
                        allReady = 0;
                        func_8005AD64(player->actor, 0, -1, 0.0f);
                        if (player->sound != NULL) amSndStop(player->sound);
                        amSndPlay(ov47Data_4A0[ov47Data_524[player->selector]], &player->sound);
                    } else if (D_8007C1A0 >= 2) {
                        player->active = 0;
                        ov47Bss_300[player->selector] = 0;
                        if (player->actor != NULL) {
                            player->leaving = 1;
                            func_8005AD64(player->actor, 5, -1, 0.0f);
                            amSndPlay(24, NULL);
                        }
                        D_8007C1A0--;
                    } else {
                        back = 1;
                    }
                }
            }
        }
        if (player->active && player->actor == NULL) {
            func_overlay_047_F0002D10_1893B28(player);
        }
        if (player->actor != NULL) {
            speed = updateRate * 10000;
            camProjectPoint(player->actor->x, player->actor->y, player->actor->z,
                         &player->screenX, &player->screenY, 1);
            if (!player->leaving && !player->active) {
                movement = 0.015f;
                player->targetX = ov47Bss_2F0.x;
                player->targetY = ov47Bss_2F0.y;
                player->targetZ = ov47Bss_2F0.z;
                player->targetRotation = ov47Bss_2F0.rotation;
            } else {
                movement = 0.125f;
                if (player->ready) {
                    player->targetX = ov47Bss_280[ov47Data_3CC[ov47Bss_30A - 1][slot]].x;
                    player->targetY = ov47Bss_280[ov47Data_3CC[ov47Bss_30A - 1][slot]].y;
                    player->targetZ = ov47Bss_280[ov47Data_3CC[ov47Bss_30A - 1][slot]].z;
                    player->targetRotation = ov47Bss_280[ov47Data_3CC[ov47Bss_30A - 1][slot]].rotation;
                    slot++;
                } else {
                    player->targetX = ov47Bss_210[ov47Data_3CC[ov47Bss_30A - 1][slot]].x;
                    player->targetY = ov47Bss_210[ov47Data_3CC[ov47Bss_30A - 1][slot]].y;
                    player->targetZ = ov47Bss_210[ov47Data_3CC[ov47Bss_30A - 1][slot]].z;
                    player->targetRotation = ov47Bss_210[ov47Data_3CC[ov47Bss_30A - 1][slot]].rotation;
                    slot++;
                }
            }
            for (i = 0; i < updateRate; i++) {
                player->x += (player->targetX - player->x) * movement;
                player->y += (player->targetY - player->y) * movement;
                player->z += (player->targetZ - player->z) * movement;
                player->rotation += (player->targetRotation - player->rotation) * movement;
                player->actor->x = player->x;
                player->actor->y = player->y;
                player->actor->z = player->z;
                player->actor->rotation = player->rotation;
            }
            if (func_8001398C(player->x, player->z, 0x1000, &height)) {
                player->y = **height;
            }
            oldFrame = player->actor->frameValue;
            func_8005ABA8(player->actor, ov47Data_3F0[(s8)player->actor->frame], rate);
            scroll = player->actor->scroll;
            for (field = 0; field < 4; field++) {
                scroll->offsets[field] += speed;
            }
            if (!player->ready && !player->leaving && player->active && !start) {
                if (D_800D3190[controller] < -16 && !ov47Bss_324) {
                    ov47Bss_300[player->selector] = 0;
                    player->selector--;
                    if (player->selector < 0) player->selector = 9;
                    while (ov47Bss_300[player->selector]) {
                        player->selector--;
                        if (player->selector < 0) player->selector = 9;
                    }
                    ov47Bss_300[player->selector] = 1;
                    ov47Bss_30B = 1;
                    func_80006EA0(player->actor);
                    player->actor = NULL;
                    func_overlay_047_F0002D10_1893B28(player);
                } else if (D_800D3190[controller] >= 17 && !ov47Bss_324) {
                    ov47Bss_300[player->selector] = 0;
                    player->selector++;
                    if (player->selector >= 10) player->selector = 0;
                    while (ov47Bss_300[player->selector]) {
                        player->selector++;
                        if (player->selector >= 10) player->selector = 0;
                    }
                    ov47Bss_300[player->selector] = 1;
                    ov47Bss_30B = 1;
                    func_80006EA0(player->actor);
                    player->actor = NULL;
                    func_overlay_047_F0002D10_1893B28(player);
                }
            }
            if (player->actor != NULL) {
                player->actor->trigger = 0;
                switch (player->actor->frame) {
                    case 1:
                        if (0.3f <= player->actor->frameValue && player->actor->frameValue < 0.65f) {
                            player->actor->trigger = 15;
                        }
                        if (oldFrame < 0.3f && 0.3f <= player->actor->frameValue) {
                            amSndPlay(26, NULL);
                        }
                        if (player->actor->frameValue == 1.0f) {
                            func_8005AD64(player->actor, 0, -1, 0.0f);
                            player->idleTimer = mathRnd(30, 300);
                        }
                        break;
                    case 3:
                    case 4:
                        if (player->actor->frameValue == 1.0f) {
                            func_8005AD64(player->actor, 0, 0, 0.0f);
                            player->idleTimer = mathRnd(30, 300);
                        }
                        break;
                    case 0:
                        if (player->idleTimer > 0) {
                            player->idleTimer -= updateRate;
                        } else if (player->actor->frameValue < 0.02f) {
                            func_8005AD64(player->actor, mathRnd(3, 4), -1, 0.0f);
                        }
                        break;
                    case 5:
                        player->actor->trigger = 15;
                        if (0.8f < player->actor->frameValue && player->leaving) {
                            player->leaving = 0;
                            slot--;
                            ov47Bss_30A--;
                        }
                        break;
                    case 2:
                        if (player->actor->frameValue == 1.0f) {
                            func_8005AD64(player->actor, 0, -1, 0.0f);
                            player->idleTimer = mathRnd(30, 300);
                        }
                        break;
                }
                if (!player->leaving && !player->active) {
                    if ((player->x - ov47Bss_2F0.x) * (player->x - ov47Bss_2F0.x) +
                        (player->z - ov47Bss_2F0.z) * (player->z - ov47Bss_2F0.z) < 1000.0f) {
                        func_80006EA0(player->actor);
                        player->actor = NULL;
                    }
                }
                if (player->actor != NULL) partUpdateTriggers(player->actor, updateRate);
            }
        }
        /* The step is the body's last statement, not the for-step (lane
         * x-047, 2 -> 0: the next statement's high half then schedules
         * first in the latch, as shipped). */
        player++;
    }
    ov47Data_550 += ov47Data_554 * updateRate;
    if (ov47Data_550 < 0) {
        ov47Data_550 = -ov47Data_550;
        ov47Data_554 = -ov47Data_554;
    } else if (ov47Data_550 >= 256) {
        ov47Data_550 = 510 - ov47Data_550;
        ov47Data_554 = -ov47Data_554;
    }
    if (showMode) {
        overlay45ConfigureLayout(ov47Bss_318, 160, 160, 0x104);
        overlay45ConfigureLayout(ov47Bss_31C, 120, 190, 0x104);
        overlay45ConfigureLayout(ov47Bss_320, 200, 190, 0x104);
        if (D_8007BF74) {
            overlay45SetField22(ov47Bss_31C, ov47Data_550);
            overlay45SetField22(ov47Bss_320, 0);
        } else {
            overlay45SetField22(ov47Bss_31C, 0);
            overlay45SetField22(ov47Bss_320, ov47Data_550);
        }
    } else {
        overlay45ConfigureLayout(ov47Bss_318, 160, -40, 0x104);
        overlay45ConfigureLayout(ov47Bss_31C, 120, 260, 0x104);
        overlay45ConfigureLayout(ov47Bss_320, 200, 260, 0x104);
    }
    oldFov = camGetFOV();
    camSetFOV(52.0f, 1);
    camSetView(&D_800D3140, NULL);
    camStandardOrtho(&D_800D3140, &D_800D3144);
    texDPTextureX(&D_800D3140, NULL, 16, 0);
    O47_COMMAND(0xFA000000, -1);
    O47_VERTICES(ov47Data_228.vertices, 6);
    O47_COMMAND(0x05300040, O47_GEOMETRY_ADDRESS(ov47Data_268));
    O47_COMMAND(0xE7000000, 0);
    if (allReady && ov47Bss_338) {
        overlay45ConfigureLayout(ov47Bss_314, 160, 178, 0x104);
    } else {
        overlay45ConfigureLayout(ov47Bss_314, 160, 272, 0x104);
    }
    for (controller = 0; controller < ov47Bss_0; controller++, icon++) {
        /* The marker loop and the rotation loop below count on i, the scan
         * loops' index, and the colour index is its own local (lane u-5:
         * 289 -> 245 aligned, replacing the colour-index-in-i stand-in). */
        colourIndex = 4;
        unready = 0;
        selected = -1;
        count = 0;
        /* The cursor starts in the for-init (lane x-047, 6 -> 4: the table
         * base is hoisted ahead of it, as shipped). */
        for (i = 0, p2 = D_800D3058; i < 4; i++, p2++) {
            if ((f32)p2->selector == icon->selector && p2->active) {
                colourIndex = i;
                if (!p2->ready) unready = 1;
                for (j = 0; j < updateRate; j++) {
                    ov47Bss_328[i] +=
                        ((((s32)icon->x + 160) << 4) - ov47Bss_328[i]) >> 2;
                }
                ov47Bss_1C0[count].texture = D_800D31C8[i + 13];
                ov47Bss_1C0[count].x = 312 - (ov47Bss_328[i] >> 4);
                count++;
                selected = i;
            }
        }
        ov47Bss_1C0[count].texture = NULL;
        func_8002FB34(&D_800D3140, ov47Bss_1C0, 320.0f, 0, 1.0f, 1.0f, -2, 0x1003);
        func_8002A82C(localMatrix);
        matrixTranslate(icon->x, icon->y, 0.0f, localMatrix);
        scale = icon->scale / 1.16f;
        matrixScale(scale, scale, scale, localMatrix);
        func_8002A604(icon->rotationZ, localMatrix);
        camCopyOrthoMatrix(cameraMatrix);
        mtxf_mul(localMatrix, cameraMatrix, resultMatrix);
        mtxf_to_mtx(resultMatrix, D_800D3144);
        savedMatrix = D_800D3144;
        /* Word 0 first: the w0 store separates the savedMatrix load from the
         * operand's reload, so unready keeps its register (lane k-8). */
        O47_MATRIX(D_800D3144);
        D_800D3144++;
        /* The icon packet and the blend share one set of channels, each
         * masked at the load: the masks spend the ring draws the target
         * spends and as1 folds them into the packet's (lane v-4). */
        red = (ov47Data_3DC[colourIndex] >> 24) & 255;
        green = (ov47Data_3DC[colourIndex] >> 16) & 255;
        blue = (ov47Data_3DC[colourIndex] >> 8) & 255;
        O47_COMMAND(0x06000000, ov47Data_300);
        O47_COMMAND(0xFA000000, ((red & 255) << 24) | ((green & 255) << 16) | ((blue & 255) << 8) | 255);
        O47_COMMAND(0xFCFFFFFF, 0xFFFDF6FB);
        O47_VERTICES(ov47Data_198, 4);
        O47_COMMAND(0x05100020, O47_GEOMETRY_ADDRESS(ov47Data_1C0));
        camStandardOrtho(&D_800D3140, &D_800D3144);
        texDPInit(&D_800D3140);
        O47_COMMAND(0xFA000000, -1);
        camDo2DSprite(&D_800D3140, &D_800D3144, &D_800D3148, icon, D_800D31C8[11], 0, 255);
        if (selected != -1 && !unready) {
            oldSelector = icon->selector;
            icon->selector = selected;
            camDo2DSprite(&D_800D3140, &D_800D3144, &D_800D3148, icon, D_800D31C8[124], 0, 255);
            icon->selector = oldSelector;
        }
        O47_MATRIX(savedMatrix);
        if (selected != -1 && !D_800D3058[selected].ready) {
            O47_COMMAND(0x06000000, ov47Data_2A8);
            red = (ov47Data_3DC[selected] >> 24) & 255;
            green = (ov47Data_3DC[selected] >> 16) & 255;
            blue = (ov47Data_3DC[selected] >> 8) & 255;
            red = red + (255 - red) * ov47Data_540;
            green = green + (255 - green) * ov47Data_540;
            blue = blue + (255 - blue) * ov47Data_540;
            O47_COMMAND(0xFA000000, ((red & 255) << 24) | ((green & 255) << 16) | ((blue & 255) << 8) | 255);
            O47_COMMAND(0xFCFFFFFF, 0xFFFDF6FB);
            O47_VERTICES(gOverlay47InitializedData, 14);
            O47_COMMAND(0x05710080, O47_GEOMETRY_ADDRESS(ov47Data_118));
            O47_VERTICES(ov47Data_8C, 14);
            O47_COMMAND(0x05710080, O47_GEOMETRY_ADDRESS(ov47Data_118));
        }
        for (i = 0; i < updateRate; i++) {
            if (icon->rotationZ < 0) {
                rotationStep = (icon->rotationZ + 0x1000) / 5;
            } else {
                rotationStep = (0x1000 - icon->rotationZ) / 5;
            }
            if (rotationStep < 51) rotationStep = 50;
            if (!ov47Data_474[controller]) {
                icon->rotationZ += rotationStep;
                if (selected == -1 && icon->rotationZ >= 0 &&
                    icon->rotationZ - rotationStep <= 0) {
                    icon->rotationZ = 0;
                }
                if (icon->rotationZ > 0x1000) {
                    icon->rotationZ = 0x2000 - icon->rotationZ;
                    ov47Data_474[controller] = 1;
                }
            } else {
                if (icon->rotationZ < 0) {
                    ov47Data_480[controller]++;
                    if (ov47Data_480[controller] >= 101) ov47Data_480[controller] = 100;
                } else {
                    ov47Data_480[controller]--;
                    if (ov47Data_480[controller] < 0) ov47Data_480[controller] = 0;
                }
                icon->rotationZ -= rotationStep;
                if (selected == -1 && icon->rotationZ <= 0 &&
                    icon->rotationZ + rotationStep >= 0) {
                    icon->rotationZ = 0;
                }
                if (icon->rotationZ < -0x1000) {
                    icon->rotationZ = -0x2000 - icon->rotationZ;
                    ov47Data_474[controller] = 0;
                }
            }
        }
    }
    fontColour(255, 255, 255, 255, 255);
    fontUseFont(2);
    labelCount = 0;
    for (controller = 0; controller != 4; controller++) {
        if ((D_800D3058 + controller)->active && (D_800D3058 + controller)->actor != NULL) {
            if (ov47Bss_30A == 4) {
                labelX = ov47Data_530[controller] + (D_800D3058 + controller)->screenX;
            } else {
                labelX = (D_800D3058 + controller)->screenX - 20.0f;
            }
            /* The row counter is j, as is the marker easing loop's, and the
             * bar row is `stat` (lane k-8 had the rows on `selected`; j in
             * both loops is 245 -> 237 aligned, lane u-5). */
            stat = 116;
            for (j = 0; j != 4; j++) {
                barX = labelX;
                if (labelCount < 4) {
                    if (ov47Bss_30A != 4) {
                        fontPrintXY(&D_800D3140, labelX - 6, stat + 2, D_8007C0B8[145 + j], 9);
                    } else {
                        fontPrintXY(&D_800D3140, 160, stat + 2, D_8007C0B8[145 + j], 12);
                    }
                    labelCount++;
                }
                O47_COMMAND(0xE7000000, 0);
                O47_COMMAND(0xEF002C0F, 0x00504340);
                O47_COMMAND(0xB6000000, 0x00010001);
                O47_COMMAND(0xFCFFFFFF, 0xFFFDF6FB);
                bars = ov47Data_4C8[ov47Data_524[(D_800D3058 + controller)->selector]][j];
                O47_COMMAND(0xFA000000, ov47Data_3DC[controller]);
                while (bars--) {
                    O47_RECTANGLE(barX, stat);
                    barX += 8;
                }
                O47_COMMAND(0xE7000000, 0);
                O47_COMMAND(0xFA000000, (ov47Data_3DC[controller] & ~0xFF) | 0x40);
                bars = 5 - ov47Data_4C8[ov47Data_524[(D_800D3058 + controller)->selector]][j];
                while (bars--) {
                    O47_RECTANGLE(barX, stat);
                    barX += 8;
                }
                stat += 8;
            }
        }
    }
    overlay45ReadPair((Overlay45PairOwner *)ov47Bss_314, &x, &y, 0);
    func_8002FB34(&D_800D3140, &sTextureNodes[0], (f32)(x - 32), (f32)(y - 6), 1.0f, 1.0f, -2, 3);
    texAnimateTexSprite(sTextureNodes[0].texture, &ov47Data_548, 2, &ov47Data_54C, updateRate);
    sTextureNodes[0].packedOffset = (s32)(ov47Data_54C * 65536.0f);
    if (start) {
        mainChangeLevel(12, 0, 0, 10, 1, 0);
        ov47Bss_324 = 1;
    } else if (back) {
        amSndPlay(13, NULL);
        mainChangeLevel(12, 0, 0, 4, 1, 0);
        ov47Bss_324 = 1;
    }
    camSetFOV(oldFov, 1);
    camSetView(&D_800D3140, NULL);
    ov47Data_540 += ov47Data_544 * rate;
    /* Both arms update the factor before its step (lane x-047, 11 -> 6:
     * the step negated first put the 2.0f literal on f16). */
    if (ov47Data_540 > 1.0f) {
        ov47Data_540 = 2.0f - ov47Data_540;
        ov47Data_544 = -ov47Data_544;
    } else if (ov47Data_540 < 0.0f) {
        ov47Data_540 = -ov47Data_540;
        ov47Data_544 = -ov47Data_544;
    }
    if (ov47Bss_30B) {
        if (ov47Bss_310 != NULL) amSndStop(ov47Bss_310);
        amSndPlay(15, &ov47Bss_310);
    }
}

static void func_overlay_047_F0002D10_1893B28(Overlay47Player *object) {
    Overlay47SpawnPacket packet;

    packet.selector = ov47Data_510[object->selector];
    packet.type = 0xE;
    packet.x = (s16)object->x;
    packet.y = (s16)object->y;
    packet.z = (s16)object->z;
    packet.state = (s8)object->rotation;
    packet.byte0B = 0x4B;
    packet.byte0A = 0;
    object->actor = func_8000590C(&packet, 1);
    if (object->actor != NULL) {
        func_80005768(object->actor);
        object->actor->field3C = 0;
        func_8005AD64(object->actor, 1, -1, 0.0f);
        object->actor->rotation = object->rotation;
    }
}
