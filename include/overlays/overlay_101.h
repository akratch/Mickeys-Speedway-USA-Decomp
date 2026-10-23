#ifndef OVERLAY_101_H
#define OVERLAY_101_H

#include "PR/ultratypes.h"
#include "n_audio/gbi.h"
#include "game/math.h"
/* Tier B: both resident texture renderers consume this exact 16-byte node
 * and stop at a null texture. Tags and fields match their resident contract. */
typedef struct RcpTextureInfo RcpTextureInfo;
typedef struct RcpTextureNode {
    RcpTextureInfo *texture;
    RcpTextureInfo *alternate;
    u32 packedOffset;
    s16 x;
    s16 y;
} RcpTextureNode;

/* Tier B: pooled entries share an output pointer and integer operation tag.
 * The allocator tests tag zero; schedulers set it before typed callbacks.
 * Concrete records appear together in the completed entry union below. */
typedef struct Overlay101Entry {
    void *output;
    s32 type;
    u8 pad08[0x28];
} Overlay101Entry;

typedef struct Overlay101Node {
    s32 value;
    struct Overlay101Node *next;
} Overlay101Node;

typedef struct Overlay101Output {
    u8 pad00[0xE];
    s16 x;
    s16 y;
} Overlay101Output;

typedef struct Overlay101TimedEntry {
    void *output;
    s32 type;
    s32 startX;
    s32 startY;
    u8 pad10[8];
    s32 endX;
    s32 endY;
    u8 pad20[8];
    s32 elapsed;
    s32 duration;
} Overlay101TimedEntry;

typedef struct Overlay101PairSlot {
    u8 pad00[0xE];
    s16 x;
    s16 y;
    u8 pad12[0xA];
} Overlay101PairSlot;

typedef struct Overlay101PairEntry {
    void *output;
    s32 type;
    s32 startX;
    s32 startY;
    u8 pad10[8];
    s32 endX;
    s32 endY;
    u8 pad20[8];
    s32 elapsed;
    s32 duration;
} Overlay101PairEntry;

typedef struct Overlay101Output12 {
    u8 pad00[0x12];
    s16 x;
    s16 y;
} Overlay101Output12;

typedef struct Overlay101TimedEntry12 {
    void *output;
    s32 type;
    s32 startX;
    s32 startY;
    u8 pad10[8];
    s32 endX;
    s32 endY;
    u8 pad20[8];
    s32 elapsed;
    s32 duration;
} Overlay101TimedEntry12;

typedef struct Overlay101PairSlot12 { u8 pad00[0x12]; s16 x; s16 y; u8 pad16[6]; } Overlay101PairSlot12;

typedef struct Overlay101PairEntry12 {
    void *output; s32 type; s32 startX; s32 startY;
    u8 pad10[8]; s32 endX; s32 endY; u8 pad20[8]; s32 elapsed; s32 duration;
} Overlay101PairEntry12;

typedef struct Overlay101Slot_ActivateSlot {
    u8 pad00[8];
    u8 state;
    u8 value;
    u8 pad0A[0x12];
} Overlay101Slot_ActivateSlot;

typedef struct Overlay101Slot_AdvanceSlot {
    u8 pad00[8];
    u8 state;
    u8 value;
    u8 pad0A[0x12];
} Overlay101Slot_AdvanceSlot;

/* DKR v77/v80 and JFG contain no exact donor for this slot-order update. */

typedef struct Overlay101Slot_PromoteSlot {
    u8 data[0x1C];
} Overlay101Slot_PromoteSlot;

typedef struct Overlay101ByteOutput17 { u8 pad00[0x17]; u8 value; } Overlay101ByteOutput17;

typedef struct Overlay101ByteEntry17_UpdateByte17 {
    void *output; s32 type; s32 start;
    u8 pad0C[0xC]; s32 end; u8 pad1C[0xC]; s32 elapsed; s32 duration;
} Overlay101ByteEntry17_UpdateByte17;

typedef struct Overlay101ByteSlot17 { u8 pad00[0x17]; u8 value; u8 pad18[4]; } Overlay101ByteSlot17;

typedef struct Overlay101ByteEntry17_ScheduleByte17 {
    void *output; s32 type; s32 start; u8 pad0C[0xC];
    s32 end; u8 pad1C[0xC]; s32 elapsed; s32 duration;
} Overlay101ByteEntry17_ScheduleByte17;

typedef struct Overlay101ByteOutput16 { u8 pad00[0x16]; u8 value; } Overlay101ByteOutput16;

typedef struct Overlay101ByteEntry16_UpdateByte16 {
    void *output; s32 type; s32 start;
    u8 pad0C[0xC]; s32 end; u8 pad1C[0xC]; s32 elapsed; s32 duration;
} Overlay101ByteEntry16_UpdateByte16;

typedef struct Overlay101ByteSlot16 { u8 pad00[0x16]; u8 value; u8 pad17[5]; } Overlay101ByteSlot16;

typedef struct Overlay101ByteEntry16_ScheduleByte16 {
    void *output; s32 type; s32 start; u8 pad0C[0xC];
    s32 end; u8 pad1C[0xC]; s32 elapsed; s32 duration;
} Overlay101ByteEntry16_ScheduleByte16;

typedef struct Overlay101Output8 { u8 pad00[8]; s16 x; s16 y; } Overlay101Output8;

typedef struct Overlay101TimedEntry8 {
    void *output; s32 type; s32 startX; s32 startY;
    u8 pad10[8]; s32 endX; s32 endY; u8 pad20[8];
    s32 elapsed; s32 duration;
} Overlay101TimedEntry8;

/* DKR v77/v80 and JFG have no exact donor for this linked-slot scheduler. */

typedef struct Overlay101LinkedSlot {
    u8 pad00[8];
    s16 x;
    s16 y;
    u8 pad0C[0x10];
} Overlay101LinkedSlot;

typedef struct Overlay101LinkedEntry {
    void *output;
    s32 type;
    s32 startX;
    s32 startY;
    u8 pad10[8];
    s32 endX;
    s32 endY;
    u8 pad20[8];
    s32 elapsed;
    s32 duration;
} Overlay101LinkedEntry;

/* DKR v77/v80 and JFG have no exact donor for this linked-slot scheduler. */

typedef struct Overlay101LinkedSlot2 {
    u8 pad00[8];
    s16 x;
    s16 y;
    u8 pad0C[0x10];
} Overlay101LinkedSlot2;

typedef struct Overlay101LinkedEntry2 {
    void *output;
    s32 type;
    s32 startX;
    s32 startY;
    u8 pad10[8];
    s32 endX;
    s32 endY;
    u8 pad20[8];
    s32 elapsed;
    s32 duration;
} Overlay101LinkedEntry2;

typedef struct Overlay101Float12Output {
    u8 pad00[0xC];
    f32 value;
} Overlay101Float12Output;

typedef struct Overlay101FloatEntry {
    void *output;
    s32 type;
    u8 pad08[8];
    f32 start;
    u8 pad14[0xC];
    f32 end;
    u8 pad24[4];
    s32 elapsed;
    s32 duration;
} Overlay101FloatEntry;

/* DKR v77/v80 and JFG have no exact donor for this linked float scheduler. */

typedef struct Overlay101LinkedFloatSlot {
    u8 pad00[0xC];
    f32 value;
    u8 pad10[0xC];
} Overlay101LinkedFloatSlot;

typedef struct Overlay101LinkedFloatEntry {
    void *output;
    s32 type;
    u8 pad08[8];
    f32 start;
    u8 pad14[0xC];
    f32 end;
    u8 pad24[4];
    s32 elapsed;
    s32 duration;
} Overlay101LinkedFloatEntry;

typedef struct Overlay101Delta16Output {
    u8 pad00[0x10];
    s16 value;
} Overlay101Delta16Output;

typedef struct Overlay101DeltaEntry {
    void *output;
    s32 type;
    s32 start;
    u8 pad0C[0xC];
    s32 delta;
    u8 pad1C[0xC];
    s32 elapsed;
    s32 duration;
} Overlay101DeltaEntry;

/* DKR v77/v80 and JFG have no exact donor for this scaled-value scheduler. */

typedef struct Overlay101LinkedScaledSlot {
    u8 pad00[0x10];
    s16 value;
    u8 pad12[0xA];
} Overlay101LinkedScaledSlot;

typedef struct Overlay101LinkedScaledEntry {
    void *output;
    s32 type;
    s32 start;
    u8 pad0C[0xC];
    s32 end;
    u8 pad1C[0xC];
    s32 elapsed;
    s32 duration;
} Overlay101LinkedScaledEntry;

