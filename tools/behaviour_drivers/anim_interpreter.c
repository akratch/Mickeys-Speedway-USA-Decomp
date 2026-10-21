/*
 * Behaviour driver for func_800517E0 (src/main/anim.c), the animation
 * command interpreter, used by tools/behaviour_check.py:
 *
 *   tools/behaviour_check.py CANDIDATE.c --driver tools/behaviour_drivers/anim_interpreter.c \
 *       --reference src/main/anim.c
 *
 * Synthetic inputs only. Each scenario resets the world (bh_reset), builds a
 * path table, objects, models, sub-states, animation tables and a track in
 * the arena, writes a command stream, runs the interpreter once and prints
 * what it did: every call it made (recorded by the generated stand-ins, or by
 * the semantic ones below), the cursor it left, the sequencer globals, and a
 * digest of every global and every structure it could have written.
 *
 * Families:
 *   single  every handled opcode, 16 edge variants and 8 random ones. Edge
 *           variant v gives the command byte EDGE8[v & 3] (0x00, 0x7F, 0x80,
 *           0xFF -- also the four path indices the table populates) and each
 *           payload halfword j the byte pair EDGE16[(5v + 3j) & 15], so over
 *           the sixteen variants every byte and halfword field takes every
 *           combination of 0x00/0x7F/0x80/0xFF; the path at the command byte
 *           is NULL, objectless, or carries an object of one of two kinds as
 *           (v >> 2) varies. A marker command follows, so an opcode that
 *           advances the cursor by the wrong length reads it wrongly.
 *   default every jump-table entry without a case, and opcodes above the
 *           table: the interpreter spins there, so a watchdog clears the run
 *           flag after a few milliseconds and the scenario records that.
 *   dense   every handled opcode against an object that can take every arm,
 *           payload bytes walking the small values the selectors test.
 *   gate    the time and state gates: a command later than the clock, a run
 *           flag other than 1, a NULL cursor.
 *   loop    0x7E rewinds to the stream start and the clock runs back.
 *   chain   random streams of up to 24 handled commands with rising
 *           durations, including 0x7B state changes.
 *
 * Semantic stand-ins: func_8005AD64 records its arguments and applies the
 * clamps the real one applies (value to [0, 1], frame to [0, frameCount - 1],
 * nothing when the frame count is 0), writing the result into a per-object
 * record the world digest covers; the sound players hand back a handle token
 * through their pointer; trackGetTrack returns the scenario's track. Every
 * other callee and global is generated (BEHAVIOUR_AUTO_STUBS), and memory is
 * big-endian (BEHAVIOUR_BIG_ENDIAN) so byte loads read what the target reads.
 *
 * Host layout: the TU's structures hold pointers, so on the host
 * AnimCommandObject and the header's AnimPathObject (two views of one object)
 * place their later fields differently. Every object block is large enough
 * for both, both views' pointer fields are set, and the one place the views
 * collide -- AnimPathObject.soundHandle, which func_8005055C clears through an
 * s32, is AnimCommandObject.table74 on the host -- is made harmless by keeping
 * every table74 at the arena base, whose low 32 bits are zero.
 */
#define BEHAVIOUR_AUTO_STUBS 1
#define BEHAVIOUR_BIG_ENDIAN 1

#include "behaviour_rt.h"
#include BEHAVIOUR_TU

#define NOBJ 16
#define PATHS 65536 /* indices past 0xFF read NULL rather than leave the table */

/* ---- semantic stand-ins ---- */

typedef struct ModelState {
    s32 calls;
    s32 frame;
    f32 value;
    s32 blend;
} ModelState;

static AnimCommandObject *gObj[NOBJ];
static s32 gFrameCount[NOBJ];
static ModelState *gModelState[NOBJ];
static s32 gObjCount;
static AnimCommandTrack *gTrack;

