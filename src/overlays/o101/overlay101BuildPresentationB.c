#include "PR/ultratypes.h"

typedef struct Overlay101BuilderRoot {
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
    void *text50;
} Overlay101BuilderRoot;

typedef struct Overlay101Node32 {
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
} Overlay101Node32;

typedef struct Overlay101Node20 {
    s32 previousType;
    void *previous;
    s16 x;
    s16 y;
    f32 scale;
    void *handle;
} Overlay101Node20;

typedef struct Overlay101Node24 {
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
} Overlay101Node24;

/* Lane c2-o101 (2026-09-10) resolved four of the A/B alias pairs against the
 * target's own addressing: the order array, the order counter, the node-20
 * pool and the node-20 counter each show ONE address in the target where this
 * candidate declared two, so the B members were folded into the A members.
 * That is worth five masked words on every sibling in this family. The
 * node-24 counter is also one address in the target, but folding it costs a
 * further instruction here and is left split until the allocator web that
 * causes it is understood. The remaining A/B aliases are unproven either way. */
/* Every remaining A/B alias below is a distinct relocation identity, even
 * where its workbench link placeholder has the same encoded addend as its
 * sibling. */
extern Overlay101BuilderRoot gOverlay101BuilderRoot;
extern s32 gOverlay101BuilderRootChainReloc;
extern s32 gOverlay101BuilderRootChildReloc;
extern s32 gOverlay101BuilderOrderCountA;
extern void *gOverlay101BuilderOrderA[];

extern s32 gOverlay101BuilderNode20CountA;
extern Overlay101Node20 gOverlay101BuilderNodes20A[];

extern s32 gOverlay101BuilderNode32CountA;
extern s32 gOverlay101BuilderNode32CountB;
extern Overlay101Node32 gOverlay101BuilderNodes32A[];
extern Overlay101Node32 gOverlay101BuilderNodes32B[];

extern s32 gOverlay101BuilderNode24CountA;
extern s32 gOverlay101BuilderNode24CountB;
extern Overlay101Node24 gOverlay101BuilderNodes24A[];
extern Overlay101Node24 gOverlay101BuilderNodes24B[];

extern u8 gOverlay101BuilderAssetA;
extern void *gOverlay101BuilderText;
extern u8 *gOverlay101BuilderInput12C;
extern u8 *gOverlay101BuilderInput130;

/* The two create calls intentionally have different arities. The old-style
 * declaration preserves that real call surface without inventing an ABI. */
extern void *overlay101BuilderCreateReloc();
extern void overlay101BuilderCreateFinalReloc(void *text, s32 index,
                                               s32 color, s32 *count);
extern s32 overlay101ByteLength(u8 *text);