typedef struct Overlay101Byte18Output {
    u8 pad00[0x12];
    u8 value;
} Overlay101Byte18Output;

typedef struct Overlay101Byte18Entry {
    void *output;
    s32 type;
    s32 start;
    u8 pad0C[0xC];
    s32 end;
    u8 pad1C[0xC];
    s32 elapsed;
    s32 duration;
} Overlay101Byte18Entry;

/* DKR v77/v80 and JFG have no exact donor for this linked byte scheduler. */

typedef struct Overlay101LinkedByteSlot {
    u8 pad00[0x12];
    u8 value;
    u8 pad13[9];
} Overlay101LinkedByteSlot;

typedef struct Overlay101LinkedByteEntry {
    void *output;
    s32 type;
    s32 start;
    u8 pad0C[0xC];
    s32 end;
    u8 pad1C[0xC];
    s32 elapsed;
    s32 duration;
} Overlay101LinkedByteEntry;

/* DKR v77/v80 and JFG have no exact donor for this linked-slot scheduler. */

typedef struct Overlay101LinkedSlot3 {
    u8 pad00[8];
    s16 x;
    s16 y;
    u8 pad0C[0x10];
} Overlay101LinkedSlot3;

typedef struct Overlay101LinkedEntry3 {
    void *output;
    s32 type;
    s32 startX;
    s32 startY;
    u8 pad10[8];
    s32 endX;
    s32 endY;
    u8 pad20[8];
    s32 elapsed;
    s32 duration;
} Overlay101LinkedEntry3;

typedef struct Overlay101ColorOutput {
    u8 pad00[0xF];
    u8 red;
    u8 green;
    u8 blue;
    u8 alpha;
} Overlay101ColorOutput;

typedef struct Overlay101ColorEntry {
    void *output;
    s32 type;
    s16 startRed;
    s16 startGreen;
    s16 startBlue;
    s16 startAlpha;
    u8 pad10[8];
    s16 endRed;
    s16 endGreen;
    s16 endBlue;
    s16 endAlpha;
    u8 pad20[8];
    s32 elapsed;
    s32 duration;
} Overlay101ColorEntry;

/* DKR v77/v80 and JFG have no exact donor for this linked color scheduler. */

typedef struct Overlay101LinkedColorSlot {
    u8 pad00[0xF];
    u8 red;
    u8 green;
    u8 blue;
    u8 alpha;
    u8 pad13[9];
} Overlay101LinkedColorSlot;

typedef struct Overlay101LinkedColorEntry {
    void *output;
    s32 type;
    s16 startRed;
    s16 startGreen;
    s16 startBlue;
    s16 startAlpha;
    u8 pad10[8];
    s16 endRed;
    s16 endGreen;
    s16 endBlue;
    s16 endAlpha;
    u8 pad20[8];
    s32 elapsed;
    s32 duration;
} Overlay101LinkedColorEntry;

/* DKR v77/v80 and JFG have no exact donor for this frame-step update. */

typedef struct Overlay101FrameOutput {
    u8 pad00[0xC];
    u8 frameCount;
    u8 frame;
} Overlay101FrameOutput;

typedef struct Overlay101FrameEntry {
    void *output;
    s32 type;
    s32 delay;
    u8 pad0C[0x1C];
    s32 elapsed;
    s32 duration;
} Overlay101FrameEntry;

/* DKR v77/v80 and JFG have no exact donor for this frame-sequence scheduler. */

typedef struct Overlay101FrameSlot {
    u8 pad00[0xC];
    u8 frameCount;
    u8 frame;
    u8 pad0E[0xE];
} Overlay101FrameSlot;

typedef struct Overlay101FrameSchedule {
    void *output;
    s32 type;
    s32 delay;
    u8 pad0C[0x1C];
    s32 elapsed;
    s32 duration;
} Overlay101FrameSchedule;

typedef struct Overlay101GlobalPairEntry_UpdateGlobalPair {
    void *output;
    s32 type;
    s32 startX;
    s32 startY;
    u8 pad10[8];
    s32 endX;
    s32 endY;
    u8 pad20[8];
    s32 elapsed;
    s32 duration;
} Overlay101GlobalPairEntry_UpdateGlobalPair;

/* DKR v77/v80 and JFG have no exact donor for this global-pair scheduler. */

typedef struct Overlay101GlobalPairEntry_ScheduleGlobalPair {
    void *output;
    s32 type;
    s32 startX;
    s32 startY;
    u8 pad10[8];
    s32 endX;
    s32 endY;
    u8 pad20[8];
    s32 elapsed;
    s32 duration;
} Overlay101GlobalPairEntry_ScheduleGlobalPair;

typedef struct Overlay101DispatchEntry {
    void *output;
    s32 type;
    u8 pad08[0x28];
} Overlay101DispatchEntry;

/* Tier B: the dispatcher reads serialized payloads as f32. The first member
 * also permits ordinary C89 initialization of those active float payloads. */
typedef union Overlay101EventValue {
    f32 f;
    s32 i;
} Overlay101EventValue;

typedef struct Overlay101Event {
    f32 time;
    u16 type;
    u8 index;
    u8 key;
    s32 value8;
    s32 valueC;
    Overlay101EventValue value10;
    Overlay101EventValue value14;
    Overlay101EventValue value18;
} Overlay101Event;

typedef struct Overlay101Gfx {
    u32 w0;
    u32 w1;
} Overlay101Gfx;

typedef struct Overlay101BoundsNode_GetBounds {
    u8 pad00[8];
    u8 type;
    u8 pad09[5];
    s16 x;
    s16 y;
    s16 width;
    s16 height;
} Overlay101BoundsNode_GetBounds;

typedef struct Overlay101BoundsNode_DrawElement {
    u8 pad00[8];
    u8 type;
    u8 pad09[5];
    s16 x;
    s16 y;
    u8 pad12[4];
    u8 opacity;
    u8 scale;
} Overlay101BoundsNode_DrawElement;

typedef struct Overlay101Element {
    u8 pad00[8];
    s16 x;
    s16 y;
    u8 count;
    u8 index;
    u8 kind;
    u8 red;
    u8 green;
    u8 blue;
    u8 alpha;
    u8 flags;
    /* Tier B: the font path patches and restores bytes in text buffers. */
    u8 *pixels;
} Overlay101Element;

typedef struct Overlay101ClipNode {
    u8 pad00[8];
    u8 type;
    u8 pad09[5];
    s16 x;
    s16 y;
    u8 pad12[4];
    u8 intensity;
    u8 alpha;
} Overlay101ClipNode;

typedef struct Overlay101Texture {
    s16 width;
    s16 height;
    u8 pad04[0xC];
    u16 pixels[1];
} Overlay101Texture;

typedef struct Overlay101TextureElement {
    u8 pad00[8];
    s16 x;
    s16 y;
    u8 pad0C[4];
    Overlay101Texture *texture;
} Overlay101TextureElement;

typedef struct Overlay101DrawNode_DrawTransformed {
    u8 pad00[8];
    u8 type;
    u8 pad09[5];
    s16 x;
    s16 y;
} Overlay101DrawNode_DrawTransformed;

typedef struct Overlay101TransformElement {
    u8 pad00[8];
    s16 x;
    s16 y;
    f32 scale;
    s16 rotation;
    u8 color;
    u8 pad13;
    f32 depth;
    u8 pad18[4];
    void *object;
} Overlay101TransformElement;

typedef struct Overlay101Transform {
    s16 rotateZ;
    s16 rotateY;
    s16 rotateX;
    s16 pad06;
    f32 scale;
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x10];
    f32 depth;
} Overlay101Transform;

typedef struct Overlay101FrameRect {
    s16 x0;
    s16 y0;
    s16 x1;
    s16 y1;
    s32 color;
} Overlay101FrameRect;

typedef struct Overlay101BorderRect {
    s16 x0;
    s16 y0;
    s16 x1;
    s16 y1;
    u32 color;
} Overlay101BorderRect;

typedef struct Overlay101PanelRect {
    s16 x0;
    s16 y0;
    s16 x1;
    s16 y1;
    u32 color;
} Overlay101PanelRect;

typedef struct Overlay101Panel {
    u8 pad00[8];
    u8 mode;
    s8 phase;
    s16 currentX;
    s16 currentY;
    s16 x;
    s16 y;
    s16 width;
    s16 height;
    u8 intensity;
    u8 alpha;
    void *content;
} Overlay101Panel;