void func_8005AD64(void *instance, s32 frame, s32 arg2, f32 value)
{
    s32 i, count;

    bh_call_begin("func_8005AD64");
    bh_arg_p(instance);
    bh_arg_i("s32", frame);
    bh_arg_i("s32", arg2);
    bh_arg_f32(value);
    for (i = 0; i < gObjCount && gObj[i] != instance; i++) {
    }
    if (i < gObjCount && gFrameCount[i] != 0) {
        count = gFrameCount[i];
        if (value > 1.0f) {
            value = 1.0f;
        } else if (value < 0.0f) {
            value = 0.0f;
        }
        if (frame >= count) {
            frame = count - 1;
        } else if (frame < 0) {
            frame = 0;
        }
        gModelState[i]->calls++;
        gModelState[i]->frame = frame;
        gModelState[i]->value = value;
        gModelState[i]->blend = arg2;
        bh_note("=> frame %d value %.9g", (int) frame, (double) value);
    }
    bh_call_end();
}

void amSndPlay(u16 soundId, void **handle)
{
    bh_call_begin("amSndPlay");
    bh_arg_i("u16", soundId);
    bh_arg_p(handle);
    if (handle != NULL) {
        *handle = bh_token();
        bh_note("=> %s", bh_ptr(*handle));
    }
    bh_call_end();
}

void amSndPlayXYZ(s32 id, f32 x, f32 y, f32 z, s32 priority, void **handle)
{
    bh_call_begin("amSndPlayXYZ");
    bh_arg_i("s32", id);
    bh_arg_f32(x);
    bh_arg_f32(y);
    bh_arg_f32(z);
    bh_arg_i("s32", priority);
    bh_arg_p(handle);
    if (handle != NULL) {
        *handle = bh_token();
        bh_note("=> %s", bh_ptr(*handle));
    }
    bh_call_end();
}

void *trackGetTrack(void)
{
    bh_call_begin("trackGetTrack");
    bh_note("=> %s", bh_ptr(gTrack));
    bh_call_end();
    return gTrack;
}

/* ---- inputs ---- */

static const u8 EDGE8[4] = {0x00, 0x7F, 0x80, 0xFF};
static const u8 EXTRA_PATHS[4] = {0x01, 0x40, 0x5A, 0xC3};

/* Total command length in bytes, from the handlers; 0 = no case. */
static const u8 LEN[0x80] = {
    [0x00] = 4, [0x01] = 4, [0x02] = 4, [0x03] = 4, [0x04] = 4, [0x05] = 4,
    [0x06] = 6, [0x07] = 6, [0x08] = 8, [0x09] = 4, [0x0A] = 4,
    [0x20] = 6, [0x21] = 4, [0x22] = 6, [0x23] = 4, [0x24] = 4, [0x25] = 6,
    [0x26] = 4, [0x27] = 6, [0x28] = 4, [0x29] = 6, [0x2A] = 4, [0x2B] = 6,
    [0x40] = 12, [0x41] = 4, [0x42] = 4, [0x43] = 6, [0x44] = 12, [0x45] = 12,
    [0x46] = 16, [0x47] = 10, [0x48] = 8, [0x49] = 12, [0x4A] = 8, [0x4B] = 6,
    [0x4C] = 14, [0x4D] = 14,
    [0x60] = 10, [0x61] = 6, [0x62] = 8, [0x63] = 8, [0x64] = 4, [0x65] = 4,
    [0x66] = 6, [0x67] = 6, [0x68] = 8, [0x6A] = 12, [0x6B] = 12, [0x6C] = 6,
    [0x6D] = 12, [0x6E] = 14, [0x6F] = 8,
    [0x74] = 4, [0x75] = 4, [0x76] = 4, [0x78] = 6, [0x79] = 6, [0x7A] = 6,
    [0x7B] = 4, [0x7C] = 4, [0x7D] = 4, [0x7E] = 4,
};

static u16 edge16(int i)
{
    return (u16) ((EDGE8[(i >> 2) & 3] << 8) | EDGE8[i & 3]);
}

static f32 nicef(void)
{
    return (f32) ((s32) (bh_rand() % 2001u) - 1000) / 8.0f;
}

static u32 pick(u32 seed, int n)
{
    return bh_hash32(seed) % (u32) n;
}

/* ---- the world ---- */

static void *gTable74;
static AnimPath **gPaths;
static u8 gPathState[256];
static AnimPath *gPathList[16];
static int gPathCount;
static AnimCommandModel *gModel[NOBJ];
static AnimCommandSub *gSub[NOBJ];
static AnimCommandSubState *gSubState[NOBJ];
static AnimPathObjectTarget *gTarget[NOBJ];
static AnimCommandAnimation *gAnim[3];
static AnimCommandAnimationHeader *gHeader[3];
static AnimCommandFrameRecord *gRecords[3];
static AnimCommandFrameReferenceEntry *gRecordTable[3];
static AnimCommandFrameReference *gReference[4];
static AnimCommandAnimation **gAnimArray;
static u8 *gTable70[NOBJ];
static u16 *gStream;
static int gStreamLen;
/* dense scenarios pin the path at 0x01 to an object with a sub-state. */
static int gDense = -1;