/* Exact overlay 101 body at +0x9D04.
 *
 * Lane s1-trio (2026-09-11) took the four presentation builders from 161, 157,
 * 163 and 151 masked words to 145, 145, 145 and 131, each measured separately.
 * Three source-shape levers, none costing an instruction:
 *
 * L59 -- every assignment group down to the node-24 header is ONE physical
 *   line. as1 minimises (start_time, -aftercycles, -latency, addr, lineno,
 *   ready-list position); with a group's stores on separate lines `lineno` is
 *   the deciding key and emits them in source order, while the ROM emits
 *   several of these groups' constant materialisations reversed. Folding
 *   retires the key and the raw ready-list order supplies the reversal. The
 *   minimal paying set is the root header, the node-32 pre-call group and the
 *   root's second header; folding the other four is flat on the positional
 *   count and moves three words out of the structural bucket into the naming
 *   one, which is the better residual to inherit. All 1,024 fold subsets were
 *   measured on each of the four.
 * The node-24 text store goes BEFORE the counter bump, never last. Worth four
 *   words on the A, B and C relatives and six on D. All 1,440 orders of the
 *   eight tail statements were measured on B: 145 is the floor, 80 orders
 *   reach it and the worst reads 149.
 *
 * Refuted here, each measured. The quadruplet's call-spanning-web lever --
 *   storing handle, previousType and previous through the PRE-call node-32
 *   pointer -- reads 184 at a size delta of +12: this relative's target
 *   re-reads the counter after the call and stores through the recomputed
 *   pointer, which is what this shape already does. The `volatile` casts the
 *   A, C and D relatives carried in the tail are exactly inert and are gone.
 *   Inlining the node-24 or node-32 index local reads 148 and 155; merging the
 *   two node-24 index locals reads 206 at +8, so the counter partition is
 *   already separated as it should be.
 * The byte-length local is `u8`, not `s8`, the store takes an `(s8)` cast and
 *   the mask in the opacity expression goes. 145 to 143 on each of the three.
 *   Semantically exact: the value is only ever truncated to eight bits, so the
 *   declaration states what the code already guarantees and `& 0xFF` becomes
 *   redundant; the cast is what draws the extra ugen temp. Every one of 540
 *   type cells that reaches 143 has the LOCAL `u8` and the store cast `(s8)`.
 *   The same axis is worth 67 and 81 words on the 1520-byte relatives.
 * Do NOT re-run these, all measured 2026-09-11 by lane p4-pres and all flat:
 *   245 L126 copy carriers over all thirteen locals; the whole tail statement
 *   order, EXHAUSTIVELY, all 4,480 permutations that respect the four chain
 *   dependences. And do not adopt what the nocs sweep offers: a second
 *   definition of `previousType` or `previous` reads 138 at size delta 0, and
 *   respelling the chain-head reads reaches 136, but every such cell reads a
 *   root field after that field has been set to this node, which self-links the
 *   chain. What they do prove is that deleting the previous-link LOAD closes
 *   the +4, so the surplus is reachable from the load side as well as from the
 *   colour constant. See the shard for the register census, which shows these
 *   naming rows are per-web colour and not one ring cycle.
 * See docs/matching-triage-handoffs/overlay101BuildPresentationB.md for the
 * remaining residual and the decision variable that blocks it. */