typedef struct Overlay101Vertex {
    s16 x;
    s16 y;
    s16 z;
    u8 red;
    u8 green;
    u8 blue;
    u8 alpha;
} Overlay101Vertex;

typedef struct Overlay101ClockPoint {
    f32 x;
    f32 y;
} Overlay101ClockPoint;

typedef union Overlay101PhaseValue {
    volatile f32 write;
    f32 read;
} Overlay101PhaseValue;

typedef struct Overlay101PhaseSpill {
    u32 pad0[2];
    Overlay101PhaseValue minor;
    u32 pad1[4];
} Overlay101PhaseSpill;

typedef struct Overlay101DrawNode_DrawChain {
    s32 type;
    struct Overlay101DrawNode_DrawChain *next;
} Overlay101DrawNode_DrawChain;

typedef struct Overlay101DrawChain {
    s32 type;
    Overlay101DrawNode_DrawChain *next;
    u8 mode;
    u8 pad09[0xE];
    u8 enabled;
} Overlay101DrawChain;

typedef struct Overlay101UpdateNode {
    s32 type;
    struct Overlay101UpdateNode *next;
    u8 pad08[0xB];
    u8 value13;
    u8 pad14[4];
    s32 flags;
    void *resource;
} Overlay101UpdateNode;

typedef struct Overlay101UpdateChain {
    s32 type;
    Overlay101UpdateNode *next;
    u8 state;
    s8 amount;
} Overlay101UpdateChain;

typedef struct O101RootGroup3A58 {
    s32 childType;
    void *child;
    u8 mode;
    u8 pad09;
    s16 x;
    s16 width;
    s16 y;
    s16 height;
    s16 value12;
    s16 value14;
    u8 color16;
    u8 color17;
    void *data18;
} O101RootGroup3A58;

typedef struct O101Node32_3A58 {
    s32 previousType;
    void *previous;
    s16 x;
    s16 y;
    f32 scale;
    s16 value10;
    u8 color12;
    u8 color13;
    f32 value14;
    s32 value18;
    void *handle;
} O101Node32_3A58;

typedef struct O101Node20_3A58 {
    s32 previousType;
    void *previous;
    s16 x;
    s16 y;
    f32 scale;
    void *handle;
} O101Node20_3A58;

typedef struct O101Node24_3A58 {
    s32 previousType;
    void *previous;
    s16 x;
    s16 y;
    u8 length;
    s8 opacity;
    u8 mode;
    u8 color0;
    u8 color1;
    u8 color2;
    u8 color3;
    u8 kind;
    void *text;
} O101Node24_3A58;

typedef struct O101Inputs3A58 {
    void *data0;
    void *text4;
    void *text8;
    void *dataC;
    void *text10;
    void *data14;
    void *data18;
    void *text1C;
    void *text20;
    void *text24;
    void *data28;
    void *text2C;
    void *data30;
    void *text34;
    void *data38;
    void *text3C;
    void *data40;
    void *text44;
    void *data48;
    void *text4C;
    void *text50;
    void *data54;
    void *text58;
    void *text5C;
    void *text60;
    void *text64;
} O101Inputs3A58;

typedef struct Root_F000512C {
    u8 pad00[0x1C];
    s32 chainType;
    void *chain;
    u8 kind;
    u8 pad25;
    s16 value26;
    s16 value28;
    s16 value2A;
    s16 value2C;
    s16 width2E;
    s16 height30;
    u8 color32;
    u8 color33;
    void *asset34;
    s32 childType;
    void *child;
    u8 mode40;
    u8 pad41;
    s16 x42;
    s16 width44;
    s16 y46;
    s16 height48;
    s16 value4A;
    s16 value4C;
    u8 color4E;
    u8 color4F;
    void *data50;
    s32 secondChildType;
    void *secondChild;
    u8 mode5C;
    u8 pad5D;
    s16 x5E;
    s16 width60;
    s16 y62;
    s16 height64;
    s16 value66;
    s16 value68;
    u8 color6A;
    u8 color6B;
    void *data6C;
} Root_F000512C;

typedef struct Node32 {
    s32 previousType;
    void *previous;
    s16 x;
    s16 y;
    f32 scale;
    s16 value10;
    u8 color12;
    u8 color13;
    f32 value14;
    s32 value18;
    void *handle;
} Node32;

typedef struct Node20 {
    s32 previousType;
    void *previous;
    s16 x;
    s16 y;
    f32 scale;
    void *handle;
} Node20;

typedef struct Node24 {
    s32 previousType;
    void *previous;
    s16 x;
    s16 y;
    u8 length;
    s8 opacity;
    u8 mode;
    u8 color0;
    u8 color1;
    u8 color2;
    u8 color3;
    u8 kind;
    void *text;
} Node24;

typedef struct Inputs_F000512C {
    u8 pad00[0x68];
    void *data68;
    void *data6C;
    void *text70;
    void *text74;
    void *text78;
    void *text7C;
} Inputs_F000512C;

typedef struct Root_F000571C {
    u8 pad00[0x1C];
    s32 chainType;
    void *chain;
    u8 kind;
    u8 pad25;
    s16 value26;
    s16 value28;
    s16 value2A;
    s16 value2C;
    s16 width2E;
    s16 height30;
    u8 color32;
    u8 color33;
    void *asset34;
    s32 childType;
    void *child;
    u8 mode40;
    u8 pad41;
    s16 x42;
    s16 width44;
    s16 y46;
    s16 height48;
    s16 value4A;
    s16 value4C;
    u8 color4E;
    u8 color4F;
    void *data50;
    s32 secondChildType;
    void *secondChild;
    u8 mode5C;
    u8 pad5D;
    s16 x5E;
    s16 width60;
    s16 y62;
    s16 height64;
    s16 value66;
    s16 value68;
    u8 color6A;
    u8 color6B;
    void *data6C;
} Root_F000571C;

typedef struct Inputs_F000571C {
    u8 pad00[0x80];
    void *data80;
    void *data84;
    void *text88;
    void *text8C;
    void *text90;
    void *text94;
} Inputs_F000571C;

typedef struct Root_F0005E08 {
    u8 pad00[0x1C];
    s32 chainType;
    void *chain;
    u8 kind;
    u8 pad25;
    s16 value26;
    s16 value28;
    s16 value2A;
    s16 value2C;
    s16 width2E;
    s16 height30;
    u8 color32;
    u8 color33;
    void *asset34;
    s32 childType;
    void *child;
    u8 mode40;
    u8 pad41;
    s16 x42;
    s16 width44;
    s16 y46;
    s16 height48;
    s16 value4A;
    s16 value4C;
    u8 color4E;
    u8 color4F;
    void *data50;
    s32 secondChildType;
    void *secondChild;
    u8 mode5C;
    u8 pad5D;
    s16 x5E;
    s16 width60;
    s16 y62;
    s16 height64;
    s16 value66;
    s16 value68;
    u8 color6A;
    u8 color6B;
    void *data6C;
} Root_F0005E08;

typedef struct Inputs_F0005E08 {
    u8 pad00[0x98];
    void *data98;
    void *data9C;
    void *textA0;
    void *textA4;
    void *textA8;
    void *textAC;
} Inputs_F0005E08;

typedef struct Root_F00063F8 {
    u8 pad00[0x1C];
    s32 chainType;
    void *chain;
    u8 kind;
    u8 pad25;
    s16 value26;
    s16 value28;
    s16 value2A;
    s16 value2C;
    s16 width2E;
    s16 height30;
    u8 color32;
    u8 color33;
    void *asset34;
    s32 childType;
    void *child;
    u8 mode40;
    u8 pad41;
    s16 x42;
    s16 width44;
    s16 y46;
    s16 height48;
    s16 value4A;
    s16 value4C;
    u8 color4E;
    u8 color4F;
    void *data50;
    s32 secondChildType;
    void *secondChild;
    u8 mode5C;
    u8 pad5D;
    s16 x5E;
    s16 width60;
    s16 y62;
    s16 height64;
    s16 value66;
    s16 value68;
    u8 color6A;
    u8 color6B;
    void *data6C;
} Root_F00063F8;

typedef struct Inputs_F00063F8 {
    u8 pad00[0xB0];
    void *dataB0;
    void *dataB4;
    void *textB8;
    void *textBC;
    void *textC0;
    void *textC4;
} Inputs_F00063F8;

typedef struct O101RootGroup {
    s32 childType;
    void *child;
    u8 mode;
    u8 pad09;
    s16 x;
    s16 width;
    s16 y;
    s16 height;
    s16 value12;
    s16 value14;
    u8 color16;
    u8 color17;
    void *data18;
} O101RootGroup;