static AnimCommandModel *make_model(u32 seed, int kind)
{
    static const s8 COUNTS[6] = {0, 1, 3, 4, 0x7F, -1};
    static const s8 TC28[5] = {0, 1, 3, 0x7F, -0x80};
    static const u8 TC29[4] = {0, 1, 3, 0xFF};
    u8 *block = bh_alloc(512 + sizeof(AnimCommandModel) + 512);
    AnimCommandModel *m = (AnimCommandModel *) (block + 512);
    int i;

    bh_fill(block, 1024 + sizeof(AnimCommandModel), seed);
    for (i = 0; i < 4; i++) {
        /* kind 2 objects animate by value (kind 1), kind 3 by frame. */
        m->animationKinds[i] = kind == 2 ? 1 : (s8) ((bh_hash32(seed + i) & 1) ? 0 : 2);
        if ((bh_hash32(seed * 7 + i) & 7) == 0) {
            m->animationKinds[i] = (s8) (bh_hash32(seed + 40 + i) % 3);
        }
        m->animationValues[i] = (bh_hash32(seed + 20 + i) & 3) == 0 ? 0.0f : nicef();
    }
    m->animationCount = COUNTS[pick(seed + 1, 6)];
    m->tableCount28 = TC28[pick(seed + 2, 5)];
    m->tableCount29 = TC29[pick(seed + 3, 4)];
    m->scale5C = nicef();
    return m;
}

static AnimCommandSub *make_sub(u32 seed, AnimCommandSubState **stateOut)
{
    AnimCommandSub *s = bh_alloc(sizeof(AnimCommandSub) + 64);
    AnimCommandSubState *n = NULL;

    bh_fill(s, sizeof(AnimCommandSub), seed);
    s->unk0 = nicef();
    s->unk4 = nicef();
    if (gDense >= 0 ? gDense % 7 != 6 : bh_hash32(seed + 9) % 4 != 0) {
        n = bh_alloc(sizeof(AnimCommandSubState) + 64);
        bh_fill(n, sizeof(AnimCommandSubState), seed + 11);
        n->unk0 = nicef();
        n->unk4 = nicef();
        n->unkAC = nicef();
        /* Half with the 8 bit the 0x6D handler tests. */
        n->type10 = (u8) ((bh_hash32(seed + 12) & 0xF7) | ((bh_hash32(seed + 13) & 1) << 3));
    }
    s->next1C = n;
    *stateOut = n;
    return s;
}

static void make_animations(u32 seed)
{
    int a, i;

    for (i = 0; i < 4; i++) {
        gReference[i] = bh_alloc(sizeof(AnimCommandFrameReference) + 16);
        bh_fill(gReference[i], sizeof(AnimCommandFrameReference), seed + 100 + i);
    }
    for (a = 0; a < 3; a++) {
        static const u8 COUNT10[5] = {0, 1, 0x7F, 0x80, 0xFF};
        gAnim[a] = bh_alloc(sizeof(AnimCommandAnimation));
        bh_fill(gAnim[a], sizeof(AnimCommandAnimation), seed + 200 + a);
        gHeader[a] = bh_alloc(sizeof(AnimCommandAnimationHeader));
        bh_fill(gHeader[a], sizeof(AnimCommandAnimationHeader), seed + 210 + a);
        gHeader[a]->count10 = COUNT10[pick(seed + 220 + a, 5)];
        gRecordTable[a] = bh_alloc(256 * sizeof(AnimCommandFrameReferenceEntry));
        for (i = 0; i < 256; i++) {
            gRecordTable[a][i].reference = gReference[bh_hash32(seed + a * 256 + i) & 3];
            gRecordTable[a][i].unk4 = bh_rand();
        }
        gHeader[a]->recordTable = gRecordTable[a];
        gRecords[a] = bh_alloc(256 * sizeof(AnimCommandFrameRecord));
        bh_fill(gRecords[a], 256 * sizeof(AnimCommandFrameRecord), seed + 230 + a);
        gAnim[a]->header = gHeader[a];
        gAnim[a]->records = gRecords[a];
    }
    /* Indexed by an s8: point at the middle so every index is in range. */
    gAnimArray = bh_alloc(256 * sizeof(AnimCommandAnimation *));
    for (i = 0; i < 256; i++) {
        gAnimArray[i] = gAnim[bh_hash32(seed + 300 + i) % 3];
    }
}