#ifdef NON_MATCHING
void overlay101BuildPresentationB(void) {
    s32 orderIndex;
    s32 node32IndexA;
    s32 node32IndexB;
    s32 node20Index;
    s32 node24IndexA;
    s32 node24IndexB;
    s32 previousType;
    void *previous;
    void *handle;
    u8 length;
    f32 opacityScale;
    s32 dimColor;
    Overlay101Node32 *node32A;
    Overlay101Node32 *node32B;
    Overlay101Node20 *node20A;
    Overlay101Node20 *node20B;
    Overlay101Node24 *node24A;
    Overlay101Node24 *node24B;

    opacityScale = 1.0f; orderIndex = gOverlay101BuilderOrderCountA; gOverlay101BuilderRoot.height30 = 0xF0; gOverlay101BuilderRoot.width2E = 0x140; gOverlay101BuilderRoot.kind = 4; gOverlay101BuilderRoot.asset34 = &gOverlay101BuilderAssetA; gOverlay101BuilderRoot.color32 = 0xFF; gOverlay101BuilderRoot.color33 = 0xFF; gOverlay101BuilderRoot.value26 = 0; gOverlay101BuilderRoot.value28 = 0; gOverlay101BuilderRoot.value2A = 0; gOverlay101BuilderRoot.value2C = 0; gOverlay101BuilderRoot.chainType = 0; gOverlay101BuilderRoot.chain = NULL; gOverlay101BuilderOrderA[orderIndex] = &gOverlay101BuilderRootChainReloc; gOverlay101BuilderOrderCountA = orderIndex + 1;

    node32IndexA = gOverlay101BuilderNode32CountA; node32A = &gOverlay101BuilderNodes32A[node32IndexA]; node32A->x = 0xF2; node32A->y = 0x14E; node32A->value10 = 0; node32A->color12 = 0xFF; node32A->color13 = 0; node32A->value18 = 0; node32A->scale = 1.0f; node32A->value14 = 0.0f; handle = overlay101BuilderCreateReloc(0x91, NULL, orderIndex);

    node32IndexB = gOverlay101BuilderNode32CountB; node32B = &gOverlay101BuilderNodes32B[node32IndexB]; node32B->previousType = gOverlay101BuilderRoot.chainType; node32B->previous = gOverlay101BuilderRoot.chain; gOverlay101BuilderNode32CountB = node32IndexB + 1; gOverlay101BuilderRoot.chainType = 2; gOverlay101BuilderRoot.chain = node32B; node32B->handle = handle;

    orderIndex = gOverlay101BuilderOrderCountA; gOverlay101BuilderRoot.x42 = 0x20; gOverlay101BuilderRoot.y46 = 0x50; gOverlay101BuilderRoot.value4A = 0xA0; gOverlay101BuilderRoot.value4C = 0xAE; gOverlay101BuilderRoot.width44 = 0x18; gOverlay101BuilderRoot.height48 = 0x18; gOverlay101BuilderRoot.mode40 = 0; gOverlay101BuilderRoot.color4E = 0xFF; gOverlay101BuilderRoot.color4F = 0xFF; gOverlay101BuilderRoot.child = NULL; gOverlay101BuilderRoot.childType = 0; gOverlay101BuilderRoot.text50 = gOverlay101BuilderInput12C; gOverlay101BuilderOrderA[orderIndex] = &gOverlay101BuilderRootChildReloc; gOverlay101BuilderOrderCountA = orderIndex + 1;

    node20Index = gOverlay101BuilderNode20CountA; node20A = &gOverlay101BuilderNodes20A[node20Index]; node20A->x = 0x10; node20A->y = 0x16; node20A->scale = 1.0f; handle = overlay101BuilderCreateReloc(0x16, node20A, orderIndex, node32IndexB);

    node20Index = gOverlay101BuilderNode20CountA; node20B = &gOverlay101BuilderNodes20A[node20Index]; previousType = gOverlay101BuilderRoot.childType; previous = gOverlay101BuilderRoot.child; gOverlay101BuilderNode20CountA = node20Index + 1; gOverlay101BuilderRoot.childType = 1; gOverlay101BuilderRoot.child = node20B; node20B->handle = handle; node20B->previousType = previousType; node20B->previous = previous;

    node24IndexA = gOverlay101BuilderNode24CountA; node24A = &gOverlay101BuilderNodes24A[node24IndexA]; node24A->x = 0x50; node24A->y = 0x9C; length = overlay101ByteLength(gOverlay101BuilderInput130);

    dimColor = 0xC0;
    node24IndexB = gOverlay101BuilderNode24CountB;
    node24B = &gOverlay101BuilderNodes24B[node24IndexB];
    node24B->length = (s8)length;
    node24B->opacity =
        (s8)(s32)((f32)(u32)length * opacityScale);
    node24B->kind = 4;
    node24B->mode = 2;
    node24B->color0 = dimColor;
    node24B->color1 = 0xFF;
    node24B->color2 = dimColor;
    node24B->color3 = 0xFF;

    previousType = gOverlay101BuilderRoot.childType;
    previous = gOverlay101BuilderRoot.child;
    node24B->text = gOverlay101BuilderInput130;
    gOverlay101BuilderNode24CountB = node24IndexB + 1;
    gOverlay101BuilderRoot.child = node24B;
    gOverlay101BuilderRoot.childType = 3;
    node24B->previousType = previousType;
    node24B->previous = previous;
    overlay101BuilderCreateFinalReloc(&gOverlay101BuilderText, node24IndexB,
                                      dimColor,
                                      &gOverlay101BuilderNode24CountB);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o101/overlay101BuildPresentationB/func_overlay_101_F0009D04_18E5524.s")
#endif

/* PLATEAU-HANDOFF:overlay101BuildPresentationB:start
 * symbol: overlay101BuildPresentationB
 * score: 143 differing words
 * frame: 0x20
 * relocations: 52
 * first-mismatch: +0x10
 * summary: Identity-gate proc 0. Extra 0xC0 into a2; w198=c5 declined. Copy/L97/L144/L145/fallthrough/remat leave +4. Force floor 134 at +4.
 * PLATEAU-HANDOFF:overlay101BuildPresentationB:end
 */