typedef struct O101Node32_69E8 {
    s32 previousType;
    void *previous;
    s16 x;
    s16 y;
    f32 scale;
    s16 value10;
    u8 color12;
    u8 color13;
    f32 value14;
    s32 value18;
    void *handle;
} O101Node32_69E8;

typedef struct O101Node20_69E8 {
    s32 previousType;
    void *previous;
    s16 x;
    s16 y;
    f32 scale;
    void *handle;
} O101Node20_69E8;

typedef struct O101Node24_69E8 {
    s32 previousType;
    void *previous;
    s16 x;
    s16 y;
    u8 length;
    s8 opacity;
    u8 mode;
    u8 color0;
    u8 color1;
    u8 color2;
    u8 color3;
    u8 kind;
    void *text;
} O101Node24_69E8;

typedef struct O101Inputs69E8 {
    u8 pad000[0xC8];
    void *dataC8;
    u8 *textCC;
    u8 *textD0;
    void *dataD4;
    void *dataD8;
    u8 *textDC;
    u8 *textE0;
    void *dataE4;
    u8 *textE8;
    u8 *textEC;
    void *dataF0;
    u8 *textF4;
} O101Inputs69E8;

typedef struct Root_F00078F4 {
    u8 pad00[0x1C];
    s32 chainType;
    void *chain;
    u8 kind;
    u8 pad25;
    s16 value26;
    s16 value28;
    s16 value2A;
    s16 value2C;
    s16 width2E;
    s16 height30;
    u8 color32;
    u8 color33;
    void *asset34;
    s32 childType;
    void *child;
    u8 mode40;
    u8 pad41;
    s16 x42;
    s16 width44;
    s16 y46;
    s16 height48;
    s16 value4A;
    s16 value4C;
    u8 color4E;
    u8 color4F;
    void *data50;
} Root_F00078F4;

typedef struct Inputs_F00078F4 {
    u8 pad00[0xF8];
    void *dataF8;
    void *textFC;
    void *text100;
    void *text104;
} Inputs_F00078F4;

typedef struct Root_F0008128 {
    u8 pad00[0x1C];
    s32 chainType;
    void *chain;
    u8 kind;
    u8 pad25;
    s16 value26;
    s16 value28;
    s16 value2A;
    s16 value2C;
    s16 width2E;
    s16 height30;
    u8 color32;
    u8 color33;
    void *asset34;
    s32 childType;
    void *child;
    u8 mode40;
    u8 pad41;
    s16 x42;
    s16 width44;
    s16 y46;
    s16 height48;
    s16 value4A;
    s16 value4C;
    u8 color4E;
    u8 color4F;
    void *data50;
} Root_F0008128;

typedef struct Inputs_F0008128 {
    u8 pad00[0xF8];
    void *dataF8;
    u8 padFC[0xC];
    void *text108;
    void *text10C;
    void *text110;
} Inputs_F0008128;

typedef struct Root_F000895C {
    u8 pad00[0x1C];
    s32 chainType;
    void *chain;
    u8 kind;
    u8 pad25;
    s16 value26;
    s16 value28;
    s16 value2A;
    s16 value2C;
    s16 width2E;
    s16 height30;
    u8 color32;
    u8 color33;
    void *asset34;
    s32 childType;
    void *child;
    u8 mode40;
    u8 pad41;
    s16 x42;
    s16 width44;
    s16 y46;
    s16 height48;
    s16 value4A;
    s16 value4C;
    u8 color4E;
    u8 color4F;
    void *data50;
} Root_F000895C;

typedef struct Inputs_F000895C {
    u8 pad00[0xF8];
    void *dataF8;
    u8 padFC[0x18];
    void *text114;
    void *text118;
    void *text11C;
} Inputs_F000895C;

typedef struct Root_F0009190 {
    u8 pad00[0x1C];
    s32 chainType;
    void *chain;
    u8 kind;
    u8 pad25;
    s16 value26;
    s16 value28;
    s16 value2A;
    s16 value2C;
    s16 width2E;
    s16 height30;
    u8 color32;
    u8 color33;
    void *asset34;
    s32 childType;
    void *child;
    u8 mode40;
    u8 pad41;
    s16 x42;
    s16 width44;
    s16 y46;
    s16 height48;
    s16 value4A;
    s16 value4C;
    u8 color4E;
    u8 color4F;
    void *data50;
} Root_F0009190;

typedef struct Inputs_F0009190 {
    u8 pad00[0xF8];
    void *dataF8;
    u8 padFC[0x24];
    void *text120;
    void *text124;
    void *text128;
} Inputs_F0009190;

typedef struct O101PresARoot {
    u8 pad00[0x1C];
    s32 chainType;
    void *chain;
    u8 kind;
    u8 pad25;
    s16 value26;
    s16 value28;
    s16 value2A;
    s16 value2C;
    s16 width2E;
    s16 height30;
    u8 color32;
    u8 color33;
    void *asset34;
    s32 childType;
    void *child;
    u8 mode40;
    u8 pad41;
    s16 x42;
    s16 width44;
    s16 y46;
    s16 height48;
    s16 value4A;
    s16 value4C;
    u8 color4E;
    u8 color4F;
    void *data50;
} O101PresARoot;

typedef struct O101PresANode32 {
    s32 previousType;
    void *previous;
    s16 x;
    s16 y;
    f32 scale;
    s16 value10;
    u8 color12;
    u8 color13;
    f32 value14;
    s32 value18;
    void *handle;
} O101PresANode32;

typedef struct O101PresANode20 {
    s32 previousType;
    void *previous;
    s16 x;
    s16 y;
    f32 scale;
    void *handle;
} O101PresANode20;

typedef struct O101PresANode24 {
    s32 previousType;
    void *previous;
    s16 x;
    s16 y;
    u8 length;
    s8 opacity;
    u8 mode;
    u8 color0;
    u8 color1;
    u8 color2;
    u8 color3;
    u8 kind;
    u8 *text;
} O101PresANode24;

typedef struct O101PresAInputs {
    u8 pad000[0x12C];
    void *data12C;
    u8 *text130;
} O101PresAInputs;

typedef struct O101PresBRoot {
    u8 pad00[0x1C];
    s32 chainType;
    void *chain;
    u8 kind;
    u8 pad25;
    s16 value26;
    s16 value28;
    s16 value2A;
    s16 value2C;
    s16 width2E;
    s16 height30;
    u8 color32;
    u8 color33;
    void *asset34;
    s32 childType;
    void *child;
    u8 mode40;
    u8 pad41;
    s16 x42;
    s16 width44;
    s16 y46;
    s16 height48;
    s16 value4A;
    s16 value4C;
    u8 color4E;
    u8 color4F;
    void *data50;
} O101PresBRoot;

typedef struct O101PresBNode32 {
    s32 previousType;
    void *previous;
    s16 x;
    s16 y;
    f32 scale;
    s16 value10;
    u8 color12;
    u8 color13;
    f32 value14;
    s32 value18;
    void *handle;
} O101PresBNode32;

typedef struct O101PresBNode20 {
    s32 previousType;
    void *previous;
    s16 x;
    s16 y;
    f32 scale;
    void *handle;
} O101PresBNode20;

typedef struct O101PresBNode24 {
    s32 previousType;
    void *previous;
    s16 x;
    s16 y;
    u8 length;
    s8 opacity;
    u8 mode;
    u8 color0;
    u8 color1;
    u8 color2;
    u8 color3;
    u8 kind;
    u8 *text;
} O101PresBNode24;

typedef struct O101PresBInputs {
    u8 pad000[0x134];
    void *data134;
    u8 *text138;
} O101PresBInputs;

typedef struct O101PresCRoot {
    u8 pad00[0x1C];
    s32 chainType;
    void *chain;
    u8 kind;
    u8 pad25;
    s16 value26;
    s16 value28;
    s16 value2A;
    s16 value2C;
    s16 width2E;
    s16 height30;
    u8 color32;
    u8 color33;
    void *asset34;
    s32 childType;
    void *child;
    u8 mode40;
    u8 pad41;
    s16 x42;
    s16 width44;
    s16 y46;
    s16 height48;
    s16 value4A;
    s16 value4C;
    u8 color4E;
    u8 color4F;
    void *data50;
} O101PresCRoot;

typedef struct O101PresCNode32 {
    s32 previousType;
    void *previous;
    s16 x;
    s16 y;
    f32 scale;
    s16 value10;
    u8 color12;
    u8 color13;
    f32 value14;
    s32 value18;
    void *handle;
} O101PresCNode32;