static AnimCommandObject *make_object(int k, u32 seed, int kind)
{
    static const s8 INDEX[12] = {0, 1, 2, 3, 0, 1, 2, 3, 0, 3, 4, -1};
    static const s32 FRAMES[4] = {300, 0, 1, 100};
    u8 *block = bh_alloc(0x400);
    AnimCommandObject *o = (AnimCommandObject *) block;
    AnimPathObject *view = (AnimPathObject *) block;

    bh_fill(block, 0x400, seed);
    o->x = nicef();
    o->y = nicef();
    o->z = nicef();
    o->velocityX = nicef();
    o->velocityY = nicef();
    o->velocityZ = nicef();
    o->animationValue = nicef();
    o->animationIndex = INDEX[pick(seed + 1, 12)];
    o->model = gModel[k] = make_model(seed + 2, kind);
    o->sub = gSub[k] = (gDense >= 0 || bh_hash32(seed + 3) % 5 != 0) ? make_sub(seed + 4, &gSubState[k]) : NULL;
    o->animations = gAnimArray + 128;
    gTable70[k] = NULL;
    if (bh_hash32(seed + 5) % 4 != 0) {
        gTable70[k] = bh_alloc(1024 + 8);
        bh_fill(gTable70[k], 1024 + 8, seed + 6);
    }
    o->table70 = (void **) gTable70[k];
    o->table74 = (bh_hash32(seed + 7) % 4 != 0) ? gTable74 : NULL;
    o->soundHandle = (bh_hash32(seed + 8) & 1) ? bh_token() : NULL;
    /* The header view's pointer, inside the TU view's padding on the host. */
    gTarget[k] = NULL;
    if (bh_hash32(seed + 9) & 1) {
        gTarget[k] = bh_alloc(sizeof(AnimPathObjectTarget));
        bh_fill(gTarget[k], sizeof(AnimPathObjectTarget), seed + 10);
    }
    view->unk58 = gTarget[k];
    gObj[k] = o;
    gFrameCount[k] = kind == 3 ? FRAMES[pick(seed + 11, 4)] : 300;
    gModelState[k] = bh_alloc(sizeof(ModelState));
    return o;
}

static AnimPath *make_path(u32 seed)
{
    AnimPath *p = bh_alloc(sizeof(AnimPath));

    bh_fill(p, sizeof(AnimPath), seed);
    p->unk8 = NULL;
    p->currentNode = NULL;
    p->nodes = NULL;
    p->unkC = nicef();
    p->unk10 = nicef();
    p->unk1C = nicef();
    p->unk2C = nicef();
    p->unk30 = nicef();
    return p;
}

static void place_path(int index, int state, u32 seed)
{
    AnimPath *p;

    gPathState[index] = (u8) state;
    if (state == 0 || gPathCount >= 16) {
        return;
    }
    p = make_path(seed);
    if (state >= 2 && gObjCount < NOBJ) {
        int k = gObjCount++;
        p->unk8 = (AnimPathObject *) make_object(k, seed + 1000, state);
    }
    gPaths[index] = p;
    gPathList[gPathCount++] = p;
}

static void make_track(u32 seed)
{
    static const s16 COUNT18[5] = {0, 3, 0x7F, -1, -0x8000};
    int i;

    gTrack = NULL;
    if (bh_hash32(seed) % 4 == 0) {
        return;
    }
    gTrack = bh_alloc(sizeof(AnimCommandTrack));
    bh_fill(gTrack, sizeof(AnimCommandTrack), seed + 1);
    gTrack->count18 = COUNT18[pick(seed + 2, 5)];
    gTrack->entries = bh_alloc(128 * sizeof(AnimCommandTrackTexture));
    for (i = 0; i < 128; i++) {
        AnimCommandTrackEntry *e = NULL;
        if (bh_hash32(seed + 10 + i) % 3 != 0) {
            e = bh_alloc(sizeof(AnimCommandTrackEntry));
            bh_fill(e, sizeof(AnimCommandTrackEntry), seed + 500 + i);
        }
        gTrack->entries[i].texture = e;
        gTrack->entries[i].unk4 = bh_rand();
    }
}