typedef struct O101PresCNode20 {
    s32 previousType;
    void *previous;
    s16 x;
    s16 y;
    f32 scale;
    void *handle;
} O101PresCNode20;

typedef struct O101PresCNode24 {
    s32 previousType;
    void *previous;
    s16 x;
    s16 y;
    u8 length;
    s8 opacity;
    u8 mode;
    u8 color0;
    u8 color1;
    u8 color2;
    u8 color3;
    u8 kind;
    u8 *text;
} O101PresCNode24;

typedef struct O101PresCInputs {
    u8 pad000[0x13C];
    void *data13C;
    u8 *text140;
} O101PresCInputs;

typedef struct O101PresDRoot {
    u8 pad00[0x1C];
    s32 chainType;
    void *chain;
    u8 kind;
    u8 pad25;
    s16 value26;
    s16 value28;
    s16 value2A;
    s16 value2C;
    s16 width2E;
    s16 height30;
    u8 color32;
    u8 color33;
    void *asset34;
    s32 childType;
    void *child;
    u8 mode40;
    u8 pad41;
    s16 x42;
    s16 width44;
    s16 y46;
    s16 height48;
    s16 value4A;
    s16 value4C;
    u8 color4E;
    u8 color4F;
    void *data50;
} O101PresDRoot;

typedef struct O101PresDNode32 {
    s32 previousType;
    void *previous;
    s16 x;
    s16 y;
    f32 scale;
    s16 value10;
    u8 color12;
    u8 color13;
    f32 value14;
    s32 value18;
    void *handle;
} O101PresDNode32;

typedef struct O101PresDNode20 {
    s32 previousType;
    void *previous;
    s16 x;
    s16 y;
    f32 scale;
    void *handle;
} O101PresDNode20;

typedef struct O101PresDNode24 {
    s32 previousType;
    void *previous;
    s16 x;
    s16 y;
    u8 length;
    s8 opacity;
    u8 mode;
    u8 color0;
    u8 color1;
    u8 color2;
    u8 color3;
    u8 kind;
    u8 *text;
} O101PresDNode24;

typedef struct O101PresDInputs {
    u8 pad000[0x144];
    void *data144;
    u8 *text148;
} O101PresDInputs;

typedef struct O101TailRoot {
    u8 pad00[0x1C];
    s32 chainType;
    void *chain;
    u8 kind;
    u8 pad25;
    s16 value26;
    s16 value28;
    s16 value2A;
    s16 value2C;
    s16 width2E;
    s16 height30;
    u8 color32;
    u8 color33;
    void *asset34;
    s32 childType;
    void *child;
    u8 mode40;
    u8 pad41;
    s16 x42;
    s16 width44;
    s16 y46;
    s16 height48;
    s16 value4A;
    s16 value4C;
    u8 color4E;
    u8 color4F;
    void *data50;
} O101TailRoot;

typedef struct O101TailNode32 {
    s32 previousType;
    void *previous;
    s16 x;
    s16 y;
    f32 scale;
    s16 value10;
    u8 color12;
    u8 color13;
    f32 value14;
    s32 value18;
    void *handle;
} O101TailNode32;

typedef struct O101TailNode24 {
    s32 previousType;
    void *previous;
    s16 x;
    s16 y;
    u8 length;
    s8 opacity;
    u8 mode;
    u8 color0;
    u8 color1;
    u8 color2;
    u8 color3;
    u8 kind;
    u8 *text;
} O101TailNode24;

typedef struct O101TailInputs {
    u8 pad000[0x14C];
    void *data14C;
    u8 *text150;
    u8 *text154;
} O101TailInputs;

typedef struct O101TailAB4CRoot {
    u8 pad00[0x1C];
    s32 chainType;
    void *chain;
    u8 kind;
    u8 pad25;
    s16 value26;
    s16 value28;
    s16 value2A;
    s16 value2C;
    s16 width2E;
    s16 height30;
    u8 color32;
    u8 color33;
    void *asset34;
    s32 childType;
    void *child;
    u8 mode40;
    u8 pad41;
    s16 x42;
    s16 width44;
    s16 y46;
    s16 height48;
    s16 value4A;
    s16 value4C;
    u8 color4E;
    u8 color4F;
    void *data50;
} O101TailAB4CRoot;

typedef struct O101TailAB4CNode32 {
    s32 previousType;
    void *previous;
    s16 x;
    s16 y;
    f32 scale;
    s16 value10;
    u8 color12;
    u8 color13;
    f32 value14;
    s32 value18;
    void *handle;
} O101TailAB4CNode32;

typedef struct O101TailAB4CNode24 {
    s32 previousType;
    void *previous;
    s16 x;
    s16 y;
    u8 length;
    s8 opacity;
    u8 mode;
    u8 color0;
    u8 color1;
    u8 color2;
    u8 color3;
    u8 kind;
    u8 *text;
} O101TailAB4CNode24;

typedef struct O101TailAB4CInputs {
    u8 pad000[0x158];
    void *data158;
    u8 *text15C;
    u8 *text160;
    u8 *text164;
    u8 *text168;
    u8 *text16C;
    u8 *text170;
    u8 *text174;
    u8 *text178;
    u8 *text17C;
    u8 *text180;
} O101TailAB4CInputs;

typedef struct Root_TailB544 {
    u8 pad00[0x1C];
    s32 chainType;
    void *chain;
    u8 kind;
    u8 pad25;
    s16 value26;
    s16 value28;
    s16 value2A;
    s16 value2C;
    s16 width2E;
    s16 height30;
    u8 color32;
    u8 color33;
    void *asset34;
    s32 childType;
    void *child;
    u8 mode40;
    u8 pad41;
    s16 x42;
    s16 width44;
    s16 y46;
    s16 height48;
    s16 value4A;
    s16 value4C;
    u8 color4E;
    u8 color4F;
    void *data50;
} Root_TailB544;

typedef struct Inputs_TailB544 {
    u8 pad000[0x184];
    void *data184;
    void *text188;
    void *text18C;
    void *text190;
    void *text194;
} Inputs_TailB544;

typedef struct O101TailBA34Root {
    u8 pad00[0x1C];
    s32 chainType;
    void *chain;
    u8 kind;
    u8 pad25;
    s16 value26;
    s16 value28;
    s16 value2A;
    s16 value2C;
    s16 width2E;
    s16 height30;
    u8 color32;
    u8 color33;
    void *asset34;
    s32 childType;
    void *child;
    u8 mode40;
    u8 pad41;
    s16 x42;
    s16 width44;
    s16 y46;
    s16 height48;
    s16 value4A;
    s16 value4C;
    u8 color4E;
    u8 color4F;
    void *data50;
    s32 tertiaryType;
    void *tertiary;
    u8 mode5C;
    u8 pad5D;
    s16 x5E;
    s16 width60;
    s16 y62;
    s16 height64;
    s16 value66;
    s16 value68;
    u8 color6A;
    u8 color6B;
    void *data6C;
} O101TailBA34Root;

typedef struct O101TailBA34Node32 {
    s32 previousType;
    void *previous;
    s16 x;
    s16 y;
    f32 scale;
    s16 value10;
    u8 color12;
    u8 color13;
    f32 value14;
    s32 value18;
    void *handle;
} O101TailBA34Node32;

typedef struct O101TailBA34Node24 {
    s32 previousType;
    void *previous;
    s16 x;
    s16 y;
    u8 length;
    s8 opacity;
    u8 mode;
    u8 color0;
    u8 color1;
    u8 color2;
    u8 color3;
    u8 kind;
    u8 *text;
} O101TailBA34Node24;

typedef struct O101TailBA34Inputs {
    u8 pad000[0x84];
    void *data084;
    u8 pad088[0x110];
    void *data198;
    u8 *text19C;
    u8 *text1A0;
    u8 *text1A4;
    u8 *text1A8;
    u8 *text1AC;
} O101TailBA34Inputs;

typedef struct O101TailC144Root {
    u8 pad00[0x1C];
    s32 chainType;
    void *chain;
    u8 kind;
    u8 pad25;
    s16 value26;
    s16 value28;
    s16 value2A;
    s16 value2C;
    s16 width2E;
    s16 height30;
    u8 color32;
    u8 color33;
    void *asset34;
    s32 childType;
    void *child;
    u8 mode40;
    u8 pad41;
    s16 x42;
    s16 width44;
    s16 y46;
    s16 height48;
    s16 value4A;
    s16 value4C;
    u8 color4E;
    u8 color4F;
    void *data50;
} O101TailC144Root;