/* edge >= 0: the four edge paths take states rotated by edge. */
static void make_world(u32 seed, int edge)
{
    int i;

    gTable74 = bh_alloc(1024); /* first: at the arena base */
    bh_fill(gTable74, 1024, seed ^ 0x74);
    bh_srand(seed);
    gPaths = bh_alloc(PATHS * sizeof(AnimPath *));
    D_800D6B00 = gPaths;
    gObjCount = 0;
    gPathCount = 0;
    make_animations(seed + 1);
    for (i = 0; i < 4; i++) {
        int state = edge >= 0 ? ((edge + i) & 3) : (int) pick(seed + 50 + i, 4);
        place_path(EDGE8[i], state, seed + 60 + i * 7);
    }
    for (i = 0; i < 4; i++) {
        int state = (int) pick(seed + 70 + i, 4);
        if (i == 0 && gDense >= 0) {
            state = 2 + (gDense & 1);
        }
        place_path(EXTRA_PATHS[i], state, seed + 80 + i * 7);
    }
    make_track(seed + 90);
    for (i = 0; i < 16; i++) {
        D_800D6B18[i] = (bh_hash32(seed + 95 + i) % 3 == 0) ? bh_token() : NULL;
    }
    D_8007BF0C = (u8) (bh_hash32(seed + 96) & 1);
    D_8007D6A8 = bh_hash32(seed + 97) % 1000;
    D_8007D6B4 = nicef();
    D_8007D6B8 = nicef();
    D_8007D6BC = (s32) (bh_hash32(seed + 98) % 50);
    gStream = bh_alloc(2048);
    gStreamLen = 0;
}

static void emit(u16 duration, int op, int byte, const u16 *payload)
{
    int n = LEN[op & 0x7F] ? (LEN[op & 0x7F] - 4) / 2 : 0;
    int j;

    if (op > 0x7E) {
        n = 0;
    }
    gStream[gStreamLen++] = duration;
    gStream[gStreamLen++] = (u16) ((op << 8) | (byte & 0xFF));
    for (j = 0; j < n; j++) {
        gStream[gStreamLen++] = payload[j];
    }
}

static void finish(u16 duration)
{
    static const u16 none[1] = {0};
    emit(duration, 0x41, 0x5A, none); /* marker: func_8004EED0(0x5A) */
    gStream[gStreamLen++] = 0;
    gStream[gStreamLen++] = 0x7F00;
    /* Trailing guard words: a wrong advance reads an unhandled opcode. */
    gStream[gStreamLen++] = 0;
    gStream[gStreamLen++] = 0x7F00;
}

/* ---- running and reporting ---- */

static void dump_world(void)
{
    uint64_t h = 0;
    int i;

    h = bh_mix64(h, bh_digest(gStream, (size_t) gStreamLen * 2));
    h = bh_mix64(h, bh_digest(gPaths, 256 * sizeof(AnimPath *)));
    for (i = 0; i < gPathCount; i++) {
        h = bh_mix64(h, bh_digest(gPathList[i], sizeof(AnimPath)));
    }
    for (i = 0; i < gObjCount; i++) {
        h = bh_mix64(h, bh_digest(gObj[i], 0x400));
        h = bh_mix64(h, bh_digest((u8 *) gModel[i] - 512, 1024 + sizeof(AnimCommandModel)));
        h = bh_mix64(h, bh_digest(gModelState[i], sizeof(ModelState)));
        if (gSub[i] != NULL) {
            h = bh_mix64(h, bh_digest(gSub[i], sizeof(AnimCommandSub)));
        }
        if (gSub[i] != NULL && gSubState[i] != NULL) {
            h = bh_mix64(h, bh_digest(gSubState[i], sizeof(AnimCommandSubState)));
        }
        if (gTarget[i] != NULL) {
            h = bh_mix64(h, bh_digest(gTarget[i], sizeof(AnimPathObjectTarget)));
        }
        if (gTable70[i] != NULL) {
            h = bh_mix64(h, bh_digest(gTable70[i], 1024 + 8));
        }
    }
    h = bh_mix64(h, bh_digest(gTable74, 1024));
    for (i = 0; i < 3; i++) {
        h = bh_mix64(h, bh_digest(gRecords[i], 256 * sizeof(AnimCommandFrameRecord)));
        h = bh_mix64(h, bh_digest(gHeader[i], sizeof(AnimCommandAnimationHeader)));
        h = bh_mix64(h, bh_digest(gAnim[i], sizeof(AnimCommandAnimation)));
    }
    for (i = 0; i < 4; i++) {
        h = bh_mix64(h, bh_digest(gReference[i], sizeof(AnimCommandFrameReference)));
    }
    if (gTrack != NULL) {
        h = bh_mix64(h, bh_digest(gTrack, sizeof(AnimCommandTrack)));
        for (i = 0; i < 128; i++) {
            if (gTrack->entries[i].texture != NULL) {
                h = bh_mix64(h, bh_digest(gTrack->entries[i].texture, sizeof(AnimCommandTrackEntry)));
            }
        }
    }
    bh_log("world h=%016llx", (unsigned long long) h);
    if (!bh_verbose()) {
        return;
    }
    /* Verbose (the explanation re-run): every structure by name. */
    for (i = 0; i < 256; i++) {
        AnimPath *p = gPaths[i];
        if (p == NULL) {
            continue;
        }
        bh_log("path[%02x] unk0=%02x unk1=%02x unk2=%04x unk4=%04x unk6=%02x unk7=%02x unk8=%s unkC=%a unk10=%a "
               "unk14=%02x unk15=%02x flags=%02x nodeCount=%02x unk1C=%a unk24=%02x unk25=%02x unk26=%02x "
               "unk27=%02x unk28=%02x unk29=%02x unk2A=%04x unk2C=%a unk30=%a",
               i, p->unk0, p->unk1, (u16) p->unk2, (u16) p->unk4, p->unk6, p->unk7, bh_ptr(p->unk8),
               (double) p->unkC, (double) p->unk10, p->unk14, p->unk15, p->flags, p->nodeCount,
               (double) p->unk1C, p->unk24, p->unk25, p->unk26, p->unk27, p->unk28, p->unk29,
               (u16) p->unk2A, (double) p->unk2C, (double) p->unk30);
    }
    for (i = 0; i < gObjCount; i++) {
        AnimCommandObject *o = gObj[i];
        bh_log("object%d=%s flags6=%04x animationValue=%a state39=%02x animationIndex=%d flags80=%08x "
               "soundHandle=%s unk88=%08x unk90=%02x frame=%d/%d value=%a calls=%d",
               i, bh_ptr(o), (u16) o->flags6, (double) o->animationValue, o->state39, o->animationIndex,
               (u32) o->flags80, bh_ptr(o->soundHandle), (u32) o->unk88, o->unk90, gModelState[i]->frame,
               gFrameCount[i], (double) gModelState[i]->value, gModelState[i]->calls);
        bh_dump_mem("object", o, 0x400);
        if (gSub[i] != NULL) {
            bh_dump_mem("sub", gSub[i], sizeof(AnimCommandSub));
        }
        if (gSub[i] != NULL && gSubState[i] != NULL) {
            bh_dump_mem("substate", gSubState[i], sizeof(AnimCommandSubState));
        }
    }
    for (i = 0; i < 3; i++) {
        bh_dump_mem("records", gRecords[i], 256 * sizeof(AnimCommandFrameRecord));
    }
    for (i = 0; i < 4; i++) {
        bh_log("reference%d flags=%04x speed12=%04x", i, (u16) gReference[i]->flags, (u16) gReference[i]->speed12);
    }
    if (gTrack != NULL) {
        for (i = 0; i < 128; i++) {
            if (gTrack->entries[i].texture != NULL) {
                bh_log("track[%d] speed12=%04x", i, (u16) gTrack->entries[i].texture->speed12);
            }
        }
    }
}