typedef struct O101TailC144Node32 {
    s32 previousType;
    void *previous;
    s16 x;
    s16 y;
    f32 scale;
    s16 value10;
    u8 color12;
    u8 color13;
    f32 value14;
    s32 value18;
    void *handle;
} O101TailC144Node32;

typedef struct O101TailC144Node24 {
    s32 previousType;
    void *previous;
    s16 x;
    s16 y;
    u8 length;
    s8 opacity;
    u8 mode;
    u8 color0;
    u8 color1;
    u8 color2;
    u8 color3;
    u8 kind;
    u8 *text;
} O101TailC144Node24;

typedef struct O101TailC144Inputs {
    u8 pad000[0x84];
    void *data084;
    u8 pad088[0x128];
    u8 *text1B0;
    u8 *text1B4;
    u8 *text1B8;
    u8 *text1BC;
    u8 *text1C0;
} O101TailC144Inputs;

typedef struct O101TailC6E8Root {
    s32 chainType;
    void *chain;
    u8 kind;
    u8 pad09;
    s16 value0A;
    s16 value0C;
    s16 x;
    s16 y;
    s16 width;
    s16 height;
    u8 color0;
    u8 color1;
    void *asset;
} O101TailC6E8Root;

typedef struct O101TailC6E8Node32 {
    s32 previousType;
    void *previous;
    s16 x;
    s16 y;
    f32 scale;
    s16 value10;
    u8 color0;
    u8 color1;
    f32 value14;
    s32 value18;
    void *handle;
} O101TailC6E8Node32;

typedef struct O101TailC6E8Node20 {
    s32 previousType;
    void *previous;
    s16 x;
    s16 y;
    f32 scale;
    void *handle;
} O101TailC6E8Node20;

typedef struct Overlay101Resource20 {
    u8 pad00[0x10];
    void *resource;
} Overlay101Resource20;

typedef struct Overlay101Resource32 {
    u8 pad00[0x1C];
    void *resource;
} Overlay101Resource32;

typedef struct Overlay101OrderEntry {
    void *first;
    void *second;
    u8 pad08[0x14];
} Overlay101OrderEntry;

extern s32 D_8007BF44;

extern s32 gOverlay101CurrentConfig;

extern s32 gOverlay101ConfigValue;

extern s32 gOverlay101CurrentResource;

extern s32 gOverlay101Ready;

extern void mainChangeLevel(s32 mode, s32 arg1, s32 arg2, s32 value,
                                     s32 enabled, s32 arg5);

extern void overlay101SetScaleReloc(f32 scale, s32 arg1);

extern void mainTitlePageInit(s32 mode);

extern void mainSetMode(s32 resource);

extern void func_80001608(void);

extern s32 gOverlay101EntryCount;

extern void amSndPlay(s32 soundId, s32 arg1);

extern s32 gOverlay101OrderCount;

extern f32 gOverlay101TimedScale;

extern f32 gOverlay101ImmediateScale;

extern s32 gOverlay101GlobalX;

extern s32 gOverlay101GlobalY;

extern Overlay101Event *gOverlay101Value220;

extern s32 gOverlay101Value224;

extern s32 gOverlay101Value228;

extern Overlay101Event *D_220;

extern s32 D_224;

extern s32 D_228;

extern s32 D_218;

extern u8 D_8007BF60;

void viGetCurrentSize(s32 *width, s32 *height);

extern u32 gOverlay101RenderFlags;

void fontBackground(s32, s32, s32, s32);

void fontColour(s32 red, s32 green, s32 blue, s32 alpha,
                             s32 opacity);

extern Gfx D_230[];

/* Tier B: this function's four runtime R_MIPS_26 records are all SYMBOL
 * operations. Two name overlay 101 +0x1F80 (overlay101SetScissor), one names
 * overlay 101 +0x2118 (overlay101GetBounds) and one names the resident
 * texDPInit at +0x344D0 past the resident base. The extracted assembly
 * shows all four as a jump to overlay offset 0 because a SYMBOL record ships
 * the 0xF0000000 addend rather than offset >> 2, so they must be routed
 * through the generated surface. The two overlay callees are ROM-exact, so
 * their prototypes are the matched ones. */
void texDPInit(Gfx **displayList);

void sprSetTextureFilter(s32 mode);

void camDo2DSprite(Overlay101Gfx **displayList, void *matrix,
                                    void *vertices,
                                    Overlay101Transform *transform,
                                    void *object, s32 rotated, s32 color);

extern RcpTextureNode D_1F4[];