static void run(int spins)
{
    int fired;

    D_8007D698 = (AnimStreamEntry *) gStream;
    bh_watchdog(spins ? 2000 : 300000, (volatile int32_t *) &D_8007D6A4, spins);
    func_800517E0();
    fired = bh_watchdog_disarm();
    bh_log("cursor=%s A4=%d A8=%u AC=%a B4=%a B8=%a BC=%d%s", bh_ptr(D_8007D69C), (int) D_8007D6A4,
           (unsigned) D_8007D6A8, (double) D_8007D6AC, (double) D_8007D6B4, (double) D_8007D6B8,
           (int) D_8007D6BC, fired ? " watchdog" : "");
    bh_dump_globals();
    dump_world();
}

static void start(u16 duration, f32 clock)
{
    D_8007D69C = (AnimStreamEntry *) gStream;
    D_8007D6AC = clock;
    D_8007D6A4 = 1;
    (void) duration;
}

/* A byte worth trying: an edge, a small value (the switch selectors and
 * counts), a value with the low nibble 7 (0x40's selector), or noise. */
static u8 interesting(u32 seed)
{
    static const u8 SMALL[12] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 0x07, 0x17, 0x87};
    u32 r = bh_hash32(seed);

    switch (r & 3) {
        case 0:
            return EDGE8[(r >> 2) & 3];
        case 1:
            return SMALL[(r >> 2) % 12];
        case 2:
            return (u8) ((r >> 4) & 3);
        default:
            return (u8) (r >> 8);
    }
}

#define EDGE_VARIANTS 64
#define RANDOM_VARIANTS 24

/*
 * Edge variant v = 16a + 4b + c: the command byte is EDGE8[c], the path at
 * that index is in state (b + c) & 3, and payload halfword j is the byte
 * pair (EDGE8[(a + j) & 3], EDGE8[(c + 3j) & 3]). For every field (the
 * command byte, each payload byte, each payload halfword) the 64 variants
 * meet every edge value with every path state.
 */
static void single(int op, int v)
{
    static const u16 DUR[8] = {0, 1, 0x7F, 0x80, 0xFF, 100, 0x7FFF, 0xFFFF};
    u32 seed = (u32) (op * 977 + v * 131 + 7);
    u16 payload[8];
    u16 duration;
    int byte, j;

    bh_reset();
    bh_scenario("single op=%02x v=%d", op, v);
    if (v < EDGE_VARIANTS) {
        int a = v >> 4, b = (v >> 2) & 3, c = v & 3;
        make_world(seed, b);
        byte = EDGE8[c];
        for (j = 0; j < 8; j++) {
            payload[j] = (u16) ((EDGE8[(a + j) & 3] << 8) | EDGE8[(c + 3 * j) & 3]);
        }
        duration = DUR[(v * 3) & 7];
    } else {
        make_world(seed, -1);
        byte = interesting(seed + 1);
        for (j = 0; j < 8; j++) {
            payload[j] = (u16) ((interesting(seed + 10 + j) << 8) | interesting(seed + 20 + j));
        }
        duration = (u16) (bh_hash32(seed + 4) % 3000);
    }
    /* 0x7B continues only on byte 1. */
    if (op == 0x7B && v >= EDGE_VARIANTS && (v & 1)) {
        byte = 1;
    }
    emit(duration, op, byte, payload);
    finish(duration);
    start(duration, (f32) duration / 100.0f + 5.0f);
    run(op == 0x7E && duration == 0);
}

/*
 * Dense: the path at 0x01 always holds an object (alternately animated by
 * value and by frame) with a sub-state, and the payload bytes walk 0..9, so
 * the small selectors -- 0x6D's type and its 0/1/2 halfword, 0x6F's low
 * byte and index, 0x61's count compare, the table indices of 0x6A/0x6B --
 * meet an object that can take every arm.
 */
static void dense(int op, int t)
{
    u32 seed = (u32) (op * 7919 + t * 104729 + 3);
    u16 payload[8];
    int j;

    bh_reset();
    bh_scenario("dense op=%02x t=%d", op, t);
    gDense = t;
    make_world(seed, -1);
    gDense = -1;
    for (j = 0; j < 8; j++) {
        payload[j] = (u16) ((((t / 2 + j) % 10) << 8) | ((t + 3 * j) % 10));
    }
    if (op == 0x6D) {
        payload[3] = (u16) ((t / 10) % 3); /* +0xA: 0, 1, 2 */
    }
    emit(150, op, 0x01, payload);
    finish(150);
    start(150, 6.5f);
    run(0);
}