void fontBackground(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

void fontColour(s32 red, s32 green, s32 blue, s32 colorAlpha,
                             s32 alpha);

void func_8002F618(Overlay101Gfx **displayList, RcpTextureNode *asset,
                                     s32 x, s32 y, s32 red, s32 green,
                                     s32 blue, s32 alpha);

extern s32 D_338;

extern void *D_33C;

extern u8 D_800002D8[];

extern Overlay101ClockPoint D_278[12];

void func_80022A44(f32 scale);

void texDPTextureX(Overlay101Gfx **displayList,
                                         void *material, s32 count,
                                         s32 flags);

extern s32 gOverlay101Time214;

extern s32 gOverlay101Time338;

extern void texAnimateSprite(void *resource, void *flags,
                                     s32 value, void *values, s32 step);

/* Tier B: runtime LOCAL relocation pairs and the ROM-exact cleanup/draw
 * siblings identify the order records, the order list, the resource arrays
 * and their three counters. */
extern O101RootGroup3A58 D_C4;

extern O101RootGroup3A58 D_E0;

extern O101RootGroup3A58 D_FC;

extern O101RootGroup3A58 D_118;

extern O101RootGroup3A58 D_134;

extern void *D_C64;

extern s32 gOverlay101Resource20Count;

extern s32 gOverlay101Resource32Count;

extern s32 D_1D0;

/* Tier B: runtime relocation records distinguish the sprite and screen loaders
 * from the overlay-local reset call. Their retail callees establish arity. */
/* The tail call is a SYMBOL relocation record, not a JUMP one, so it goes
 * through the generated surface name even though the callee is defined in
 * this same overlay. */
/* Tier B: initialized data +0x340, distinct from the BSS node pool. */
extern Overlay101Event D_o101ResetData340[];

extern void *D_C78;

/* Tier B: the stream-head declarations below are address-only arguments
 * to overlay101Reset; its dispatcher establishes Overlay101Event records.
 * A scalar head declaration does not claim the complete array extent. */
extern Overlay101Event D_CE0;

extern void *D_1C0[];

extern s32 D_1C4;

extern s32 D_1C8;

extern s32 D_1CC;

extern Node20 D_200[];

extern void *D_C8C;

extern Overlay101Event D_10EC;

extern u8 D_CA0[];

extern Overlay101Event D_1514;

extern u8 D_CB4[];

extern Overlay101Event D_1920;

/* Tier B: runtime LOCAL relocation pairs and ROM-exact cleanup/draw
 * siblings identify the order records, order list, resource arrays and
 * their three counters. The text-node pool and input block are separate
 * objects; equal encoded addends never establish an alias. */
extern u8 D_CC8[];

extern Overlay101Event D_1D2C[];

/* The builder flags word sits at the head of the same shared block. It has
 * to be its own symbol rather than a field of D_0 or D_INPUT: the shipped
 * code materialises a separate base register for it, and spelling it as a
 * field lets uopt share one of theirs and drops five instructions. The
 * per-overlay name is required -- overlay 55 already publishes a D_F4 at a
 * different offset, and a shared generic name silently mis-relocates it. */
extern u32 D_800D3128;

extern u8 D_CDC[];

extern Overlay101Event D_2608;

extern u8 D_CF0[];

extern Overlay101Event D_2934;

extern u8 D_D04[];

extern Overlay101Event D_2C60;

extern u8 D_D18[];

extern Overlay101Event D_2F8C;

extern s32 gO101PresAOrderCount;

extern void *gO101PresAOrderSlots[];

extern s32 gO101PresANode32Count;

extern s32 gO101PresANode20Count;

extern O101PresANode20 gO101PresANodes20[];

extern s32 gO101PresANode24Count;

extern O101PresAInputs gO101PresAInputs;

extern u8 gO101PresAAssetD2C[];

extern Overlay101Event gO101PresAFinalObject32B8;

extern s32 gO101PresBOrderCount;

extern void *gO101PresBOrderSlots[];

extern s32 gO101PresBNode32Count;

extern s32 gO101PresBNode20Count;

extern O101PresBNode20 gO101PresBNodes20[];

extern s32 gO101PresBNode24Count;

extern O101PresBInputs gO101PresBInputs;

extern u8 gO101PresBAssetD40[];

extern Overlay101Event gO101PresBFinalObject34E8;

extern s32 gO101PresCOrderCount;

extern void *gO101PresCOrderSlots[];

extern s32 gO101PresCNode32Count;

extern s32 gO101PresCNode20Count;

extern O101PresCNode20 gO101PresCNodes20[];

extern s32 gO101PresCNode24Count;

extern O101PresCInputs gO101PresCInputs;

extern u8 gO101PresCAssetD54[];

extern Overlay101Event gO101PresCFinalObject3718;

extern s32 gO101PresDOrderCount;

extern void *gO101PresDOrderSlots[];

extern s32 gO101PresDNode32Count;

extern s32 gO101PresDNode20Count;

extern O101PresDNode20 gO101PresDNodes20[];

extern s32 gO101PresDNode24Count;

extern O101PresDInputs gO101PresDInputs;

extern u8 gO101PresDAssetD68[];

extern Overlay101Event gO101PresDFinalObject3948;

extern s32 gO101TailOrderCount;

extern void *gO101TailOrderSlots[];

extern s32 gO101TailNode32Count;

extern s32 gO101TailNode24Count;

extern O101TailInputs gO101TailInputs;

extern u8 gO101TailAssetD7C[];

extern Overlay101Event gO101TailFinalObject3B78;

extern s32 gO101TailAB4COrderCount;

extern void *gO101TailAB4COrderSlots[];

extern s32 gO101TailAB4CNode32Count;

extern s32 gO101TailAB4CNode24Count;

extern O101TailAB4CInputs gO101TailAB4CInputs;

extern u8 gO101TailAB4CAssetD90[];

extern Overlay101Event gO101TailAB4CFinalObject3F14;

extern u8 D_DA4[];

extern Overlay101Event D_4358;

extern s32 gO101TailBA34OrderCount;

extern s32 gO101TailBA34OrderCountCall0;

extern s32 gO101TailBA34OrderCountCall1;

extern s32 gO101TailBA34OrderCountCall2;

extern void *gO101TailBA34OrderSlots[];

extern s32 gO101TailBA34Node32Count;

extern s32 gO101TailBA34Node24Count;

extern O101TailBA34Inputs gO101TailBA34Inputs;

extern u8 gO101TailBA34AssetDB8[];

extern Overlay101Event gO101TailBA34FinalObject4630;

extern s32 gO101TailC144OrderCount;

extern void *gO101TailC144OrderSlots[];

extern s32 gO101TailC144Node32Count;

extern s32 gO101TailC144Node24Count;

extern O101TailC144Inputs gO101TailC144Inputs;

extern u8 gO101TailC144AssetDCC[];

extern Overlay101Event gO101TailC144FinalObject4A90;

extern u8 D_8007BF3C[];

extern Overlay101Event *gO101TailC6E8PresentationActive;

extern s32 gO101TailC6E8PresentationDone;

extern s32 gO101TailC6E8OrderCount;

extern O101TailC6E8Root *gO101TailC6E8OrderSlots[];

extern s32 gO101TailC6E8Node32Count;

extern s32 gO101TailC6E8Node20Count;

extern O101TailC6E8Node20 gO101TailC6E8Nodes20[];

extern u8 gO101TailC6E8AssetDF0;

extern RcpTextureNode gO101TailC6E8Handle1D4[2];

extern RcpTextureNode gO101TailC6E8Handle1F4[2];

extern void *gO101TailC6E8Handle33C;

extern void *gO101TailC6E8Handle338;

/* Every call in this function is a SYMBOL relocation record, so each callee
 * is a placeholder: the shipped word carries the 0xF0000000 addend. Named by
 * the target each record decodes to (overlay 101 offsets, or one resident
 * routine per name). The selector's 25-case switch table is the retained
 * rodata at rodata-relative +0xEAC; mk/overlays.mk rebinds the compiler's
 * private copy onto it. */
extern s32 levelGetNumber(void);

extern void *texLoadTexture(s32 key);

extern void *mathRnd(s32 a, s32 b);

extern void amTunePlay(s32 key);

extern void func_800015F8(void);

extern Overlay101Event *gOverlay101PresentationActive;

extern s32 gOverlay101PresentationDone;

extern s32 gOverlay101PresentationTimer;

extern s32 gOverlay101InputMode;

extern RcpTextureNode gOverlay101Handle1D4[2];

extern void func_8002FB34(void *context, RcpTextureNode *handle,
                                              f32 x, f32 y, f32 scaleX,
                                              f32 scaleY, s32 layer,
                                              s32 mode);

extern s32 overlay101ReadInputReloc(s32 controller);

extern RcpTextureNode gOverlay101Handle1F4[2];

extern void *gOverlay101Handle33C;

extern void overlay101ReleaseReloc(void *resource, ...);

/* Unions over the per-function views of shared module storage. */
typedef union O101Elem_D_0 { Overlay101PairSlot m0; Overlay101PairSlot12 m1; Overlay101Slot_ActivateSlot m2; Overlay101Slot_AdvanceSlot m3; Overlay101Slot_PromoteSlot m4; Overlay101ByteSlot17 m5; Overlay101ByteSlot16 m6; Overlay101LinkedSlot m7; Overlay101LinkedSlot2 m8; Overlay101LinkedFloatSlot m9; Overlay101LinkedScaledSlot m10; Overlay101LinkedByteSlot m11; Overlay101LinkedSlot3 m12; Overlay101LinkedColorSlot m13; Overlay101FrameSlot m14; O101RootGroup3A58 m15; O101RootGroup m16; Overlay101OrderEntry m17; } O101Elem_D_0;
typedef union O101View_D_0 { Root_F000512C m0; Root_F0005E08 m1; Root_F00063F8 m2; Root_F00078F4 m3; Root_F0008128 m4; Root_F000895C m5; Root_F0009190 m6; O101PresARoot m7; O101PresBRoot m8; O101PresCRoot m9; O101PresDRoot m10; O101TailRoot m11; O101TailAB4CRoot m12; Root_TailB544 m13; O101TailBA34Root m14; O101TailC144Root m15; O101TailC6E8Root m16; Root_F000571C m17; O101Elem_D_0 el[16]; } O101View_D_0;
typedef union O101Elem_D_340 { O101Node32_3A58 m0; Node32 m1; O101Node32_69E8 m2; O101PresANode32 m3; O101PresBNode32 m4; O101PresCNode32 m5; O101PresDNode32 m6; O101TailNode32 m7; O101TailAB4CNode32 m8; O101TailBA34Node32 m9; O101TailC144Node32 m10; O101TailC6E8Node32 m11; Overlay101Resource32 m12; } O101Elem_D_340;
typedef union O101Elem_D_540 { O101Node24_3A58 m0; Node24 m1; O101Node24_69E8 m2; O101PresANode24 m3; O101PresBNode24 m4; O101PresCNode24 m5; O101PresDNode24 m6; O101TailNode24 m7; O101TailAB4CNode24 m8; O101TailBA34Node24 m9; O101TailC144Node24 m10; } O101Elem_D_540;
typedef union O101View_D_1C { O101RootGroup3A58 m0; void * m1; O101RootGroup m2; } O101View_D_1C;
typedef union O101View_D_38 { O101RootGroup3A58 m0; void * m1; O101RootGroup m2; } O101View_D_38;
typedef union O101View_D_54 { O101RootGroup3A58 m0; void * m1; O101RootGroup m2; } O101View_D_54;
typedef union O101View_D_8C { O101RootGroup3A58 m0; O101RootGroup m1; } O101View_D_8C;
typedef union O101View_D_A8 { O101RootGroup3A58 m0; O101RootGroup m1; } O101View_D_A8;
typedef union O101View_D_INPUT { O101Inputs3A58 m0; Inputs_F000512C m1; Inputs_F000571C m2; Inputs_F0005E08 m3; Inputs_F00063F8 m4; O101Inputs69E8 m5; Inputs_F00078F4 m6; Inputs_F0008128 m7; Inputs_F000895C m8; Inputs_F0009190 m9; Inputs_TailB544 m10; } O101View_D_INPUT;
/* The generic reads use only the compatible output/type prefix. Dispatch
 * selects concrete records by that tag; root/node presentation views above
 * retain their separate interpretation and are not claimed common tags. */
typedef union O101Elem_gOverlay101Entries {
    Overlay101Entry m0;
    Overlay101DispatchEntry m1;
    Overlay101Byte18Entry variant0;
    Overlay101ByteEntry16_ScheduleByte16 variant1;
    Overlay101ByteEntry16_UpdateByte16 variant2;
    Overlay101ByteEntry17_ScheduleByte17 variant3;
    Overlay101ByteEntry17_UpdateByte17 variant4;
    Overlay101ColorEntry variant5;
    Overlay101DeltaEntry variant6;
    Overlay101FloatEntry variant7;
    Overlay101FrameEntry variant8;
    Overlay101FrameSchedule variant9;
    Overlay101GlobalPairEntry_ScheduleGlobalPair variant10;
    Overlay101GlobalPairEntry_UpdateGlobalPair variant11;
    Overlay101LinkedByteEntry variant12;
    Overlay101LinkedColorEntry variant13;
    Overlay101LinkedEntry variant14;
    Overlay101LinkedEntry2 variant15;
    Overlay101LinkedEntry3 variant16;
    Overlay101LinkedFloatEntry variant17;
    Overlay101LinkedScaledEntry variant18;
    Overlay101PairEntry variant19;
    Overlay101PairEntry12 variant20;
    Overlay101TimedEntry variant21;
    Overlay101TimedEntry12 variant22;
    Overlay101TimedEntry8 variant23;
} O101Elem_gOverlay101Entries;
typedef union O101Elem_gOverlay101Order { Overlay101Slot_PromoteSlot * m0; Overlay101UpdateChain * m1; void * m2; } O101Elem_gOverlay101Order;
typedef union O101Elem_gOverlay101Resources20 { O101Node20_3A58 m0; O101Node20_69E8 m1; Overlay101Resource20 m2; } O101Elem_gOverlay101Resources20;
extern O101View_D_0 D_0;
extern O101Elem_D_340 D_340[16];
extern O101Elem_D_540 D_540[32];
extern O101View_D_1C D_1C;
extern O101View_D_38 D_38;
extern O101View_D_54 D_54;
extern O101View_D_8C D_8C;
extern O101View_D_A8 D_A8;
extern O101View_D_INPUT D_INPUT;
extern O101Elem_gOverlay101Entries gOverlay101Entries[40];
extern O101Elem_gOverlay101Order gOverlay101Order[16];
extern O101Elem_gOverlay101Resources20 gOverlay101Resources20[16];
extern void camStandardOrtho();
extern void fontPrintXY();
extern void fontUseFont();
extern void frontDrawRectangles();
extern void * func_80036DD0();
extern void overlay101SubmitLinkedPairReloc();
extern void * texLoadSprite();

void overlay101Initialize(void);
void *overlay101AllocateEntry(void);
Overlay101Node *overlay101FindEntry(Overlay101Node *head, s32 value, s32 occurrence);
void overlay101UpdateEntry(Overlay101TimedEntry *entry, s32 step);
void overlay101SchedulePair(s32 index, s32 x, s32 y, f32 seconds, s32 submitValue);
void overlay101UpdateEntry12(Overlay101TimedEntry12 *entry, s32 step);
void overlay101SchedulePair12(s32 index, s32 x, s32 y, f32 seconds, s32 submitValue);
void overlay101ActivateSlot(s32 index);
void overlay101AdvanceSlot(s32 index);
void overlay101PromoteSlot(s32 index);
void overlay101UpdateByte17(Overlay101ByteEntry17_UpdateByte17 *entry, s32 step);
void overlay101ScheduleByte17(s32 index, s32 value, f32 seconds, s32 submitValue);
void overlay101UpdateByte16(Overlay101ByteEntry16_UpdateByte16 *entry, s32 step);
void overlay101ScheduleByte16(s32 index, s32 value, f32 seconds, s32 submitValue);
void overlay101UpdateEntry8(Overlay101TimedEntry8 *entry, s32 step);
void overlay101ScheduleLinkedPair(s32 index, s32 key, s32 x, s32 y, f32 seconds, s32 submitValue);
void overlay101UpdateEntry8B(Overlay101TimedEntry8 *entry, s32 step);
void overlay101ScheduleLinkedPair2(s32 index, s32 key, s32 x, s32 y, f32 seconds, s32 submitValue);
void overlay101UpdateFloat12(Overlay101FloatEntry *entry, s32 step);
void overlay101ScheduleLinkedFloat(s32 index, s32 key, f32 value, f32 seconds, s32 submitValue);
void overlay101UpdateDelta16(Overlay101DeltaEntry *entry, s32 step);
void overlay101ScheduleLinkedScaled(s32 index, s32 key, f32 value, f32 seconds, s32 submitValue);
void overlay101UpdateByte18(Overlay101Byte18Entry *entry, s32 step);
void overlay101ScheduleLinkedByte(s32 index, s32 key, s32 value, f32 seconds, s32 submitValue);
void overlay101UpdateEntry8C(Overlay101TimedEntry8 *entry, s32 step);
void overlay101ScheduleLinkedPair3(s32 index, s32 key, s32 x, s32 y, f32 seconds, s32 submitValue);
void overlay101UpdateColor(Overlay101ColorEntry *entry, s32 step);
void overlay101ScheduleLinkedColor(s32 index, s32 key, s32 red, s32 green, s32 blue, s32 alpha, f32 seconds, s32 submitValue);
void overlay101UpdateFrames(Overlay101FrameEntry *entry, s32 step);
void overlay101ScheduleFrames(s32 index, s32 key, f32 fraction, f32 rate, f32 seconds, s32 submitValue);
void overlay101UpdateGlobalPair(Overlay101GlobalPairEntry_UpdateGlobalPair *entry, s32 step);
void overlay101ScheduleGlobalPair(s32 x, s32 y, f32 seconds, s32 submitValue);
void overlay101DispatchActive(s32 value);
void overlay101Reset(void *value);
void overlay101DispatchEvents(s32 step);
void overlay101SetScissor(Overlay101Gfx **displayList, register s32 left, s32 top, s32 right, s32 bottom);
s32 overlay101GetBounds(Overlay101BoundsNode_GetBounds *node, s32 *leftOut, s32 *topOut, s32 *rightOut, s32 *bottomOut);
void overlay101DrawElement(s32 displayList, Overlay101BoundsNode_DrawElement *node, Overlay101Element *element);
void func_overlay_101_F0002510_18DDD30(Gfx **dList, Overlay101ClipNode *node, Overlay101TextureElement *element);
void overlay101DrawTransformed(Overlay101Gfx **displayList, void *matrix, void *vertices, Overlay101DrawNode_DrawTransformed *node, Overlay101TransformElement *element);
void overlay101BuildFrame(s32 kind, s32 x, s32 y, s32 width, s32 height, s32 color);
void overlay101BuildIntensityColors(s32 intensity, s32 alpha, u32 *full, u32 *dim, u32 *dimmer, u32 *darkest);
void overlay101BuildBorder(Overlay101Gfx **displayList, s32 x, s32 y, s32 width, s32 height, s32 intensity, s32 alpha, s32 swapColors);
void overlay101DrawPanel(Overlay101Gfx **displayList, Overlay101Panel *panel);
void overlay101DrawClock(Overlay101Gfx **displayList, Overlay101Panel *panel, Overlay101Vertex **vertexCursor, s32 originX, s32 originY);
void overlay101DrawChain(void *context, void *mtx, s32 *vertices, Overlay101DrawChain *chain);
void overlay101UpdateChains(s32 step);
void overlay101DrawSlots(void *context, s32 x, s32 y);
void func_overlay_101_F0003A58_18DF278(void);
void func_overlay_101_F000512C_18E094C(void);
void func_overlay_101_F000571C_18E0F3C(void);
void func_overlay_101_F0005E08_18E1628(void);
void func_overlay_101_F00063F8_18E1C18(void);
void func_overlay_101_F00069E8_18E2208(void);
void func_overlay_101_F00078F4_18E3114(void);
void func_overlay_101_F0008128_18E3948(void);
void func_overlay_101_F000895C_18E417C(void);
void func_overlay_101_F0009190_18E49B0(void);
void overlay101BuildPresentationA(void);
void overlay101BuildPresentationB(void);
void overlay101BuildPresentationC(void);
void overlay101BuildPresentationD(void);
void overlay101TailA6BC(s32 base);
void func_overlay_101_F000AB4C_18E636C(void);
void func_overlay_101_F000B544_18E6D64(void);
void func_overlay_101_F000BA34_18E7254(s32 variant);
void func_overlay_101_F000C144_18E7964(void);
void func_overlay_101_F000C6E8_18E7F08(void);
void overlay101UpdatePresentation(void *context, void *data, void *vertices, s32 step);
void overlay101Cleanup(void);
s32 overlay101ByteLength(u8 *text);

#endif