static void spin(int op, int byte)
{
    static const u16 payload[8] = {0x1234, 0x8000, 0xFFFF, 0, 0x7F7F, 0x0080, 0xFF00, 0x5A5A};

    bh_reset();
    bh_scenario("default op=%02x byte=%02x", op, byte);
    make_world((u32) (op * 31 + byte), op & 3);
    emit(10, op, byte, payload);
    finish(10);
    start(10, 3.0f);
    run(1);
}

static void gates(void)
{
    static const u16 payload[8] = {0x0102, 0x0304, 0x0506, 0x0708, 0x090A, 0x0B0C, 0x0D0E, 0x0F10};
    int k;

    for (k = 0; k < 6; k++) {
        bh_reset();
        bh_scenario("gate %d", k);
        make_world((u32) (5000 + k), k & 3);
        emit(250, 0x41, 0x11, payload);
        emit(500, 0x01, 0x7F, payload);
        finish(600);
        start(0, k == 0 ? 2.5f : (k == 1 ? 2.4999f : (k == 2 ? 5.0f : 10.0f)));
        if (k == 3) {
            D_8007D6A4 = 0;
        } else if (k == 4) {
            D_8007D6A4 = 2;
        } else if (k == 5) {
            D_8007D69C = NULL;
        }
        run(0);
    }
}

static void loops(void)
{
    static const u16 payload[8] = {0x8001, 0x7F02, 0x0003, 0xFF04, 0, 0, 0, 0};
    int k;

    for (k = 0; k < 6; k++) {
        bh_reset();
        bh_scenario("loop %d", k);
        make_world((u32) (6000 + k), k & 3);
        emit(0, 0x41, 0x21, payload);
        emit((u16) (50 + k * 25), 0x20, 0x00, payload);
        emit((u16) (100 + k * 40), 0x7E, k, payload);
        finish(2000);
        start(0, 2.75f + k * 0.5f);
        run(0);
    }
}

static int handled(int op)
{
    return op <= 0x7E && LEN[op] != 0;
}

static void chain(int k)
{
    u32 seed = (u32) (9000 + k * 101);
    u16 payload[8];
    int n = 8 + (int) pick(seed, 17), i, j;
    u16 t = 0;

    bh_reset();
    bh_scenario("chain %d", k);
    make_world(seed, -1);
    for (i = 0; i < n; i++) {
        int op;
        int byte;
        u32 draw = 0;
        do {
            op = (int) pick(seed + 100 + (u32) i * 977 + draw++, 0x7F);
        } while (!handled(op) || op == 0x7E);
        byte = (bh_hash32(seed + 200 + i) & 1) ? EDGE8[bh_hash32(seed + 300 + i) & 3]
                                                : (int) (bh_hash32(seed + 400 + i) & 0xFF);
        if (op == 0x7B) {
            byte = (bh_hash32(seed + 500 + i) % 4) ? 1 : byte;
        }
        for (j = 0; j < 8; j++) {
            u32 r = bh_hash32(seed + 600 + i * 8 + j);
            payload[j] = (r & 3) == 0 ? edge16((int) (r >> 2)) : (u16) (r >> 8);
        }
        t = (u16) (t + bh_hash32(seed + 700 + i) % 40);
        emit(t, op, byte, payload);
    }
    finish((u16) (t + 1));
    start(0, (f32) (bh_hash32(seed + 800) % ((u32) t + 30)) / 100.0f + 0.05f);
    run(0);
}

int main(void)
{
    int op, v, k;

    for (op = 0; op <= 0x7E; op++) {
        if (!handled(op)) {
            continue;
        }
        for (v = 0; v < EDGE_VARIANTS + RANDOM_VARIANTS; v++) {
            single(op, v);
        }
    }
    for (op = 0; op <= 0x7E; op++) {
        if (handled(op) && op != 0x7E) {
            for (v = 0; v < 30; v++) {
                dense(op, v);
            }
        }
    }
    for (op = 0; op <= 0x7E; op++) {
        if (!handled(op)) {
            spin(op, op == 0x0B ? 0x80 : 0x00);
        }
    }
    spin(0x7F, 0x01);
    spin(0x80, 0x00);
    spin(0xFF, 0xFF);
    gates();
    loops();
    for (k = 0; k < 48; k++) {
        chain(k);
    }
    return 0;
}
