/*
 * Resident animation/collision block -- ROM 0x50C00-0x58570
 * (VRAM 0x80050000-0x80057970).
 *
 * PROVENANCE -- names and structural comparisons in this file use Jet Force
 * Gemini's public decompilation, principally src/anim.c, src/hit.c, src/fmv.c,
 * and their declarations. JFG is a permitted published retail-derived decomp
 * under docs/CLEANROOM.md. Mickey's own ROM remains authoritative; the block
 * is kept under its existing 16-byte-aligned boundaries because no internal
 * whole-object boundary has yet been proved.
 *
 * Flags: -O2 -mips2 -32, inherited from the src/main/ build rule.
 */

#include "PR/ultratypes.h"
#include "game/anim.h"
#include "game/charControl.h"

void *func_8002B280();
AnimPathObject *func_8000590C(ControlSpawnPacket *packet, s32 mode);
void func_80005768(AnimPathObject *object);
void piRomLoadSection();
u8 *levelGetLevel(void);
void func_800511C4();
void func_80021504(f32 value, s32 arg1);
f32 sqrtf(f32 value);
extern void func_800031E8(void *handle);
HitCopyState **func_80005750(s32 *count);

/*
 * PROVENANCE: adapted from JFG's func_80076020_76C20. Mickey's globals and
 * final compiler output are independently established from Mickey's ROM.
 */
void func_80050000(s32 *stream) {
    D_800D6D54 = stream;
    D_800D6D58 = (u8 *) *stream;
    D_800D6D5C = 0x80;
}

/*
 * PROVENANCE: adapted from JFG's func_80076044_76C44. The bitstream globals
 * are Mickey's, and the compiled result is checked against Mickey's ROM.
 */
s32 func_80050024(u32 bitCount) {
    s32 value;

    value = 0;
    if (bitCount != 0) {
        bitCount = 1 << (bitCount + 0x1F);
        do {
            if (D_800D6D5C == 0) {
                D_800D6D58++;
                D_800D6D5C = 0x80;
            }
            if (*D_800D6D58 & D_800D6D5C) {
                value |= bitCount;
            }
            bitCount >>= 1;
            D_800D6D5C >>= 1;
        } while (bitCount != 0);
    }
    return value;
}

/*
 * PROVENANCE: adapted from JFG's func_800760C0_76CC0. Mickey's ROM fixes the
 * signed-extension expression and all generated instruction choices.
 */
s32 func_800500A4(u32 bitCount) {
    u32 signMask;
    s32 value;

    value = 0;
    if (bitCount != 0) {
        signMask = 0xFFFFFFFF << (bitCount - 1);
        bitCount = 1 << (bitCount - 1);
        do {
            if (D_800D6D5C == 0) {
                D_800D6D58++;
                D_800D6D5C = 0x80;
            }
            if (*D_800D6D58 & D_800D6D5C) {
                value |= bitCount;
            }
            bitCount >>= 1;
            D_800D6D5C >>= 1;
        } while (bitCount != 0);
        if (value & signMask) {
            value |= signMask;
        }
    }
    return value;
}

void func_8005013C(void) {
    if (D_800D6D5C != 0x80) {
        D_800D6D58++;
    }
    *D_800D6D54 = (s32) D_800D6D58;
}


/*
 * Clear the current animation-sequence cursors. The exact JFG donor assembly
 * corroborates the three-global shape; this C is reconstructed from Mickey.
 */
void func_8005017C(void) {
    if (D_8007D698 != NULL) {
        D_8007D698 = NULL;
        D_8007D69C = NULL;
        D_8007D6A0 = 0;
    }
}

s32 func_800501AC(AnimStreamEntry *entry) {
    return D_8007D6C0[(entry->command >> 8) & 0xFF];
}

/*
 * PROVENANCE: JFG asm/nonmatchings/anim/func_80076968.s corroborates the CFG;
 * this exact typed body is reconstructed from Mickey's target and m2c draft.
 */
s32 func_800501C8() {
    s32 step;
    s32 done;
    s32 entryCount;
    s32 total;
    AnimStreamEntry *cursor;

    cursor = D_8007D698;
    entryCount = 0;
    done = 0;
    total = 0;
    if (cursor != NULL) {
        do {
            if (((cursor->command >> 8) & 0xFF) == 0x7F) {
                done = 1;
            }
            step = func_800501AC(cursor);
            total += step;
            if (step != 0) {
                cursor = (AnimStreamEntry *)
                    ((u8 *) cursor + (step >> 1) * 2);
                entryCount++;
                if (entryCount >= 0x2001) {
                    goto end;
                }
            } else {
                total = 0;
end:
                done = 1;
            }
        } while (done == 0);
    }
    return total;
}

/* Exact JFG donor assembly corroborates this setup shape; C is Mickey-led. */
void func_8005027C(void) {
    s32 *base;
    s32 header;

    base = D_8007D68C;
    header = *base;
    D_8007D698 = (u8 *) base + (header & 0xFFFFFF);
    D_8007D69C = D_8007D698;
    D_8007D6A0 = func_800501C8(&D_8007D698);
}

void func_800502CC(u8 pathIndex) {
    AnimPath *path;

    path = D_800D6B00[pathIndex];
    if (path != NULL) {
        if (path->unk8 != NULL) {
            func_80006EA0(path->unk8);
        }
        mmFree(path);
    }
    D_800D6B00[pathIndex] = NULL;
}

/*
 * PROVENANCE: adapted from JFG's public
 * asm/nonmatchings/anim/animseqInitPath.s. Mickey's shorter character-table
 * selection, resident object layout, and final compiler output are
 * independently established from Mickey's ROM.
 */
void func_80050348(pathIndex)
u8 pathIndex;
{
    ControlSpawnPacket packet;
    AnimPathNode *node;
    AnimPathObject *object;
    AnimPath *path;
    s16 objectId;

    path = D_800D6B00[pathIndex];
    if (path != NULL) {
        node = path->nodes;
        if ((node != NULL) && (path->unk8 == NULL) && (path->unk2 != -1)) {
            packet.x = node->unkC;
            packet.y = (s16) node->unk10;
            packet.z = (s16) node->unk14;
            packet.mode = 0xA;
            objectId = path->unk2;
            if (objectId == 0x115) {
                packet.kind = D_8007D780[D_8007BF04 & 3][D_8007BF20 & 0xF];
            } else if (objectId == 0x11A) {
                packet.kind = D_8007D780[D_8007BF04 & 3][D_8007BF24 & 0xF];
            } else if (objectId == 0x119) {
                packet.kind = D_8007D780[D_8007BF04 & 3][D_8007BF28 & 0xF];
            } else {
                packet.kind = objectId;
            }
            object = func_8000590C(&packet, 1);
            path->unk8 = object;
            if (object != NULL) {
                object->unk3C = 0;
                object->unk6 |= 0x400;
                if (object->unk44 == 0x1D) {
                    func_80005768(path->unk8);
                }
                path->unk24 = 0xFF;
                path->unk25 = 0xFF;
                path->unk26 = 0;
                path->unk27 = 0;
                path->unk2A = 0;
                path->unk2C = 1.0f;
                path->unk30 = 0.0f;
                if (path->unk8->unk58 != NULL) {
                    path->unk8->unk58->unk132 = 0;
                }
            }
        }
        path->unk28 = 0x64;
        path->unk29 = 0;
    }
}
/* JFG's animseqResetPath assembly corroborates this Mickey-led reset. */
#pragma weak animResetTrap = TrapDanglingJump
extern s32 animResetTrap(AnimPath *, f32, s32, s32);
void func_8005055C(pathIndex)
u8 pathIndex;
{
    AnimPath *path;
    AnimPathObject *object;
    u8 flags;

    path = D_800D6B00[pathIndex];
    if (path != NULL) {
        flags = path->flags;
        if (!(flags & 8)) {
            object = path->unk8;
            path->unk10 = 1.0f;
            path->unk1 = path->unk0;
            path->unkC = path->unk4 / 16384.0f;
            path->unk1C = 0.0f;
            path->unk14 = path->unk6;
            path->unk15 = path->unk7;
            path->currentNode = path->nodes;
            path->flags = flags & 0x80;
            if (object != NULL) {
                object->unk6 |= 0x400;
                path->unk24 = 0xFF;
                path->unk25 = 0xFF;
                path->unk26 = 0;
                path->unk27 = 0;
                path->unk2A = 0;
                path->unk2C = 1.0f;
                path->unk30 = 0.0f;
                if (path->unk8->unk58 != NULL) {
                    path->unk8->unk58->unk132 = 0;
                }
                animResetTrap(path, 0.0f, 0, 0);
                if (*(s32 *) &object->soundHandle != 0) {
                    func_800031E8(*(s32 *) &object->soundHandle);
                    *(s32 *) &object->soundHandle = 0;
                }
            }
        }
    }
}
void animseqStartPath(u8 pathIndex) {
    AnimPath *path;
    AnimPathObject *object;

    path = D_800D6B00[pathIndex];
    if (path != NULL) {
        if (!(path->flags & 8) && !(path->flags & 1)) {
            object = path->unk8;
            path->flags |= 1;
            path->flags &= ~4;
            if (object != NULL) {
                object->unk6 &= ~0x400;
                if (path->unk8->unk58 != NULL) {
                    path->unk8->unk58->unk132 = 1;
                }
            }
        }
    }
}
void animseqStopPath(u8 pathIndex) {
    AnimPath *path;
    AnimPathObject *object;

    path = D_800D6B00[pathIndex];
    if (path != NULL) {
        if (!(path->flags & 8) && (path->flags & 5)) {
            object = path->unk8;
            path->flags &= ~5;
            if (object != NULL) {
                object->unk6 |= 0x400;
                if (path->unk8->unk58 != NULL) {
                    path->unk8->unk58->unk132 = 0;
                }
            }
        }
    }
}
u32 func_8005077C(u8 pathIndex) {
    AnimPath *path;
    u32 result;

    path = D_800D6B00[pathIndex];
    result = 1;
    if (path != NULL) {
        return (path->flags & 1) == 0;
    }
    return result;
}

void animseqHoldPath(u8 pathIndex) {
    AnimPath *path;

    path = D_800D6B00[pathIndex];
    if (path != NULL) {
        if (!(path->flags & 8)) {
            if (path->flags & 1) {
                path->flags &= ~1;
            } else if (path->unk8 != NULL) {
                path->unk8->unk6 &= ~0x400;
                if (path->unk8->unk58 != NULL) {
                    path->unk8->unk58->unk132 = 1;
                }
            }
            path->flags |= 4;
        }
    }
}
void animseqLockPath(u8 pathIndex) {
    AnimPath *path;

    path = D_800D6B00[pathIndex];
    if (path != NULL) {
        path->flags |= 8;
    }
}

void animseqUnLockPath(u8 pathIndex) {
    AnimPath *path;

    path = D_800D6B00[pathIndex];
    if (path != NULL) {
        path->flags &= ~8;
    }
}

AnimPath *func_800508B4(u8 pathIndex) {
    return D_800D6B00[pathIndex];
}

/*
 * PROVENANCE: adapted from JFG's public
 * asm/nonmatchings/anim/func_800772C4.s. Mickey's bit-reader calls, field
 * layout, constants, and final compiler output are independently established
 * from Mickey's ROM.
 * The unsigned scale is the literal 0.01f: 0x3C23D70A needs its low halfword,
 * so IDO materializes it with lwc1 from the TU's own literal pool, which
 * places it third in the invariant group (f26,f24,f22,f20) exactly as the
 * ROM orders it. An extern read (the old D_80083FA8 spelling) stays a
 * statement-position load and surfaces four schedule words early
 * (as1 -Wa,-R trace: the constant pairs chain on $at and carry the loop-head
 * line; only a pool literal joins that group).
 */
void func_800508D4(s32 count, AnimPathNode *node, s32 stream,
                   AnimPathNode **nodeEnd, s32 *streamEnd) {
    f32 valueFloat;
    u32 value;
    s32 wideValues;

    if (count > 0) {
        f32 halfScale = 0.5f;
        f32 wideScale = 0.390625f;
        f32 unsignedScale = 0.01f;
        f32 signedScale = 0.125f;

        do {
            func_80050000(&stream);
            node->unk6 = func_80050024(8);
            wideValues = func_80050024(1);
            node->unk7 = func_80050024(1);
            node->unkC = func_800500A4(0x12) * signedScale;
            node->unk10 = func_800500A4(0x12) * signedScale;
            node->unk14 = func_800500A4(0x12) * signedScale;
            node->unk0 = func_80050024(0xC) * 0x10;
            node->unk2 = func_80050024(0xC) * 0x10;
            node->unk4 = func_80050024(0xC) * 0x10;
            node->unk8 = (u32) func_80050024(0xC) * unsignedScale;
            if (wideValues == 0) {
                node->unk8 *= wideScale;
            }
            node->unk18 = func_800500A4(0xC) * 4;
            node->unk1A = func_800500A4(0xC) * 4;
            node->unk1C = func_800500A4(0xC) * 4;
            value = func_80050024(0xC);
            valueFloat = value;
            node->unk20 = valueFloat * halfScale;
            node->unk1E = 0;
            func_8005013C();
            count--;
            node++;
        } while (count > 0);
    }
    if (nodeEnd != NULL) {
        *nodeEnd = node;
    }
    if (streamEnd != NULL) {
        *streamEnd = stream;
    }
}
/* JFG's animseqLinkNodes assembly corroborates this Mickey-led body. */
void func_80050AD4(u8 pathIndex) {
    AnimPath *path;
    s32 nodeIndex;

    path = D_800D6B00[pathIndex];
    if (path != NULL) {
        if (path->nodeCount >= 2) {
            nodeIndex = 0;
            if (path->nodeCount > 0) {
                do {
                    if (nodeIndex > 0) {
                        path->nodes[nodeIndex].previous =
                            &path->nodes[nodeIndex - 1];
                    }
                    if (nodeIndex < path->nodeCount - 1) {
                        path->nodes[nodeIndex].next =
                            &path->nodes[nodeIndex + 1];
                    }
                    nodeIndex++;
                } while (nodeIndex < path->nodeCount);
            }
        }
        if (path->flags & 0x80) {
            path->nodes[0].previous = &path->nodes[path->nodeCount - 1];
            path->nodes[path->nodeCount - 1].next = &path->nodes[0];
        } else {
            path->nodes[0].previous = &path->nodes[0];
            path->nodes[path->nodeCount - 1].next =
                &path->nodes[path->nodeCount - 1];
        }
    }
}
/*
 * PROVENANCE: adapted from JFG's src/anim.c animseqInit assembly. Mickey's
 * globals, allocator call, data boundaries, and compiler output are
 * independently established from Mickey's ROM.
 *
 * Matched ordinary C: 87 words, frame 0x18, and all 41 relocation identities.
 * Retained-seed search preserved the declarations and complete loop bodies.
 * Physical source grouping is code-generation-sensitive; keep it intact.
 * Configured untouched output is checked against the owned linked ROM range.
 */
void func_80050BF4(void)
{
  s32 emptyIndex;
  s32 offset;
  int new_var;
  u8 *cursor;
  D_800D6B04 = piRomLoad(0x3D);
  D_800D6B00 = func_8002B280(0x400, 0x81);
  offset = 0;
  new_var = 4;
  do
  {
 *((s32 *) (((u8 *) D_800D6B00) + offset)) = 0; offset += new_var; } while (offset < 0x400); cursor = (u8 *) D_800D6B08; do { cursor += new_var; *((void **) (cursor - new_var)) = (void *) 0; } while (((u32) cursor) < ((u32) D_800D6B18)); cursor = (u8 *) D_800D6B18; do { cursor = cursor + new_var; *((void **) (cursor - new_var)) = (void *) 0; } while (((u32) cursor) < ((u32) D_800D6B58)); cursor = (u8 *) D_800D6B58; do { cursor += 0x14; cursor[-0x14] = 0xFF; *((s32 *) (cursor - 0x10)) = 0; *((s32 *) (cursor - 8)) = 0; new_var += 0; } while (((u32) cursor) < ((u32) D_800D6BF8)); emptyIndex = -1; cursor = D_800D6BF8; do {
    cursor += 8;
    *((s8 *) (cursor - 8)) = emptyIndex;
  }
  while (((u32) cursor) < ((u32) D_800D6C38));
  D_8007D6B0 = 0;
 cursor = (u8 *) D_800D6C58; do { cursor += 0x40; *((s32 *) (cursor - 0x40)) = 0; *((s32 *) (cursor - 0x30)) = 0; *((s32 *) (cursor - 0x20)) = 0; *((s32 *) (cursor - 0x10)) = 0; } while (cursor != ((u8 *) D_800D6D18)); D_800D6C3E = 0; D_800D6C44 = 0;
  D_800D6C48 = 0;
  D_800D6C52 = 0xFF;
  D_800D6C54 = D_800D6C52;
  D_800D6C4C = 0;
  func_800534C0();
}
void func_80050D50(void) {
    void **entry = D_800D6B18, **end = D_800D6B58;
    do {
        if (*entry != NULL) {
            amSndStop(*entry);
            *entry = NULL;
        }
        entry++;
    } while (entry != end);
}

void animseqFreeLevelData(void) {
    if (D_8007D680 != NULL) {
        mmFree(D_8007D680);
        D_8007D680 = NULL;
        D_8007D688 = -1;
        func_80050E9C();
    }
}

/*
 * PROVENANCE: adapted from JFG's public animseqLoadLevelData assembly.
 * Mickey's third allocator argument and two-word local layout establish the
 * source-offset home independently against Mickey's ROM.
 */
void func_80050DF0(s32 levelId) {
    struct {
        s32 unused;
        s32 source;
    } locals;
    s32 *bounds;

    if (levelId != -1 && levelId != D_8007D688) {
        animseqFreeLevelData();
        bounds = (s32 *) D_800D6B04 + levelId;
        locals.source = bounds[0];
        D_8007D684 = bounds[1] - locals.source;
        if (D_8007D684 > 0) {
            D_8007D680 =
                func_8002B280(D_8007D684, 0x81, locals.source);
            if (D_8007D680 != NULL) {
                piRomLoadSection(0x3E, D_8007D680, locals.source,
                                 D_8007D684);
                D_8007D688 = levelId;
            }
        }
    }
}
/*
 * PROVENANCE: adapted from JFG's animseqFreeGroup assembly. Mickey's data
 * boundaries, calls, scheduling, and final compiler output remain authoritative.
 * Preserve the same-line cursor setup and the integer identity expressions:
 * together they retain the stock compiler's exact temporary allocation.
 */
void func_80050E9C(void) {
    s32 emptyIndex;
    u8 *cursor;
    AnimScrollReset *scroll;
    AnimLockonReset *lockon;
    AnimLightReset *light;
    s32 pathIndex;

    if (D_8007D68C != NULL) {
        if (D_8007D694 != NULL) {
            mmFree(D_8007D68C);
            D_8007D694 = NULL;
        }
        emptyIndex = -1;
        D_8007D68C = NULL;
        D_8007D690 = emptyIndex;
        func_8005017C();

        pathIndex = 0;
        do {
            func_800502CC((u8) pathIndex);
            pathIndex++;
        } while ((pathIndex ^ 0) < 0x100);

        cursor = (u8 *) D_800D6B08; do {
            cursor += 4;
            *(void **) (cursor - 4) = NULL;
        } while ((u32) cursor < (u32) D_800D6B18);

        cursor = (u8 *) D_800D6B58; do {
            cursor += 0x14;
            cursor[-0x14] = 0xFF;
            *(s32 *) (cursor - 0x10) = 0;
            *(s32 *) (cursor - 8) = 0;
        } while ((u32) cursor < (u32) D_800D6BF8);

        cursor = D_800D6BF8; do {
            cursor += 8;
            *(s8 *) (cursor - 8) = emptyIndex;
        } while ((u32) cursor < (u32) D_800D6C38);

        D_8007D6B0 = 0;
        cursor = (u8 *) D_800D6C58; do {
            cursor += 0x40;
            *(s32 *) (cursor - 0x40) = 0;
            *(s32 *) (cursor - 0x30) = 0;
            *(s32 *) (cursor - 0x20) = 0;
            *(s32 *) (cursor - 0x10) = 0;
        } while (cursor != (u8 *) D_800D6D18);

        D_800D6C3E = (pathIndex < 0x100) * 0;
        D_800D6C44 = 0;
        D_800D6C48 = 0;
        D_800D6C52 = 0xFF;
        D_800D6C54 = D_800D6C52;
        D_800D6C4C = 0;
        func_80050D50();
        func_800534C0();
    }
}
/*
 * PROVENANCE: adapted from JFG's public animseqSetupGroup assembly. Mickey's
 * directory layout, level-header field, globals, and calls are authoritative.
 */
void func_80051004(s32 groupId) {
    AnimGroupDirectoryEntry *entry;
    AnimLevelHeader *level;
    u8 *base;
    u32 packed;
    s32 foundId;
    s32 offset;
    s32 found;

    if ((groupId >= 0) && (groupId < 0x100) &&
        (groupId != D_8007D690)) {
        func_80050E9C();
        base = D_8007D680;
        entry = (AnimGroupDirectoryEntry *) base;
        do {
            packed = entry->packed;
            entry++;
            foundId = packed >> 24;
        } while ((groupId != foundId) && (foundId != 0xFF));
        found = (foundId == groupId);
        if (!found) {
            goto done;
        }
        offset = packed & 0xFFFFFF;
        if (offset == 0xFFFFFF) {
            goto done;
        }
        D_8007D690 = foundId;
        D_8007D68C = (s32 *) (base + offset);
        func_800511C4();
        animseqInitGroup();
        animseqResetGroup();
        level = (AnimLevelHeader *) levelGetLevel();
        D_8007D6B4 = level->sequenceRate;
        D_8007D6B8 = 0.0f;
        D_8007D6BC = 0;
        D_8007D6A4 = 1;
    }
done:
    return;
}
/* Exact JFG donor assembly corroborates the loop; C is Mickey-led. */
void animseqInitGroup(void) {
    s32 pathIndex;

    pathIndex = 0;
    do {
        func_80050348(pathIndex & 0xFF);
        pathIndex++;
    } while (pathIndex != 0x100);
}

/* JFG corroborates the reset-path loop; the remaining control flow is Mickey-led. */
void animseqResetGroup(void) {
    s32 pathIndex;

    if (D_8007D68C != NULL) {
        pathIndex = 0;
        do {
            func_8005055C(pathIndex & 0xFF);
            pathIndex++;
        } while (pathIndex != 0x100);
        D_8007D69C = D_8007D698;
        D_8007D6A8 = 0;
        D_8007D6AC = 0.0f;
        if (TrapDanglingJump(osRomBase) == 0) {
            D_8007D69C = NULL;
        }
        func_80050D50();
        func_800534C0();
    }
}

typedef struct AnimGroupPathHeader {
    u8 nodeCount;
    u8 unk1;
    s16 unk2;
    s16 unk4;
    u8 unk6;
    u8 flags;
    u8 nodeData[1];
} AnimGroupPathHeader;

void func_800508D4();

/*
 * PROVENANCE: adapted from JFG's func_80077468_78068 assembly. Mickey's
 * resident globals, packed fields, call identities, and output are checked
 * independently against Mickey's ROM.
 */
void func_800511C4(void) {
    u32 *entryCursor;
    s32 remaining;
    u32 headerWord;
    u32 entryWord;
    s32 pathIndex;
    AnimGroupPathHeader *source;
    AnimPath *path;
    s32 highBit;
    void func_8005055C(u8 pathIndex);

    entryCursor = (u32 *) D_8007D68C;
    headerWord = *entryCursor++;
    remaining = (headerWord >> 24) & 0xFF;
    func_8005027C();
    if (remaining > 0) {
        highBit = 0x80;
        do {
            entryWord = *entryCursor++;
            source = (AnimGroupPathHeader *)
                ((u8 *) D_8007D68C + (entryWord & 0xFFFFFF));
            path = func_8002B280((source->nodeCount * sizeof(AnimPathNode)) +
                                 sizeof(AnimPath),
                                 0x81);
            pathIndex = (entryWord >> 24) & 0xFF;
            D_800D6B00[pathIndex] = path;
            path = D_800D6B00[pathIndex];
            if (path != NULL) {
                if (source->nodeCount > 0) {
                    path->nodes = (AnimPathNode *)((u8 *)path +
                                                   sizeof(AnimPath));
                } else {
                    path->nodes = NULL;
                }
                path->unk2 = source->unk2;
                path->unk8 = NULL;
                path->unk0 = source->unk1;
                path->unk4 = source->unk4;
                path->unk6 = source->unk6;
                path->unk7 = source->flags;
                path->flags = 0;
                if (path->unk7 & highBit) {
                    path->flags = highBit;
                    path->unk7 &= 0x7F;
                }
                path->nodeCount = source->nodeCount;
                func_8005055C(pathIndex);
                func_800508D4(path->nodeCount, path->nodes, source->nodeData,
                              0, 0);
                func_80050AD4(pathIndex);
            }
            remaining--;
        } while (remaining > 0);
    }
}

extern s32 osTvType;
void func_800030B4(void *soundHandle, u8 pitch);
void func_800031C0(void *soundHandle, f32 x, f32 y, f32 z);
void func_800517E0(void);
#pragma weak animUpdateTrap = TrapDanglingJump
extern void animUpdateTrap(AnimPath *path, f32 delta, s32 updateRate,
                           s32 originalRate);

/*
 * PROVENANCE: adapted from JFG's public
 * asm/nonmatchings/anim/animseqUpdate.s. Mickey's command layout, resident
 * globals, sound-object offset, and final compiler output are independently
 * established from Mickey's ROM.
 *
 * Size-exact at 287 words. Early playback loads keep the state address
 * across the trap; the or-zero on those loads is the copy-prop barrier,
 * and that pointer dies before the path loops. The store and the
 * post-loop test reload the global. Remaining: frame 0x48 vs 0x40 and
 * the command shift after PAL math. A barrier on the store regressed.
 * Retain NON_MATCHING. */
#ifdef NON_MATCHING
void func_80051364(s32 updateRate) {
    AnimStreamEntry *command;
    AnimCameraSource **camera;
    AnimPath *path;
    AnimPathObject *object;
    s32 originalRate;
    s32 offset;
    s32 adjustedRate;
    s32 cmd; s32 *playing;
    s32 newClock;
    u16 cmdWord;
    u16 duration;
    f32 timeScale;
    f32 speed;

    if (D_8007D68C != NULL) {
        playing = &D_8007D6A4; if (*(s32 *)((u32) playing | 0) != 0) {
            if (osTvType == 0) {
                timeScale = D_80083FAC;
            } else {
                timeScale = D_80083FB0;
            }
            originalRate = updateRate; if (D_8007D6B0 > 0) {
                TrapDanglingJump(updateRate);
            }
            command = D_8007D69C; if (command != NULL) {
                if (*(s32 *)((u32) playing | 0) == 1) {
                    cmdWord = command->command; if ((cmdWord >> 8) == 0x7B) {
                        duration = command->duration;
                        if (((f32) (u32) duration / 100.0f) <
                            ((f32) (u32) (D_8007D6A8 + updateRate) *
                             timeScale)) {
                            if (osTvType == 0) {
                                adjustedRate = duration >> 1;
                            } else {
                                adjustedRate = (duration * 6) / 10;
                            }
                            updateRate = adjustedRate - D_8007D6A8;
                            D_8007D69C = command + 1; cmd = (s8) cmdWord; *playing = cmd; if (cmd == 0) {
                                originalRate = updateRate;
                            }
                        }
                    }
                }
            }
            if (updateRate > 0) {
                newClock = D_8007D6A8 + updateRate; camera = D_800D6B08; do {
                    *camera++ = NULL;
                } while (camera < (AnimCameraSource **) D_800D6B18);
                if (D_8007D6BC != 0) {
                    if (updateRate < D_8007D6BC) {
                        D_8007D6B4 += D_8007D6B8 * (f32) updateRate;
                        D_8007D6BC -= updateRate;
                    } else {
                        D_8007D6B4 += D_8007D6B8 * (f32) D_8007D6BC;
                        D_8007D6BC = 0;
                    }
                }
                D_8007D6A8 = newClock; D_8007D6AC = (f32) (u32) newClock * timeScale;
                offset = 0; do {
                    path = *(AnimPath **) ((u8 *) D_800D6B00 + offset);
                    if ((path != NULL) && (path->flags & 5)) {
                        animUpdateTrap(path, (f32) updateRate * timeScale,
                                       updateRate, originalRate);
                    }
                    offset += 4;
                } while (offset < 0x400);
                if (D_8007D6A4 == 1) {
                    func_800517E0();
                }
                TrapDanglingJump(updateRate); /* runtime: overlay 41 +0x000 */
                TrapDanglingJump(updateRate); /* runtime: overlay 41 +0x124 */
                TrapDanglingJump(updateRate); /* runtime: overlay 41 +0x1B00 */
                offset = 0; do {
                    path = *(AnimPath **) ((u8 *) D_800D6B00 + offset);
                    if (path != NULL) {
                        object = path->unk8;
                        if ((object != NULL) &&
                            (object->soundHandle != NULL)) {
                            func_800031C0(object->soundHandle, object->x,
                                          object->y, object->z);
                            if ((path->unk28 != 0x64) ||
                                (path->unk29 != 0)) {
                                speed = sqrtf((object->velocityX *
                                               object->velocityX) +
                                              (object->velocityY *
                                               object->velocityY) +
                                              (object->velocityZ *
                                               object->velocityZ));
                                func_800030B4(
                                    object->soundHandle,
                                    (u32) ((f32) path->unk28 +
                                           ((f32) path->unk29 * speed)) &
                                        0xFF);
                            }
                        }
                    }
                    offset += 4;
                } while (offset != 0x400);
            }
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_80051364.s")
#endif

/*
 * PROVENANCE: the command-interpreter role and switch organization are
 * adapted from JFG's public animseqProcessCommandList assembly. Mickey's
 * command widths, globals, object offsets, call targets, and body below are
 * established independently from Mickey's ROM and remain authoritative.
 */
#ifdef NON_MATCHING
struct AnimCommandSub;
struct AnimCommandSubState;
struct AnimCommandAnimation;
struct AnimCommandFrameReference;
struct AnimCommandFrameReferenceEntry;

typedef struct AnimCommandModel {
    u8 pad0[0x1E];
    s8 animationKinds[4];
    s8 animationCount;
    u8 pad23[5];
    s8 tableCount28;
    u8 tableCount29;
    u8 pad2A[0x32];
    f32 scale5C;
    u8 pad60[0x70];
    f32 animationValues[4];
} AnimCommandModel;

typedef struct AnimCommandObject {
    u8 pad0[6];
    s16 flags6;
    u8 pad8[4];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[4];
    f32 velocityX;
    f32 velocityY;
    f32 velocityZ;
    f32 animationValue;
    u8 pad2C[0xD];
    u8 state39;
    s8 animationIndex;
    u8 pad3B;
    s32 unk3C;
    AnimCommandModel *model;
    u8 pad44[8];
    struct AnimCommandSub *sub;
    u8 pad50[0x18];
    struct AnimCommandAnimation **animations;
    /* Tier D layout evidence places table70 at 0x70, after this pad. */
    u8 pad6C[4];
    void **table70;
    void **table74;
    u8 pad78[8];
    s32 flags80;
    void *soundHandle;
    s32 unk88;
    u8 pad8C[4];
    u8 unk90;
} AnimCommandObject;

typedef struct AnimCommandSub {
    f32 unk0;
    f32 unk4;
    u8 pad8[8];
    u8 type10;
    s8 type11;
    u8 pad12[8];
    struct AnimCommandSubState *next1C;
} AnimCommandSub;

typedef struct AnimCommandSubState {
    f32 unk0;
    f32 unk4;
    u8 pad8[8];
    u8 type10;
    s8 type11;
    u8 pad12[0x9A];
    f32 unkAC;
    u8 padB0[0xA];
    u8 unkBA;
} AnimCommandSubState;

typedef struct AnimCommandAnimationHeader {
    u8 pad0[0x10];
    u8 count10;
    u8 pad11[7];
    struct AnimCommandFrameReferenceEntry *recordTable;
} AnimCommandAnimationHeader;

typedef struct AnimCommandFrameRecord {
    u8 pad0[4];
    s32 flags;
} AnimCommandFrameRecord;

typedef struct AnimCommandFrameReference {
    u8 pad0[4];
    s16 flags;
    u8 pad6[0xC];
    s16 speed12;
} AnimCommandFrameReference;

typedef struct AnimCommandFrameReferenceEntry {
    AnimCommandFrameReference *reference;
    u32 unk4;
} AnimCommandFrameReferenceEntry;

typedef struct AnimCommandAnimation {
    AnimCommandAnimationHeader *header;
    u8 pad4[0x48];
    AnimCommandFrameRecord *records;
} AnimCommandAnimation;

/* The scroll command fills the fields left opaque by AnimScrollReset. */
typedef struct AnimCommandScroll {
    u8 textureIndex;
    u8 pad1;
    s16 duration;
    s32 x;
    s32 stepX;
    s32 y;
    s32 stepY;
} AnimCommandScroll;

typedef struct AnimCommandTrackEntry {
    u8 pad0[0x12];
    s16 speed12;
} AnimCommandTrackEntry;

typedef struct AnimCommandTrackTexture {
    AnimCommandTrackEntry *texture;
    u32 unk4;
} AnimCommandTrackTexture;

typedef struct AnimCommandTrack {
    AnimCommandTrackTexture *entries;
    u8 pad4[0x14];
    s16 count18;
} AnimCommandTrack;

/* Tier B: resident relocation entries resolve these calls to overlay 100
 * motion creation and overlay 41 slot insertion. Typed trampoline aliases
 * preserve the destination ABI without default float promotion. */
#pragma weak animCommandMotionTrap = TrapDanglingJump
extern void *animCommandMotionTrap(f32 x, f32 y, f32 z, s32 count,
                                  s32 colorB0, s32 colorB1, s32 colorB2,
                                  s32 colorA0, s32 colorA1, s32 colorA2,
                                  f32 duration);
#pragma weak animCommandSlotTrap = TrapDanglingJump
extern void animCommandSlotTrap(void *object, s32 red, s32 green, s32 blue,
                               s32 alpha, f32 duration, s32 alternateColors);

extern u8 D_8007BF0C;
extern s16 D_800D6C4E;
extern s16 D_800D6C50;

void func_80000510(u8 sequenceId);
void func_800005CC(f32 fade, u8 volume);
void amTuneResetFade(void);
void amTuneSetVolume(u8 volume);
void amAmbientPlay(u8 sequenceId);
void amAmbientResetFade(void);
void amAmbientSetFade(f32 fade, u8 volume);
void amAmbientSetVolume(u8 volume);
void amSndPlay(u16 soundId, void **handle);
void func_80002FE0(s32 id, f32 x, f32 y, f32 z, s32 priority,
                   void **handle);
void changeWeather(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4,
                   s32 arg5);
void func_80014BAC(s32 fogIndex, s32 red, s32 green, s32 blue, s32 near,
                   s32 far, f32 timer);
void func_800498FC(s32 index, f32 duration, f32 delay, s32 red,
                   s32 green, s32 blue, s32 flags);
void func_8004EED0(u8 arg0);
void func_8004E99C(void);
f32 func_8002A8BC(s32 angle);
f32 func_8002A8C0(s32 angle);
void camStartShake(s32 camNo, f32 attack, f32 sustain, f32 decay,
                   s32 magnitude);
void rumbleStart(s32 playerIndex, s32 strength, f32 duration);
void *trackGetTrack(void);
void func_8005AD64(void *instance, s32 frame, s32 arg2, f32 value);
void mainSyncNextLevel(void);
void func_80029084(s32 arg0, s32 arg1);
u8 frontGetMode(void);
void mainChangeLevel(s32 nextLevel, s32 nextCharacter, s32 nextAnimGroup,
                     s32 frontMode, s32 arg4, s32 arg5);
void mainSetAnimGroup(s32 arg0);
void joyDisable(s32 player);
void joyEnable(s32 player);

/* NON_MATCHING: Mickey-led call/type reconstruction. Shared loop carriers
 * and handler dataflow still differ; see the symbol-owned handoff. */
void func_800517E0(void) {
    AnimPath **paths;
    AnimStreamEntry *cursor;
    u16 currentCommand;
    u16 commandDuration;
    u16 duration;
    s32 opcode;
    s32 pathIndex;
    f32 commandTime;
    f32 hundred;
    f32 delta;
    f32 value;
    f32 value2;
    f32 factor;
    f32 unit;
    f32 scale;
    f32 target;
    f32 start;
    f32 end;
    f32 color;
    f32 normalized;
    s32 packed;
    s32 packed2;
    s32 high;
    s32 low;
    s32 high2;
    s32 low2;
    s32 targetValue;
    s32 index;
    s32 frame;
    s32 timer;
    s32 state;
    s32 flagsValue;
    u32 objectFlags;
    s16 signedValue;
    AnimPath *path;
    AnimCommandObject *object;
    AnimCommandSub *sub;
    AnimCommandSubState *next;
    AnimCommandAnimation *animation;
    AnimCommandAnimationHeader *header;
    AnimCommandFrameRecord *record;
    AnimCommandFrameReference *reference;
    AnimCommandTrack *track;
    AnimCommandTrackEntry *trackEntry;
    void **soundSlot;
    void *sound;
    void *entry;
    u8 type;
    s32 motionAngle;
    s32 motionCount;
    s32 colorB0;
    s32 colorB1;
    s32 colorB2;
    s32 colorA0;
    s32 colorA1;
    s32 packedField;
    f32 motionDuration;
    f32 radius;
    f32 height;
    f32 motionX;
    f32 motionZ;

    cursor = D_8007D69C;
    paths = D_800D6B00;
    if (cursor != NULL) {
        hundred = 100.0f;
        scale = 60.0f;
        factor = 0.01f;
        while ((currentCommand = cursor->command) != 0x7F00 &&
               (commandTime = (f32) (commandDuration = cursor->duration) /
                              hundred) < D_8007D6AC && D_8007D6A4 == 1) {
            opcode = (currentCommand >> 8) & 0xFF;
            pathIndex = currentCommand & 0xFF;
            delta = D_8007D6AC - commandTime;
            switch (opcode) {
                case 0: {
                    cursor = (AnimStreamEntry *) ((u8 *) cursor + 4);
                    animseqStartPath(pathIndex);
                    animResetTrap(paths[pathIndex], delta, 0, 0);
                    break;
                }
                case 1:
                    cursor = (AnimStreamEntry *) ((u8 *) cursor + 4);
                    animseqStopPath(pathIndex);
                    break;
                case 2: {
                    path = paths[pathIndex];
                    cursor = (AnimStreamEntry *) ((u8 *) cursor + 4);
                    if (path != NULL) {
                        if (path->flags & 1) {
                            func_8005055C(pathIndex);
                            animseqStartPath(pathIndex);
                            animResetTrap(paths[pathIndex], delta, 0, 0);
                        } else {
                            func_8005055C(pathIndex);
                        }
                    }
                    break;
                }
                case 3:
                    path = paths[pathIndex];
                    cursor = (AnimStreamEntry *) ((u8 *) cursor + 4);
                    if (path != NULL) {
                        path->flags |= 2;
                        object = (AnimCommandObject *) path->unk8;
                        if ((object != NULL) && (path->flags & 5)) {
                            D_800D6B08[0] = (AnimCameraSource *) object;
                        }
                    }
                    break;
                case 4:
                    path = paths[pathIndex];
                    cursor = (AnimStreamEntry *) ((u8 *) cursor + 4);
                    if (path != NULL) {
                        path->flags &= ~2;
                    }
                    break;
                case 5:
                    cursor = (AnimStreamEntry *) ((u8 *) cursor + 4);
                    animseqHoldPath(pathIndex);
                    break;
                case 6:
                    path = paths[pathIndex];
                    packed = *((u16 *) ((u8 *) cursor + 4));
                    cursor = (AnimStreamEntry *) ((u8 *) cursor + 6);
                    if (path != NULL) {
                        path->unk15 = (s8) (packed >> 8);
                        path->unk1 = packed & 0xFF;
                    }
                    break;
                case 7:
                    path = paths[pathIndex];
                    value = (f32) (*((u16 *) ((u8 *) cursor + 4))) * 0.001f;
                    cursor = (AnimStreamEntry *) ((u8 *) cursor + 6);
                    if (path != NULL) {
                        path->unk10 = value;
                    }
                    break;
                case 8:
                    path = paths[pathIndex];
                    start = (f32) (*((u16 *) ((u8 *) cursor + 4)));
                    end = (f32) (*((u16 *) ((u8 *) cursor + 6)));
                    cursor = (AnimStreamEntry *) ((u8 *) cursor + 8);
                    if (path != NULL) {
                        start *= factor;
                        path->unk2A = (s16) (s32) (end * scale * factor);
                        if (path->unk2A == 0) {
                            path->unk2C = start;
                        } else {
                            path->unk30 =
                                (start - path->unk2C) /
                                (f32) path->unk2A;
                        }
                    }
                    break;
                case 9:
                    cursor = (AnimStreamEntry *) ((u8 *) cursor + 4);
                    animseqLockPath(pathIndex);
                    break;
                case 0xA:
                    cursor = (AnimStreamEntry *) ((u8 *) cursor + 4);
                    animseqUnLockPath(pathIndex);
                    break;
                case 0x20:
                    packed = *((u16 *) ((u8 *) cursor + 4));
                    cursor = (AnimStreamEntry *) ((u8 *) cursor + 6);
                    amSndPlay(packed, NULL);
                    break;
                case 0x21:
                    cursor = (AnimStreamEntry *) ((u8 *) cursor + 4);
                    func_80000510(pathIndex);
                    amTuneResetFade();
                    break;
                case 0x22:
                    duration = *((u16 *) ((u8 *) cursor + 4));
                    value = (f32) duration;
                    cursor = (AnimStreamEntry *) ((u8 *) cursor + 6);
                    func_800005CC(value / hundred - delta,
                                  pathIndex);
                    break;
                case 0x23:
                    cursor = (AnimStreamEntry *) ((u8 *) cursor + 4);
                    amTuneSetVolume(pathIndex);
                    amTuneResetFade();
                    break;
                case 0x24:
                    cursor = (AnimStreamEntry *) ((u8 *) cursor + 4);
                    amAmbientPlay(pathIndex);
                    amAmbientResetFade();
                    break;
                case 0x25:
                    duration = *((u16 *) ((u8 *) cursor + 4));
                    value = (f32) duration;
                    cursor = (AnimStreamEntry *) ((u8 *) cursor + 6);
                    amAmbientSetFade(value / hundred - delta,
                                     pathIndex);
                    break;
                case 0x26:
                    cursor = (AnimStreamEntry *) ((u8 *) cursor + 4);
                    amAmbientSetVolume(pathIndex);
                    amAmbientResetFade();
                    break;
                case 0x27:
                    soundSlot = &D_800D6B18[pathIndex];
                    sound = *soundSlot;
                    packed = *((u16 *) ((u8 *) cursor + 4));
                    cursor = (AnimStreamEntry *) ((u8 *) cursor + 6);
                    if (sound != NULL) {
                        amSndStop(sound);
                        *soundSlot = NULL;
                    }
                    amSndPlay(packed, soundSlot);
                    break;
                case 0x28:
                    soundSlot = &D_800D6B18[pathIndex];
                    sound = *soundSlot;
                    cursor = (AnimStreamEntry *) ((u8 *) cursor + 4);
                    if (sound != NULL) {
                        amSndStop(sound);
                        *soundSlot = NULL;
                    }
                    break;
                case 0x29:
                    path = paths[pathIndex];
                    packed = *((u16 *) ((u8 *) cursor + 4));
                    cursor = (AnimStreamEntry *) ((u8 *) cursor + 6);
                    if ((path != NULL) && (path->unk8 != NULL)) {
                        object = (AnimCommandObject *) path->unk8;
                        if (object->soundHandle != NULL) {
                            func_800031E8(object->soundHandle);
                            object->soundHandle = NULL;
                        }
                        func_80002FE0(packed & 0xFFFF, object->x,
                                      object->y, object->z, 1,
                                      &object->soundHandle);
                    }
                    break;
                case 0x2A:
                    path = paths[pathIndex];
                    cursor = (AnimStreamEntry *) ((u8 *) cursor + 4);
                    if ((path != NULL) && (path->unk8 != NULL)) {
                        object = (AnimCommandObject *) path->unk8;
                        if (object->soundHandle != NULL) {
                            func_800031E8(object->soundHandle);
                            object->soundHandle = NULL;
                        }
                    }
                    break;
                case 0x2B:
                    path = paths[pathIndex];
                    packed = *((u16 *) ((u8 *) cursor + 4));
                    cursor = (AnimStreamEntry *) ((u8 *) cursor + 6);
                    if (path != NULL) {
                        path->unk28 = packed >> 8;
                        path->unk29 = packed & 0xFF;
                    }
                    break;
                case 0x40:
                    packed = *((u16 *) ((u8 *) cursor + 4));
                    packed2 = *((u16 *) ((u8 *) cursor + 6));
                    index = pathIndex;
                    high = (packed >> 8) & 0xFF;
                    low = packed & 0xFF;
                    high2 = (packed2 >> 8) & 0xFF;
                    low2 = packed2 & 0xFF;
                    value = (f32) (*((u16 *) ((u8 *) cursor + 8)));
                    duration = *((u16 *) ((u8 *) cursor + 0xA));
                    target = (f32) duration;
                    cursor = (AnimStreamEntry *) ((u8 *) cursor + 0xC);
                    if ((index & 0xF) == 7) {
                        packed = low != 0 ? 0 : 0xFF;
                        TrapDanglingJump(high, high2, low2, packed,
                                         (s32) (value * factor * scale));
                    } else {
                        color = target != 65535.0f ?
                            target / hundred : -1.0f;
                        if (low != 0) {
                            index |= 0x80;
                        }
                        normalized = value / hundred;
                        func_800498FC(4, normalized, color, high,
                                      high2, low2, index);
                    }
                    break;
                case 0x41:
                    cursor = (AnimStreamEntry *) ((u8 *) cursor + 4);
                    func_8004EED0(pathIndex);
                    break;
                case 0x42:
                    cursor = (AnimStreamEntry *) ((u8 *) cursor + 4);
                    func_8004E99C();
                    break;
                case 0x43:
                    duration = *((u16 *) ((u8 *) cursor + 4));
                    value = (f32) duration;
                    color = (f32) (pathIndex);
                    color = (color / 255.0f) * 65535.0f;
                    cursor = (AnimStreamEntry *) ((u8 *) cursor + 6);
                    changeWeather(0, 0, 0, (s32) color, 0xFFFF,
                                  (s32) (value * factor * scale));
                    break;
                case 0x44:
                    duration = *((u16 *) ((u8 *) cursor + 4));
                    value = (f32) duration;
                    signedValue = *((s16 *) ((u8 *) cursor + 6));
                    packed = *((s16 *) ((u8 *) cursor + 8));
                    packed2 = *((s16 *) ((u8 *) cursor + 0xA));
                    cursor = (AnimStreamEntry *) ((u8 *) cursor + 0xC);
                    unit = 0.00390625f;
                    changeWeather((s32) ((f32) signedValue * unit),
                                  (s32) ((f32) packed * unit),
                                  (s32) ((f32) packed2 * unit), 1, 1,
                                  (s32) value);
                    break;
                case 0x45:
                    duration = *((u16 *) ((u8 *) cursor + 6));
                    value = (f32) duration;
                    packed = *((u16 *) ((u8 *) cursor + 4));
                    packed2 = *((u16 *) ((u8 *) cursor + 8));
                    index = *((u16 *) ((u8 *) cursor + 0xA));
                    high = (packed >> 8) & 0xFF;
                    low = packed & 0xFF;
                    cursor = (AnimStreamEntry *) ((u8 *) cursor + 0xC);
                    func_80014BAC(0, pathIndex, high, low,
                                  packed2, index, value / hundred - delta);
                    break;
                case 0x46: {
                    packedField = *((u16 *) ((u8 *) cursor + 8));
                    radius = *((s16 *) ((u8 *) cursor + 4));
                    height = *((s16 *) ((u8 *) cursor + 6));
                    motionDuration = packedField & 0xFF;
                    motionCount = packedField >> 8;
                    packedField = *((u16 *) ((u8 *) cursor + 0xA));
                    colorB0 = packedField >> 8;
                    colorB1 = packedField & 0xFF;
                    packedField = *((u16 *) ((u8 *) cursor + 0xC));
                    colorB2 = packedField >> 8;
                    colorA0 = packedField & 0xFF;
                    packedField = *((u16 *) ((u8 *) cursor + 0xE));
                    colorA1 = packedField >> 8;
                    packedField &= 0xFF;
                    motionAngle = currentCommand << 8;
                    cursor = (AnimStreamEntry *) ((u8 *) cursor + 0x10);
                    motionX = func_8002A8C0(motionAngle) * radius;
                    motionZ = func_8002A8BC(motionAngle) * radius;
                    animCommandMotionTrap(motionX, height, motionZ,
                                          motionCount * 2, colorB0, colorB1,
                                          colorB2, colorA0, colorA1, packedField,
                                          motionDuration / hundred);
                    break;
                }
                case 0x47:
                    pathIndex = currentCommand & 0xFF;
                    value = (f32) *((u16 *) ((u8 *) cursor + 4));
                    value2 = (f32) *((u16 *) ((u8 *) cursor + 6));
                    target = (f32) *((u16 *) ((u8 *) cursor + 8));
                    cursor = (AnimStreamEntry *) ((u8 *) cursor + 0xA);
                    camStartShake(0, value / hundred, value2 / hundred,
                                  target / hundred, pathIndex);
                    if (D_8007BF0C == 0) {
                        rumbleStart(0, 0x4B, 2.0f);
                    }
                    break;
                case 0x48:
                    index = *((u16 *) ((u8 *) cursor + 4));
                    frame = *((u16 *) ((u8 *) cursor + 6));
                    cursor = (AnimStreamEntry *) ((u8 *) cursor + 8);
                    track = (AnimCommandTrack *) trackGetTrack();
                    if ((track != NULL) && (index < track->count18)) {
                        trackEntry = track->entries[index].texture;
                        if (trackEntry != NULL) {
                            trackEntry->speed12 = (s16) ((frame << 8) /
                                                           6000);
                        }
                    }
                    break;
                case 0x49: {
                    AnimCommandScroll *scroll;
                    s32 scrollX;
                    s32 scrollY;
                    f32 scrollDuration;

                    index = *((u16 *) ((u8 *) cursor + 4));
                    scrollX = *((s16 *) ((u8 *) cursor + 6));
                    scrollY = *((s16 *) ((u8 *) cursor + 8));
                    scrollDuration = *((u16 *) ((u8 *) cursor + 0xA));
                    scrollX = (s32) ((u32) scrollX << 16) / 6000;
                    scrollY = (s32) ((u32) scrollY << 16) / 6000;
                    cursor = (AnimStreamEntry *) ((u8 *) cursor + 0xC);
                    scroll = (AnimCommandScroll *)
                        &D_800D6B58[currentCommand & 7];
                    scroll->textureIndex = index;
                    scroll->duration = (s16) (s32) (scrollDuration *
                                               (60.0f * 0.01f));
                    timer = scroll->duration;
                    scroll->stepX = (scrollX - scroll->x) / timer;
                    scroll->stepY = (scrollY - scroll->y) / timer;
                    break;
                }
                case 0x4A:
                    packed = *((u16 *) ((u8 *) cursor + 4));
                    packed2 = *((u16 *) ((u8 *) cursor + 6));
                    index = pathIndex;
                    cursor = (AnimStreamEntry *) ((u8 *) cursor + 8);
                    if ((packed >> 8) != 0) {
                        TrapDanglingJump(index, packed & 0xFF,
                                         (packed2 >> 8) & 0xFF,
                                         packed2 & 0xFF, packed >> 8);
                    } else {
                        TrapDanglingJump(index);
                    }
                    break;
                case 0x4B:
                    duration = *((u16 *) ((u8 *) cursor + 4));
                    value = (f32) duration;
                    cursor = (AnimStreamEntry *) ((u8 *) cursor + 6);
                    timer = (s32) (value * scale * factor);
                    D_800D6C4E = timer;
                    D_800D6C4C = timer;
                    D_800D6C50 = D_800D6C54;
                    D_800D6C52 = pathIndex;
                    break;
                case 0x4C:
                    packed = *((u16 *) ((u8 *) cursor + 4));
                    packed2 = *((u16 *) ((u8 *) cursor + 6));
                    timer = *((u16 *) ((u8 *) cursor + 8));
                    frame = *((u16 *) ((u8 *) cursor + 0xA));
                    duration = *((u16 *) ((u8 *) cursor + 0xC));
                    cursor = (AnimStreamEntry *) ((u8 *) cursor + 0xE);
                    TrapDanglingJump(pathIndex,
                                     (packed >> 8) & 0xFF, packed & 0xFF,
                                     packed2 & 0xFF00,
                                     (packed2 & 0xFF) << 8,
                                     timer & 0xFF00, (timer & 0xFF) << 8,
                                     frame, duration);
                    break;
                case 0x4D:
                    packed = *((u16 *) ((u8 *) cursor + 4));
                    duration = *((u16 *) ((u8 *) cursor + 6));
                    value = (f32) duration;
                    packed2 = *((u16 *) ((u8 *) cursor + 8));
                    timer = *((u16 *) ((u8 *) cursor + 0xA));
                    frame = *((u16 *) ((u8 *) cursor + 0xC));
                    cursor = (AnimStreamEntry *) ((u8 *) cursor + 0xE);
                    TrapDanglingJump(packed, (s32) value,
                                     packed2, timer, frame >> 8,
                                     frame & 0xFF, pathIndex);
                    break;
                case 0x60:
                    packed = *((u16 *) ((u8 *) cursor + 4));
                    high = (packed >> 8) & 0xFF;
                    low = packed & 0xFF;
                    duration = *((u16 *) ((u8 *) cursor + 6));
                    value = (f32) duration / 16384.0f;
                    duration = *((u16 *) ((u8 *) cursor + 8));
                    value2 = (f32) duration / 16384.0f;
                    path = paths[pathIndex];
                    cursor = (AnimStreamEntry *) ((u8 *) cursor + 0xA);
                    if ((path != NULL) && (path->unk8 != NULL)) {
                        object = (AnimCommandObject *) path->unk8;
                        if (object->model->animationKinds[object->animationIndex] == 1) {
                            object->animationValue = value;
                        } else {
                            func_8005AD64(object, high, -1, value);
                        }
                        path->unkC = value2;
                        path->unk14 = low;
                    }
                    break;
                case 0x61:
                    path = paths[pathIndex];
                    frame = *((u16 *) ((u8 *) cursor + 4));
                    cursor = (AnimStreamEntry *) ((u8 *) cursor + 6);
                    if ((path != NULL) && (path->unk8 != NULL)) {
                        object = (AnimCommandObject *) path->unk8;
                        if (frame < object->model->animationCount) {
                            object->animationIndex = frame;
                        }
                    }
                    break;
                case 0x62:
                    packed = *((u16 *) ((u8 *) cursor + 4));
                    packed2 = *((u16 *) ((u8 *) cursor + 6));
                    objectFlags = ((u32) packed << 16) | (u32) packed2;
                    path = paths[pathIndex];
                    cursor = (AnimStreamEntry *) ((u8 *) cursor + 8);
                    if ((path != NULL) && (path->unk8 != NULL)) {
                        object = (AnimCommandObject *) path->unk8;
                        object->flags80 |= objectFlags;
                    }
                    break;
                case 0x63:
                    packed = *((u16 *) ((u8 *) cursor + 4));
                    packed2 = *((u16 *) ((u8 *) cursor + 6));
                    objectFlags = ((u32) packed << 16) | (u32) packed2;
                    path = paths[pathIndex];
                    cursor = (AnimStreamEntry *) ((u8 *) cursor + 8);
                    if ((path != NULL) && (path->unk8 != NULL)) {
                        object = (AnimCommandObject *) path->unk8;
                        object->flags80 &= ~objectFlags;
                    }
                    break;
                case 0x64:
                    path = paths[pathIndex];
                    cursor = (AnimStreamEntry *) ((u8 *) cursor + 4);
                    if ((path != NULL) && (path->unk8 != NULL)) {
                        object = (AnimCommandObject *) path->unk8;
                        object->flags6 &= ~0x400;
                    }
                    break;
                case 0x65:
                    path = paths[pathIndex];
                    cursor = (AnimStreamEntry *) ((u8 *) cursor + 4);
                    if ((path != NULL) && (path->unk8 != NULL)) {
                        object = (AnimCommandObject *) path->unk8;
                        object->flags6 |= 0x400;
                    }
                    break;
                case 0x66:
                    path = paths[pathIndex];
                    frame = *((u16 *) ((u8 *) cursor + 4));
                    cursor = (AnimStreamEntry *) ((u8 *) cursor + 6);
                    if (path != NULL) {
                        object = (AnimCommandObject *) path->unk8;
                        if (object != NULL) {
                            object->unk88 = frame;
                        }
                    } else {
                        D_800D6C48 = frame;
                    }
                    break;
                case 0x67:
                    path = paths[pathIndex];
                    frame = *((u16 *) ((u8 *) cursor + 4));
                    cursor = (AnimStreamEntry *) ((u8 *) cursor + 6);
                    if ((path != NULL) && (path->unk8 != NULL)) {
                        object = (AnimCommandObject *) path->unk8;
                        if (frame != 0) {
                            object->flags6 |= 4;
                        } else {
                            object->flags6 &= ~4;
                        }
                    }
                    break;
                case 0x68:
                    path = paths[pathIndex];
                    frame = *((u16 *) ((u8 *) cursor + 4));
                    duration = *((u16 *) ((u8 *) cursor + 6));
                    cursor = (AnimStreamEntry *) ((u8 *) cursor + 8);
                    if ((path != NULL) && (path->unk8 != NULL)) {
                        object = (AnimCommandObject *) path->unk8;
                        type = path->unk25;
                        path->unk25 = frame;
                        path->unk27 = 0;
                        path->unk24 = type;
                        path->unk26 = (u8) (s32) ((f32) duration /
                                            hundred * scale);
                        object->state39 = type;
                    }
                    break;
                case 0x6A: {
                    packed = *((u16 *) ((u8 *) cursor + 4));
                    packed2 = *((u16 *) ((u8 *) cursor + 6));
                    frame = *((u16 *) ((u8 *) cursor + 8));
                    duration = *((u16 *) ((u8 *) cursor + 0xA));
                    high = (packed >> 8) & 0xFF;
                    low = packed & 0xFF;
                    high2 = (packed2 >> 8) & 0xFF;
                    low2 = packed2 & 0xFF;
                    value = (f32) duration;
                    path = paths[pathIndex];
                    cursor = (AnimStreamEntry *) ((u8 *) cursor + 0xC);
                    if ((path != NULL) && (path->unk8 != NULL)) {
                        object = (AnimCommandObject *) path->unk8;
                        entry = object->table70;
                        if ((entry != NULL) &&
                            (high < object->model->tableCount28)) {
                            entry = *(void **)
                                ((u8 *) entry + high * 4);
                            animCommandSlotTrap(
                                entry, low, high2, low2, frame, value, 1);
                        }
                    }
                    break;
                }
                case 0x6B: {
                    packed = *((u16 *) ((u8 *) cursor + 4));
                    packed2 = *((u16 *) ((u8 *) cursor + 6));
                    frame = *((u16 *) ((u8 *) cursor + 8));
                    duration = *((u16 *) ((u8 *) cursor + 0xA));
                    high = (packed >> 8) & 0xFF;
                    low = packed & 0xFF;
                    high2 = (packed2 >> 8) & 0xFF;
                    low2 = packed2 & 0xFF;
                    value = (f32) duration;
                    path = paths[pathIndex];
                    cursor = (AnimStreamEntry *) ((u8 *) cursor + 0xC);
                    if ((path != NULL) && (path->unk8 != NULL)) {
                        object = (AnimCommandObject *) path->unk8;
                        entry = object->table74;
                        if ((entry != NULL) &&
                            (high < object->model->tableCount29)) {
                            entry = *(void **)
                                ((u8 *) entry + high * 4);
                            animCommandSlotTrap(
                                entry, low, high2, low2, frame, value, 0);
                        }
                    }
                    break;
                }
                case 0x6C:
                    path = paths[pathIndex];
                    type = *((u8 *) cursor + 5);
                    cursor = (AnimStreamEntry *) ((u8 *) cursor + 6);
                    if ((path != NULL) && (path->unk8 != NULL)) {
                        ((AnimCommandObject *) path->unk8)->unk90 = type;
                    }
                    break;
                case 0x6D:
                    type = *((u8 *) cursor + 5);
                    signedValue = *((s16 *) ((u8 *) cursor + 6));
                    packed = *((s16 *) ((u8 *) cursor + 8));
                    duration = *((u16 *) ((u8 *) cursor + 0xA));
                    value = (f32) signedValue * 0.00390625f;
                    value2 = (f32) packed * 0.00390625f;
                    path = paths[pathIndex];
                    cursor = (AnimStreamEntry *) ((u8 *) cursor + 0xC);
                    if ((path != NULL) && (path->unk8 != NULL)) {
                        object = (AnimCommandObject *) path->unk8;
                        sub = object->sub;
                        if (sub != NULL) {
                            switch (type) {
                                case 0:
                                    sub->type10 = 0;
                                    break;
                                case 1:
                                    sub->type10 = 5;
                                    object->sub->type11 = 1;
                                    break;
                                case 2:
                                case 3:
                                case 4:
                                    sub->type10 = 1;
                                    object->sub->type11 = type - 1;
                                    break;
                                case 5:
                                case 6:
                                case 7:
                                case 8:
                                    sub->type10 = type == 5 ? 0xD : 9;
                                    next = object->sub->next1C;
                                    if ((next != NULL) &&
                                        (next->type10 & 8)) {
                                        next->unkBA = duration;
                                        next->type11 = type == 5 ?
                                            1 : type - 5;
                                        if (duration == 0) {
                                            next->unk0 = object->sub->unk0;
                                            next->unk4 = object->sub->unk4;
                                            next->unkAC =
                                                object->model->scale5C;
                                        } else if (duration == 2) {
                                            next->unk0 = value;
                                            next->unk4 = value;
                                            next->unkAC = value2;
                                        }
                                    }
                                    break;
                            }
                        }
                    }
                    break;
                case 0x6E:
                    packed = *((u16 *) ((u8 *) cursor + 4));
                    packed2 = *((u16 *) ((u8 *) cursor + 6));
                    timer = *((u16 *) ((u8 *) cursor + 8));
                    frame = *((u16 *) ((u8 *) cursor + 0xA));
                    duration = *((u16 *) ((u8 *) cursor + 0xC));
                    value = (f32) duration;
                    path = paths[pathIndex];
                    cursor = (AnimStreamEntry *) ((u8 *) cursor + 0xE);
                    if ((path != NULL) && (path->unk8 != NULL)) {
                        TrapDanglingJump(path->unk8, (packed >> 8) & 0xFF,
                                         packed & 0xFF, packed2, timer,
                                         frame, (s32) (value * factor *
                                                       scale));
                    }
                    break;
                case 0x6F:
                    packed = *((u16 *) ((u8 *) cursor + 4));
                    high = (packed >> 8) & 0xFF;
                    low = packed & 0xFF;
                    frame = *((u16 *) ((u8 *) cursor + 6));
                    path = paths[pathIndex];
                    cursor = (AnimStreamEntry *) ((u8 *) cursor + 8);
                    if ((path != NULL) && (path->unk8 != NULL)) {
                        object = (AnimCommandObject *) path->unk8;
                        if ((object->animationIndex >= 4) ||
                            (object->model->animationValues[object->animationIndex] ==
                             0.0f)) {
                            state = object->model->animationKinds[0];
                        } else {
                            state = object->model->animationKinds[object->animationIndex];
                        }
                        if (state == 1) {
                            object->animationValue = high;
                            path->unkC = (f32) frame / scale;
                        } else if (state == 0) {
                            animation = object->animations[
                                object->animationIndex];
                            if (high < animation->header->count10) {
                                record = animation->records + high;
                                flagsValue = record->flags;
                                header = animation->header;
                                reference = header->recordTable[flagsValue & 0xFF].reference;
                                record->flags = flagsValue & 0xFF1FFFFF;
                                reference->flags = reference->flags &
                                    0xFF1FFFFF;
                                reference->speed12 =
                                    (s16) ((frame << 8) / 6000);
                                if (low == 1) {
                                    record->flags |= 0x800000;
                                    reference->flags |= 0x800000;
                                } else if (low == 2) {
                                    record->flags |= 0x400000;
                                    reference->flags |= 0x400000;
                                }
                            }
                        }
                    }
                    break;
                case 0x74:
                    cursor = (AnimStreamEntry *) ((u8 *) cursor + 4);
                    TrapDanglingJump(1);
                    break;
                case 0x75:
                    cursor = (AnimStreamEntry *) ((u8 *) cursor + 4);
                    TrapDanglingJump(0);
                    break;
                case 0x76:
                    cursor = (AnimStreamEntry *) ((u8 *) cursor + 4);
                    mainSyncNextLevel();
                    break;
                case 0x78:
                    packed = *((u16 *) ((u8 *) cursor + 4));
                    cursor = (AnimStreamEntry *) ((u8 *) cursor + 6);
                    func_80029084(packed, pathIndex);
                    break;
                case 0x79:
                    value = (f32) (pathIndex);
                    duration = *((u16 *) ((u8 *) cursor + 4));
                    value2 = (f32) duration;
                    cursor = (AnimStreamEntry *) ((u8 *) cursor + 6);
                    timer = (s32) (value2 * scale * factor);
                    D_8007D6BC = timer;
                    if (timer > 0) {
                        D_8007D6B8 = (value - D_8007D6B4) /
                                     (f32) timer;
                    } else {
                        D_8007D6B4 = value;
                    }
                    break;
                case 0x7A:
                    packed = *((u16 *) ((u8 *) cursor + 4));
                    index = packed & 0xFFF;
                    high = (currentCommand >> 4) & 0xF;
                    low = (packed >> 12) & 0xF;
                    cursor = (AnimStreamEntry *) ((u8 *) cursor + 6);
                    if (index == 0xFFF) {
                        index = 0;
                    }
                    mainChangeLevel(index, high, low, frontGetMode(), 1,
                                     0);
                    mainSetAnimGroup(currentCommand & 0xF);
                    break;
                case 0x7B:
                    D_8007D6A8 = ((s32) commandDuration * 0x3C) / 100;
                    duration = cursor->duration;
                    value = (f32) duration;
                    cursor = (AnimStreamEntry *) ((u8 *) cursor + 4);
                    D_8007D6AC = value * factor;
                    D_8007D6A4 = (s8) *((u16 *) ((u8 *) cursor - 2));
                    break;
                case 0x7C:
                    cursor = (AnimStreamEntry *) ((u8 *) cursor + 4);
                    joyDisable(currentCommand & 3);
                    break;
                case 0x7D:
                    cursor = (AnimStreamEntry *) ((u8 *) cursor + 4);
                    joyEnable(currentCommand & 3);
                    break;
                case 0x7E:
                    D_8007D6A8 -= ((s32) commandDuration * 0x3C) / 100;
                    duration = cursor->duration;
                    value2 = (f32) duration;
                    cursor = D_8007D698;
                    D_8007D6AC = D_8007D6AC - value2 * factor;
                    break;
                default:
                    break;
            }
        }
        D_8007D69C = cursor;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_800517E0.s")
#endif
/*
 * PROVENANCE: JFG's public src/anim.c supplies the ordered animseqCamera
 * comparison; the body and local layouts are reconstructed from Mickey.
 */
AnimCameraSource *func_80053420(s32 index, AnimCameraTarget *target) {
    AnimCameraSource *source;

    source = D_800D6B08[index];
    if (source != NULL) {
        target->unkC = source->unkC;
        target->unk10 = source->unk10;
        target->unk14 = source->unk14;
        target->unk3E = source->unk2E;
        target->unk0 = 0x8000 - source->unk0;
        target->unk2 = -source->unk2;
        target->unk4 = source->unk4;
        func_80021504(D_8007D6B4, 0);
    }
    return source;
}
/* JFG's ordered anim.c tail and this store establish the tier-D Play name. */
void animseqPlay(void) {
    D_8007D6A4 = 1;
}

void func_800534C0(s32 i) {
    AnimPauseSlot *slot;

    /* The incoming scratch value is replaced before its first use. */
    slot = D_800D6D18;
    i = 4;
    do {
        slot->unkB = 0;
        slot->unk0 = 0;
        slot++;
    } while (i--);
}

void func_800534EC(s32 arg0) {
    AnimPauseSlot *slot;
    s32 i;

    slot = D_800D6D18;
    i = 4;
    do {
        if (slot->unkB > 0) {
            TrapDanglingJump(arg0, slot);
        }
        slot++;
    } while (i--);
}

void func_80002FE0(s32 id, f32 x, f32 y, f32 z, s32 priority,
                   void **handle);
u8 *func_80028F54(void);
void rumbleStart(s32 playerIndex, s32 strength, f32 duration);
void mathOneFloatRPY(ControlTransform *transform, f32 *output);

/*
 * PROVENANCE: JFG's public asm/nonmatchings/hit/hitInitObjectHit.s supplies
 * the structural comparison. Mickey's ABI, field offsets, branches, and
 * generated code remain independently established from Mickey's ROM.
 *
 * Workbench: exact; 198 instructions, -0x78 frame, zero differing words.
 * The local offset vector precedes the model locals, and the live hit pointer
 * follows the model declaration, reproducing the target stack homes.
 */
void func_80053550(HitInitSource *source, s32 kind, s32 mode, s16 rotationX,
                   s16 rotationY, s16 rotationZ, f32 radius, f32 height,
                   f32 arg8, f32 arg9, s32 collisionType, u16 flags) {
    f32 offset[3];
    HitInitModel *model;
    HitInitRecord *hit;
    HitInitHeader *header;
    HitInitDescriptor *descriptor;
    HitInitEntry *entry;
    f32 *floatPosition;
    s16 *vertex;
    s32 entryCount;
    s32 remaining;
    f32 extent;

    hit = source->hit;
    if (hit != NULL) {
        hit->rotationX = rotationX;
        hit->rotationY = rotationY;
        hit->rotationZ = rotationZ;
        hit->collisionType = collisionType;
        hit->kind = kind;
        hit->mode = mode;
        hit->position.x = source->position.x;
        hit->position.y = source->position.y;
        hit->position.z = source->position.z;
        if ((rotationX | rotationY | rotationZ) != 0) {
            hit->localOffset.x = rotationX * source->scale;
            hit->localOffset.y = rotationY * source->scale;
            hit->localOffset.z = rotationZ * source->scale;
            offset[0] = hit->localOffset.x;
            offset[1] = hit->localOffset.y;
            offset[2] = hit->localOffset.z;
            mathOneFloatRPY((ControlTransform *) source, offset);
            hit->position.x += offset[0];
            hit->position.y += offset[1];
            hit->position.z += offset[2];
        }
        hit->basePosition.x = hit->position.x;
        hit->basePosition.y = hit->position.y;
        hit->basePosition.z = hit->position.z;
        hit->radius = source->scale * radius;
        hit->height = source->scale * height;
        if (hit->mode == 0) {
            extent = hit->radius + 5.0f;
            hit->minX = hit->basePosition.x - extent;
            hit->maxX = hit->basePosition.x + extent;
            hit->minZ = hit->basePosition.z - extent;
            hit->maxZ = hit->basePosition.z + extent;
            extent = hit->height + 5.0f;
            hit->minY = hit->basePosition.y - extent;
            hit->maxY = hit->basePosition.y + extent;
        }
        hit->unk68 = arg8;
        hit->unk6C = arg9;
        hit->flags |= flags;
        entryCount = hit->entryCount;
        if (entryCount != 0) {
            entry = hit->entries;
            remaining = entryCount;
            model = *source->model;
            header = model->header;
            floatPosition = model->floatPositions;
            descriptor = header->descriptors;
            do {
                if (header->useFloatPositions != 0) {
                    entry->position.x = floatPosition[0];
                    floatPosition += 3;
                    entry->position.y = floatPosition[-2];
                    entry->position.z = floatPosition[-1];
                } else {
                    vertex = (s16 *)((u8 *)model->vertices +
                                     (descriptor->vertexIndex * 10));
                    offset[0] = vertex[0];
                    offset[1] = vertex[1];
                    offset[2] = vertex[2];
                    mathOneFloatRPY((ControlTransform *) source, offset);
                    entry->position.x = offset[0] + source->position.x;
                    entry->position.y = offset[1] + source->position.y;
                    entry->position.z = offset[2] + source->position.z;
                }
                remaining--;
                entry++;
                extent = descriptor->scale * source->scale;
                descriptor++;
                entry[-1].scaleX = extent;
                entry[-1].scaleY = extent;
            } while (remaining > 0);
        }
    }
}
typedef struct Func538Shape Func538Shape;
typedef struct Func538Object Func538Object;
typedef struct Func538Model Func538Model;
typedef struct Func538Vertex Func538Vertex;
typedef struct Func538Pair Func538Pair;

struct Func538Shape {
    s16 unk0;
    s16 unk2;
    s16 unk4;
    u16 unk6;
    u8 pad8;
    u8 unk9;
    u8 padA[2];
    f32 unkC;
    f32 unk10;
    f32 unk14;
    f32 previous[3];
    f32 position[3];
    f32 displacement[3];
    f32 minimum[3];
    f32 maximum[3];
    f32 unk54;
    f32 unk58;
    f32 unk5C;
    s8 unk60;
    u8 unk61;
    u8 unk62;
    u8 unk63;
    f32 unk64;
    u8 pad68[8];
    Func538Object *unk70;
};

struct Func538Vertex {
    f32 unk0;
    f32 unk4;
    f32 unk8;
};

struct Func538Model {
    u8 pad0[0x40];
    Func538Vertex *unk40;
};

struct Func538Object {
    u8 pad0[0xC];
    f32 unkC;
    f32 unk10;
    f32 unk14;
    u8 pad18[0x22];
    s8 unk3A;
    u8 pad3B[9];
    s16 unk44;
    u8 pad46[2];
    Func538Shape *unk48;
    u8 pad4C[0x1C];
    Func538Model **unk68;
    u8 pad6C[0x25];
    u8 unk91;
};

struct Func538Pair {
    Func538Object *first;
    Func538Object *second;
    f32 fraction;
    AnimVec3f normal;
};

typedef struct AnimCollisionShape {
    u8 pad0[6];
    u16 flags;
    u8 shape;
    u8 pad9[0xF];
    AnimVec3f position;
    AnimVec3f edge;
    AnimVec3f vector;
    u8 pad3C[0x1C];
    f32 radius;
    f32 height;
} AnimCollisionShape;

typedef struct AnimCollisionResult {
    s32 object;
    s32 value;
    f32 fraction;
    AnimVec3f normal;
} AnimCollisionResult;

extern Func538Object *D_800D6D60[0x100];
extern Func538Object *D_800D7160[0x100];
extern Func538Pair D_800D7560[];

#define FUNC538_PAIR(index) (&D_800D7560[index])

extern Func538Object **func_8000572C(s32 *start, s32 *end);
s32 func_80054B3C(s32 arg0, AnimCollisionShape *arg1,
                  s32 arg2, AnimCollisionShape *arg3,
                  AnimCollisionResult *arg4);
void func_80055104(HitCopyState *first, HitCopyState *second, f32 scale);
void func_80055970(HitCopyState *first, HitCopyState *second, f32 unused);
void func_80055B24(HitCopyState *first, HitCopyState *second, f32 unused);
void func_80055F64(HitCopyState *first, HitCopyState *second, f32 unused);
void func_800560D0(HitCopyState *first, HitCopyState *second, f32 unused);
s32 func_800563B4(s32 object, AnimCollisionShape *first, s32 value,
                  AnimCollisionShape *second, AnimCollisionResult *result);
void func_80056DD8(HitCopyState *first, HitCopyState *second,
                   AnimVec3f *normal, f32 timeStep);
void func_8005716C(HitCopyState *state, void *unused, AnimVec3f *normal,
                   f32 timeStep);
void func_800573C8(HitOverlapState *state, HitOverlapVolume *other,
                   HitOverlapState *trigger, HitOverlapVolume *volume);

/*
 * PROVENANCE: JFG's public hit-update assembly supplies the broad collision
 * update role and case ordering; Mickey's fields, globals, and call targets
 * remain authoritative for this reconstruction.
 */
#ifdef NON_MATCHING
/* NON_MATCHING: collision-update reconstruction; Mickey-only field/ABI audit.
 * Reuse phase-local scratch values and order stack homes from the target
 * accesses. The vector result carrier preserves the repeated float dataflow. */
void func_80053868(s32 updateRate) {
    f32 remainingTime;
    f32 fraction;
    f32 low;
    f32 high;
    f32 extent;
    Func538Pair *selectedPair;
    Func538Pair *pairCursor;
    Func538Vertex *vertex;
    f32 offset[3];
    Func538Object **otherCursor;
    Func538Object **movingCursor;
    Func538Object *firstObject;
    Func538Object *secondObject;
    Func538Shape *firstShape;
    Func538Shape *secondShape;
    s32 firstIndex;
    s32 objectCount;
    s32 fixedCount;
    s32 movingCount;
    s32 pairCount;
    s32 pairIndex;
    s32 overlaps;
    s32 iteration;
    s32 i;
    s32 j;
    s32 axis;
    s32 remainder;
    s32 result;
    s16 firstKind;
    s16 secondKind;
    u8 kind;

    fraction = (f32) updateRate;
    remainingTime = fraction;
    otherCursor = func_8000572C(&firstIndex, &objectCount);
    fixedCount = 0;
    movingCount = 0;
    i = firstIndex;
    if (firstIndex < objectCount) {
        movingCursor = &otherCursor[firstIndex];
        do {
            firstObject = *movingCursor;
            firstShape = firstObject->unk48;
            if ((firstObject->unk91 == 0) && (firstShape != NULL)) {
                kind = firstShape->unk9;
                if ((kind == 2) || (kind == 1)) {
                    if (firstShape->unk60 != -1) {
                        firstShape->previous[0] = firstShape->position[0];
                        firstShape->previous[1] = firstShape->position[1];
                        vertex = firstObject->unk68[firstObject->unk3A]->unk40 + firstShape->unk60;
                        firstShape->previous[2] = firstShape->position[2];
                        firstShape->position[0] = vertex->unk0;
                        firstShape->position[1] = vertex->unk4;
                        firstShape->position[2] = vertex->unk8;
                    } else {
                        firstShape->previous[0] = firstShape->position[0];
                        firstShape->previous[1] = firstShape->position[1];
                        firstShape->previous[2] = firstShape->position[2];
                        firstShape->position[0] = firstObject->unkC;
                        firstShape->position[1] = firstObject->unk10;
                        firstShape->position[2] = firstObject->unk14;
                        if ((firstShape->unk0 | firstShape->unk2 | firstShape->unk4) != 0) {
                            offset[0] = firstShape->unkC;
                            offset[1] = firstShape->unk10;
                            offset[2] = firstShape->unk14;
                            mathOneFloatRPY((ControlTransform *) firstObject, &offset[0]);
                            firstShape->position[0] += offset[0];
                            firstShape->position[1] += offset[1];
                            firstShape->position[2] += offset[2];
                        }
                        firstShape->position[1] += firstShape->unk54;
                    }
                    kind = firstShape->unk9;
                    if (kind == 2) {
                        extent = firstShape->unk58 + 5.0f;
                        for (axis = 0; axis < 3; axis++) {
                            low = firstShape->previous[axis];
                            high = firstShape->position[axis];
                            if (low < high) {
                                firstShape->minimum[axis] = low;
                                firstShape->maximum[axis] = high;
                            } else {
                                firstShape->minimum[axis] = high;
                                firstShape->maximum[axis] = low;
                            }
                            firstShape->minimum[axis] -= extent;
                            firstShape->maximum[axis] += extent;
                        }
                    } else if (kind == 1) {
                        low = firstShape->position[0];
                        fraction = firstShape->position[2];
                        extent = firstShape->unk58 + 5.0f;
                        high = firstShape->position[1];
                        firstShape->minimum[0] = (low - extent);
                        firstShape->maximum[0] = (low + extent);
                        firstShape->minimum[2] = (fraction - extent);
                        firstShape->maximum[2] = (fraction + extent);
                        extent = firstShape->unk5C + 5.0f;
                        firstShape->minimum[1] = (high - extent);
                        firstShape->maximum[1] = (high + extent);
                    }
                }
                if (firstShape->unk6 & 1) {
                    kind = firstShape->unk9;
                    if (((kind == 0) || (kind == 1)) && (fixedCount < 0x100)) {
                        D_800D6D60[fixedCount] = firstObject;
                        fixedCount += 1;
                    } else if ((kind == 2) && (movingCount < 0x100)) {
                        D_800D7160[movingCount] = firstObject;
                        movingCount += 1;
                    }
                }
                firstShape->unk61 = 0;
                firstShape->unk62 = 0;
                firstShape->unk63 = 0;
                firstShape->unk64 = 0.0f;
            }
            i += 1;
            movingCursor++;
        } while (i < objectCount);
    }
    iteration = 0;
    while (remainingTime > 0.0f) {
        fraction = 1.0f;
        pairCount = 0;
        i = 0;
        pairIndex = -1;
        if (movingCount > 0) {
            movingCursor = D_800D7160;
            do {
                firstShape = (*movingCursor)->unk48;
                firstShape->displacement[0] = firstShape->position[0] - firstShape->previous[0];
                firstShape->displacement[1] = firstShape->position[1] - firstShape->previous[1];
                firstShape->displacement[2] = firstShape->position[2] - firstShape->previous[2];
                extent = firstShape->unk58 + 5.0f;
                for (axis = 0; axis < 3; axis++) {
                    low = firstShape->previous[axis];
                    high = firstShape->position[axis];
                    if (low < high) {
                        firstShape->minimum[axis] = low;
                        firstShape->maximum[axis] = high;
                    } else {
                        firstShape->minimum[axis] = high;
                        firstShape->maximum[axis] = low;
                    }
                    firstShape->minimum[axis] -= extent;
                    firstShape->maximum[axis] += extent;
                }
                i += 1;
                movingCursor++;
            } while (i != movingCount);
            i = 0;
        }
        movingCursor = D_800D7160;
        if (movingCount > 0) {
            do {
                firstObject = *movingCursor;
                j = 0;
                firstShape = firstObject->unk48;
                if (fixedCount > 0) {
                    otherCursor = D_800D6D60;
                    do {
                        secondObject = *otherCursor;
                        secondShape = secondObject->unk48;
                        if ((firstShape->unk6 & 1) && (secondShape->unk6 & 1) && (secondObject != firstShape->unk70)) {
                            if (firstObject != secondShape->unk70) {
                                axis = 0;
                                overlaps = 1;
                                do {
                                    high = firstShape->minimum[axis];
                                    low = secondShape->minimum[axis];
                                    if ((high < low) && (firstShape->maximum[axis] < low)) {
                                        overlaps = 0;
                                    } else {
                                        low = secondShape->maximum[axis];
                                        if ((low < high) && (low < firstShape->maximum[axis])) {
                                            overlaps = 0;
                                        }
                                    }
                                    axis++;
                                } while ((axis < 3) && overlaps);
                                if (overlaps != 0) {
                                    result = func_800563B4((s32) firstObject, (AnimCollisionShape *) firstShape, (s32) secondObject, (AnimCollisionShape *) secondShape, (AnimCollisionResult *) FUNC538_PAIR(pairCount));
                                    if ((result == 0) && (secondShape->unk9 == 1)) {
                                        func_800573C8((HitOverlapState *) firstObject, (HitOverlapVolume *) firstShape, (HitOverlapState *) secondObject, (HitOverlapVolume *) secondShape);
                                    } else if (result == 1) {
                                        if (pairCount < 0xF) {
                                            pairCount += 1;
                                        }
                                    } else if (result == 2) {
                                        firstKind = firstObject->unk44;
                                        if (firstKind == 0x40U) {
                                            TrapDanglingJump(firstObject, 1);
                                        } else if (firstKind == 0x39U) {
                                            TrapDanglingJump(firstObject, 5);
                                        } else if (firstKind == 0x3AU) {
                                            TrapDanglingJump(firstObject, 5);
                                        }
                                    }
                                }
                            }
                        }
                        j += 1;
                        otherCursor++;
                    } while (j != fixedCount);
                }
                j = i + 1;
                if (j < movingCount) {
                    otherCursor = &D_800D7160[j];
                    do {
                        secondObject = *otherCursor;
                        secondShape = secondObject->unk48;
                        if ((firstShape->unk6 & 1) && (secondShape->unk6 & 1) && (secondObject != firstShape->unk70)) {
                            if (firstObject != secondShape->unk70) {
                                axis = 0;
                                overlaps = 1;
                                do {
                                    high = firstShape->minimum[axis];
                                    low = secondShape->minimum[axis];
                                    if ((high < low) && (firstShape->maximum[axis] < low)) {
                                        overlaps = 0;
                                    } else {
                                        low = secondShape->maximum[axis];
                                        if ((low < high) && (low < firstShape->maximum[axis])) {
                                            overlaps = 0;
                                        }
                                    }
                                    axis++;
                                } while ((axis < 3) && overlaps);
                                if ((overlaps != 0) && (func_80054B3C((s32) firstObject, (AnimCollisionShape *) firstShape, (s32) secondObject, (AnimCollisionShape *) secondShape, (AnimCollisionResult *) FUNC538_PAIR(pairCount)) != 0) && (pairCount < 0xF)) {
                                    pairCount += 1;
                                }
                            }
                        }
                        j += 1;
                        otherCursor++;
                    } while (j != movingCount);
                }
                i++;
                movingCursor++;
            } while (i != movingCount);
            i = 0;
        }
        if (pairCount > 0) {
            remainder = pairCount & 3;
            if (remainder != 0) {
                pairCursor = FUNC538_PAIR(i);
                do {
                    low = pairCursor->fraction;
                    if (low <= fraction) {
                        fraction = low;
                        pairIndex = i;
                    }
                    i += 1;
                    pairCursor++;
                } while (i != remainder);
            }
            if (i < pairCount) {
                pairCursor = FUNC538_PAIR(i);
                do {
                    low = pairCursor->fraction;
                    if (low <= fraction) {
                        fraction = low;
                        pairIndex = i;
                    }
                    if (pairCursor[1].fraction <= fraction) {
                        fraction = pairCursor[1].fraction;
                        pairIndex = i + 1;
                    }
                    if (pairCursor[2].fraction <= fraction) {
                        fraction = pairCursor[2].fraction;
                        pairIndex = i + 2;
                    }
                    if (pairCursor[3].fraction <= fraction) {
                        fraction = pairCursor[3].fraction;
                        pairIndex = i + 3;
                    }
                    i += 4;
                    pairCursor += 4;
                } while (i != pairCount);
            }
        }
        if (pairIndex != -1) {
            selectedPair = FUNC538_PAIR(pairIndex);
            i = 0;
            remainder = movingCount & 3;
            remainingTime *= 1.0f - selectedPair->fraction;
            if (movingCount > 0) {
                if (remainder != 0) {
                    movingCursor = &D_800D7160[i];
                    i++;
                    firstObject = *movingCursor;
                    if (i < remainder) {
                        do {
                            firstShape = firstObject->unk48;
                            i += 1;
                            movingCursor++;
                            fraction = firstShape->displacement[0] * selectedPair->fraction;
                            fraction = firstShape->previous[0] + fraction;
                            firstShape->previous[0] = fraction;
                            fraction = firstShape->displacement[1] * selectedPair->fraction;
                            fraction = firstShape->previous[1] + fraction;
                            firstShape->previous[1] = fraction;
                            fraction = firstShape->displacement[2] * selectedPair->fraction;
                            fraction = firstShape->previous[2] + fraction;
                            firstShape->previous[2] = fraction;
                            firstObject = *movingCursor;
                        } while (i != remainder);
                    }
                    firstShape = firstObject->unk48;
                    fraction = firstShape->displacement[0] * selectedPair->fraction;
                    fraction = firstShape->previous[0] + fraction;
                    firstShape->previous[0] = fraction;
                    fraction = firstShape->displacement[1] * selectedPair->fraction;
                    fraction = firstShape->previous[1] + fraction;
                    firstShape->previous[1] = fraction;
                    fraction = firstShape->displacement[2] * selectedPair->fraction;
                    fraction = firstShape->previous[2] + fraction;
                    firstShape->previous[2] = fraction;
                }
                if (i < movingCount) {
                    otherCursor = &D_800D7160[movingCount];
                    movingCursor = &D_800D7160[i + 4];
                    firstObject = movingCursor[-4];
                    if (movingCursor != otherCursor) {
                        do {
                            firstShape = firstObject->unk48;
                            movingCursor += 4;
                            fraction = firstShape->displacement[0] * selectedPair->fraction;
                            fraction = firstShape->previous[0] + fraction;
                            firstShape->previous[0] = fraction;
                            fraction = firstShape->displacement[1] * selectedPair->fraction;
                            fraction = firstShape->previous[1] + fraction;
                            firstShape->previous[1] = fraction;
                            fraction = firstShape->displacement[2] * selectedPair->fraction;
                            fraction = firstShape->previous[2] + fraction;
                            firstShape->previous[2] = fraction;
                            firstShape = movingCursor[-7]->unk48;
                            fraction = firstShape->displacement[0] * selectedPair->fraction;
                            fraction = firstShape->previous[0] + fraction;
                            firstShape->previous[0] = fraction;
                            fraction = firstShape->displacement[1] * selectedPair->fraction;
                            fraction = firstShape->previous[1] + fraction;
                            firstShape->previous[1] = fraction;
                            fraction = firstShape->displacement[2] * selectedPair->fraction;
                            fraction = firstShape->previous[2] + fraction;
                            firstShape->previous[2] = fraction;
                            firstShape = movingCursor[-6]->unk48;
                            fraction = firstShape->displacement[0] * selectedPair->fraction;
                            fraction = firstShape->previous[0] + fraction;
                            firstShape->previous[0] = fraction;
                            fraction = firstShape->displacement[1] * selectedPair->fraction;
                            fraction = firstShape->previous[1] + fraction;
                            firstShape->previous[1] = fraction;
                            fraction = firstShape->displacement[2] * selectedPair->fraction;
                            fraction = firstShape->previous[2] + fraction;
                            firstShape->previous[2] = fraction;
                            firstShape = movingCursor[-5]->unk48;
                            fraction = firstShape->displacement[0] * selectedPair->fraction;
                            fraction = firstShape->previous[0] + fraction;
                            firstShape->previous[0] = fraction;
                            fraction = firstShape->displacement[1] * selectedPair->fraction;
                            fraction = firstShape->previous[1] + fraction;
                            firstShape->previous[1] = fraction;
                            fraction = firstShape->displacement[2] * selectedPair->fraction;
                            fraction = firstShape->previous[2] + fraction;
                            firstShape->previous[2] = fraction;
                            firstObject = movingCursor[-4];
                        } while (movingCursor != otherCursor);
                    }
                    firstShape = firstObject->unk48;
                    fraction = firstShape->displacement[0] * selectedPair->fraction;
                    fraction = firstShape->previous[0] + fraction;
                    firstShape->previous[0] = fraction;
                    fraction = firstShape->displacement[1] * selectedPair->fraction;
                    fraction = firstShape->previous[1] + fraction;
                    firstShape->previous[1] = fraction;
                    fraction = firstShape->displacement[2] * selectedPair->fraction;
                    fraction = firstShape->previous[2] + fraction;
                    firstShape->previous[2] = fraction;
                    firstShape = movingCursor[-3]->unk48;
                    fraction = firstShape->displacement[0] * selectedPair->fraction;
                    fraction = firstShape->previous[0] + fraction;
                    firstShape->previous[0] = fraction;
                    fraction = firstShape->displacement[1] * selectedPair->fraction;
                    fraction = firstShape->previous[1] + fraction;
                    firstShape->previous[1] = fraction;
                    fraction = firstShape->displacement[2] * selectedPair->fraction;
                    fraction = firstShape->previous[2] + fraction;
                    firstShape->previous[2] = fraction;
                    firstShape = movingCursor[-2]->unk48;
                    fraction = firstShape->displacement[0] * selectedPair->fraction;
                    fraction = firstShape->previous[0] + fraction;
                    firstShape->previous[0] = fraction;
                    fraction = firstShape->displacement[1] * selectedPair->fraction;
                    fraction = firstShape->previous[1] + fraction;
                    firstShape->previous[1] = fraction;
                    fraction = firstShape->displacement[2] * selectedPair->fraction;
                    fraction = firstShape->previous[2] + fraction;
                    firstShape->previous[2] = fraction;
                    firstShape = movingCursor[-1]->unk48;
                    fraction = firstShape->displacement[0] * selectedPair->fraction;
                    fraction = firstShape->previous[0] + fraction;
                    firstShape->previous[0] = fraction;
                    fraction = firstShape->displacement[1] * selectedPair->fraction;
                    fraction = firstShape->previous[1] + fraction;
                    firstShape->previous[1] = fraction;
                    fraction = firstShape->displacement[2] * selectedPair->fraction;
                    fraction = firstShape->previous[2] + fraction;
                    firstShape->previous[2] = fraction;
                }
            }
            firstObject = selectedPair->first;
            secondObject = selectedPair->second;
            firstKind = firstObject->unk44;
            secondKind = secondObject->unk44;
            if ((firstKind == 1) && (secondKind == 1)) {
                func_80055104((HitCopyState *) firstObject, (HitCopyState *) secondObject, remainingTime);
            } else if ((firstKind == 0x40) && (secondKind == 0x40)) {
                TrapDanglingJump(firstObject, 1);
                TrapDanglingJump(secondObject, 2);
            } else if ((firstKind == 0x39) && (secondKind == 0x39)) {
                func_800560D0((HitCopyState *) firstObject, (HitCopyState *) secondObject, remainingTime);
            } else if ((firstKind == 0x3A) && (secondKind == 0x3A)) {
                func_80056274((HitCopyState *) firstObject, (HitCopyState *) secondObject, remainingTime);
            } else if ((firstKind == 1) && (secondKind == 0x40)) {
                func_800557F8((HitCopyState *) firstObject, (HitCopyState *) secondObject, remainingTime);
            } else if ((firstKind == 0x40) && (secondKind == 1)) {
                func_800557F8((HitCopyState *) secondObject, (HitCopyState *) firstObject, remainingTime);
            } else if ((firstKind == 1) && (secondKind == 0x39)) {
                func_80055970((HitCopyState *) firstObject, (HitCopyState *) secondObject, remainingTime);
            } else if ((firstKind == 0x39) && (secondKind == 1)) {
                func_80055970((HitCopyState *) secondObject, (HitCopyState *) firstObject, remainingTime);
            } else if ((firstKind == 1) && (secondKind == 0x3A)) {
                func_80055B24((HitCopyState *) firstObject, (HitCopyState *) secondObject, remainingTime);
            } else if ((firstKind == 0x3A) && (secondKind == 1)) {
                func_80055B24((HitCopyState *) secondObject, (HitCopyState *) firstObject, remainingTime);
            } else if ((firstKind == 0x40) && (secondKind == 0x39)) {
                func_80055D08((HitCopyState *) firstObject, (HitCopyState *) secondObject, remainingTime);
            } else if ((firstKind == 0x39) && (secondKind == 0x40)) {
                func_80055D08((HitCopyState *) secondObject, (HitCopyState *) firstObject, remainingTime);
            } else if ((firstKind == 0x40) && (secondKind == 0x3A)) {
                func_80055E50((HitCopyState *) firstObject, (HitCopyState *) secondObject, remainingTime);
            } else if ((firstKind == 0x3A) && (secondKind == 0x40)) {
                func_80055E50((HitCopyState *) secondObject, (HitCopyState *) firstObject, remainingTime);
            } else if ((firstKind == 0x39) && (secondKind == 0x3A)) {
                func_80055F64((HitCopyState *) firstObject, (HitCopyState *) secondObject, remainingTime);
            } else if ((firstKind == 0x3A) && (secondKind == 0x39)) {
                func_80055F64((HitCopyState *) secondObject, (HitCopyState *) firstObject, remainingTime);
            } else if ((firstKind == 1) && ((secondKind == 3) || (secondKind == 0x43) || (secondKind == 0x52))) {
                func_80056DD8((HitCopyState *) firstObject, (HitCopyState *) secondObject, &selectedPair->normal, remainingTime);
            } else if ((firstKind == 0x40) && ((secondKind == 3) || (secondKind == 0x43) || (secondKind == 0x52))) {
                func_8005716C((HitCopyState *) firstObject, (HitCopyState *) secondObject, &selectedPair->normal, remainingTime);
            } else if ((firstKind == 0x39) && ((secondKind == 3) || (secondKind == 0x43) || (secondKind == 0x52))) {
                func_800572AC((HitCopyState *) firstObject, (HitCopyState *) secondObject, &selectedPair->normal, remainingTime);
            } else if ((firstKind == 0x3A) && ((secondKind == 3) || (secondKind == 0x43) || (secondKind == 0x52))) {
                func_80057350((HitCopyState *) firstObject, (HitCopyState *) secondObject, &selectedPair->normal, remainingTime);
            }
            iteration++;
            if (iteration >= 0xB) {
                i = 0;
                if (movingCount > 0) {
                    remainder = movingCount & 3;
                    if (remainder != 0) {
                        movingCursor = &D_800D7160[i];
                        i++;
                        firstObject = *movingCursor;
                        if (i < remainder) {
                            do {
                                firstShape = firstObject->unk48;
                                i += 1;
                                movingCursor++;
                                firstShape->position[0] = firstShape->previous[0];
                                firstShape->position[1] = firstShape->previous[1];
                                firstShape->position[2] = firstShape->previous[2];
                                firstObject = *movingCursor;
                            } while (i != remainder);
                        }
                        firstShape = firstObject->unk48;
                        firstShape->position[0] = firstShape->previous[0];
                        firstShape->position[1] = firstShape->previous[1];
                        firstShape->position[2] = firstShape->previous[2];
                    }
                    if (i < movingCount) {
                        otherCursor = &D_800D7160[movingCount];
                        movingCursor = &D_800D7160[i + 4];
                        firstShape = movingCursor[-4]->unk48;
                        if (movingCursor != otherCursor) {
                            do {
                                firstShape->position[0] = firstShape->previous[0];
                                firstShape->position[1] = firstShape->previous[1];
                                firstShape->position[2] = firstShape->previous[2];
                                firstObject = movingCursor[-3];
                                movingCursor += 4;
                                firstShape = firstObject->unk48;
                                firstShape->position[0] = firstShape->previous[0];
                                firstShape->position[1] = firstShape->previous[1];
                                firstShape->position[2] = firstShape->previous[2];
                                firstShape = movingCursor[-6]->unk48;
                                firstShape->position[0] = firstShape->previous[0];
                                firstShape->position[1] = firstShape->previous[1];
                                firstShape->position[2] = firstShape->previous[2];
                                firstShape = movingCursor[-5]->unk48;
                                firstShape->position[0] = firstShape->previous[0];
                                firstShape->position[1] = firstShape->previous[1];
                                firstShape->position[2] = firstShape->previous[2];
                                firstShape = movingCursor[-4]->unk48;
                            } while (movingCursor != otherCursor);
                        }
                        firstShape->position[0] = firstShape->previous[0];
                        firstShape->position[1] = firstShape->previous[1];
                        firstShape->position[2] = firstShape->previous[2];
                        firstShape = movingCursor[-3]->unk48;
                        firstShape->position[0] = firstShape->previous[0];
                        firstShape->position[1] = firstShape->previous[1];
                        firstShape->position[2] = firstShape->previous[2];
                        firstShape = movingCursor[-2]->unk48;
                        firstShape->position[0] = firstShape->previous[0];
                        firstShape->position[1] = firstShape->previous[1];
                        firstShape->position[2] = firstShape->previous[2];
                        firstShape = movingCursor[-1]->unk48;
                        firstShape->position[0] = firstShape->previous[0];
                        firstShape->position[1] = firstShape->previous[1];
                        firstShape->position[2] = firstShape->previous[2];
                    }
                }
                remainingTime = 0.0f;
            }
        } else {
            remainingTime = 0.0f;
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_80053868.s")
#endif
s32 func_80012234(AnimVec3f *point, AnimVec3f *direction,
                  AnimVec3f *origin, AnimVec3f *planeDirection,
                  f32 radius, f32 *minimum, f32 *maximum);
s32 func_80012574(AnimVec3f *origin, AnimVec3f *direction,
                  AnimVec3f *center, f32 radius, f32 *minimum,
                  f32 *maximum);
s32 func_800131AC(AnimVec3f *origin, AnimVec3f *direction,
                  AnimVec3f *minimum, AnimVec3f *maximum,
                  f32 *near, f32 *far);
extern f32 D_8008420C;

#ifdef NON_MATCHING
/* PROVENANCE: JFG's public hit/collision code supplies the capsule and
 * endpoint-overlap role; Mickey's collision-shape offsets are authoritative. */
/* Workbench verdict: structure-mismatch, 365 differing words; first mismatch is at +0x0. */
/* Target is 370 instructions/frame -216; candidate is 371 instructions/frame -392. */
/* Remaining gap is frame/allocation: all three sqrtf identities resolve, but none has the target offset. */
/*
 * Swept-sphere intersection between two collision shapes, reconstructed from
 * Mickey's own assembly. The quadratic's three coefficients are written out
 * term by term because the target is: it never forms a difference vector, it
 * squares and cross-multiplies the six coordinates and the six doubled
 * coordinates directly.
 *
 * Plateau: 380 of 370 words, 374 differing from +0x8, frame 0xD8 -- the
 * target's. The previous candidate was an untranslated m2c draft whose ~40
 * single-use temporaries each reserved a stack home; that alone held the
 * frame at 0x188, 176 bytes above the target. Inlining them and letting the
 * dead coordinate carriers hold the later quadratic values reaches the exact
 * frame. What is left is ten words of surplus code: the target keeps arg1 in
 * a saved register and homes arg0, and it needs only one callee-saved
 * floating-point register where this candidate needs two.
 */
s32 func_80054B3C(s32 arg0, AnimCollisionShape *arg1,
                  s32 arg2, AnimCollisionShape *arg3,
                  AnimCollisionResult *arg4) {
    AnimVec3f *firstPoint;
    AnimVec3f *secondPoint;
    f32 radiusSq;
    f32 quadA;
    f32 quadB;
    f32 quadC;
    f32 discriminant;
    f32 fraction;
    f32 stepX1;
    f32 stepY1;
    f32 stepZ1;
    f32 stepX2;
    f32 stepY2;
    f32 stepZ2;
    f32 x1;
    f32 y1;
    f32 z1;
    f32 x2;
    f32 y2;
    f32 z2;
    f32 twoX1;
    f32 twoY1;
    f32 twoZ1;
    f32 twoX2;
    f32 twoY2;
    f32 twoZ2;
    s32 hit;

    radiusSq = arg1->radius + arg3->radius;
    hit = 0;
    firstPoint = &arg1->position;
    radiusSq = radiusSq * radiusSq;
    secondPoint = &arg3->position;
    if ((arg1->flags & 2) || (arg3->flags & 2)) {
        stepX1 = secondPoint->x - firstPoint->x;
        stepY1 = secondPoint->y - firstPoint->y;
        stepZ1 = secondPoint->z - firstPoint->z;
        if (((stepX1 * stepX1) + (stepY1 * stepY1) + (stepZ1 * stepZ1)) <=
            radiusSq) {
            arg4->object = arg0;
            arg4->value = arg2;
            arg4->fraction = 0.0f;
            return 1;
        }
    }
    firstPoint = &arg1->vector;
    stepX1 = firstPoint->x;
    secondPoint = &arg3->vector;
    stepX2 = secondPoint->x;
    stepY1 = firstPoint->y;
    stepY2 = secondPoint->y;
    stepZ1 = firstPoint->z;
    stepZ2 = secondPoint->z;
    secondPoint = &arg3->position;
    firstPoint = &arg1->position;
    quadA = (stepZ2 * stepZ2) +
            ((stepZ1 * stepZ1) - (2.0f * stepZ1 * stepZ2)) +
            (((stepX1 * stepX1) - (2.0f * stepX1 * stepX2)) +
             (stepX2 * stepX2) +
             (((stepY1 * stepY1) - (2.0f * stepY1 * stepY2)) +
              (stepY2 * stepY2)));
    z2 = secondPoint->z;
    twoZ2 = 2.0f * z2;
    z1 = firstPoint->z;
    twoZ1 = 2.0f * z1;
    x1 = firstPoint->x;
    twoX1 = 2.0f * x1;
    x2 = secondPoint->x;
    twoX2 = 2.0f * x2;
    y1 = firstPoint->y;
    twoY1 = 2.0f * y1;
    y2 = secondPoint->y;
    twoY2 = y2 + y2;
    quadB = ((stepZ2 * twoZ2) +
             (((twoZ1 * stepZ1) - (twoZ1 * stepZ2)) - (twoZ2 * stepZ1))) +
            ((((twoX1 * stepX1) - (twoX1 * stepX2)) - (twoX2 * stepX1)) +
             (twoX2 * stepX2) +
             ((((twoY1 * stepY1) - (twoY1 * stepY2)) - (twoY2 * stepY1)) +
              (twoY2 * stepY2)));
    quadC = (z2 * z2) + ((z1 * z1) - (twoZ1 * z2)) +
            (((x1 * x1) - (twoX1 * x2)) + (x2 * x2) +
             (((y1 * y1) - (twoY1 * y2)) + (y2 * y2)));
    if (quadA != 0.0f) {
        twoZ2 = 4.0f * quadA;
        discriminant = twoZ2 * (quadC - radiusSq);
        twoX2 = quadB * quadB;
        if (discriminant < twoX2) {
            discriminant = sqrtf(twoX2 - discriminant);
            hit = 0;
            quadB = -quadB;
            quadA = 2.0f * quadA;
            fraction = (quadB - discriminant) / quadA;
            if ((fraction >= 0.0f) && (fraction <= 1.0f)) {
                discriminant = twoZ2 * (quadC - (radiusSq + 83.0f));
                if (discriminant < twoX2) {
                    fraction = (quadB - sqrtf(twoX2 - discriminant)) / quadA;
                    hit = 1;
                    if (fraction > 1.0f) {
                        fraction = 1.0f;
                    } else if (fraction < 0.0f) {
                        fraction = 0.0f;
                    }
                    arg4->object = arg0;
                    arg4->value = arg2;
                    arg4->fraction = fraction;
                }
            } else {
                stepX1 = arg3->edge.x - arg1->edge.x;
                stepY1 = arg3->edge.y - arg1->edge.y;
                stepZ1 = arg3->edge.z - arg1->edge.z;
                if (((stepX1 * stepX1) + (stepY1 * stepY1) +
                     (stepZ1 * stepZ1)) <= radiusSq) {
                    discriminant = twoZ2 * (quadC - (radiusSq + 83.0f));
                    if (discriminant < twoX2) {
                        fraction =
                            (quadB - sqrtf(twoX2 - discriminant)) / quadA;
                        hit = 1;
                        if (fraction > 1.0f) {
                            fraction = 1.0f;
                        } else if (fraction < 0.0f) {
                            fraction = 0.0f;
                        }
                        arg4->object = arg0;
                        arg4->value = arg2;
                        arg4->fraction = fraction;
                    }
                }
            }
        }
    }
    if (hit == 0) {
        stepX1 = arg3->edge.x - arg1->edge.x;
        stepY1 = arg3->edge.y - arg1->edge.y;
        stepZ1 = arg3->edge.z - arg1->edge.z;
        if (((stepX1 * stepX1) + (stepY1 * stepY1) + (stepZ1 * stepZ1)) <=
            radiusSq) {
            arg4->object = arg0;
            arg4->value = arg2;
            hit = 1;
            arg4->fraction = 0.0f;
        }
    }
    return hit;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_80054B3C.s")
#endif
typedef struct HitResolveMass {
    u8 pad0[4];
    f32 mass;
} HitResolveMass;

typedef struct HitResolveVehicle {
    s8 playerIndex;
    u8 pad1[3];
    f32 rotatedZ;
    f32 rotatedX;
    u8 padC[0x88];
    AnimVec3f velocity;
    u8 padA0[0x18];
    void *soundHandle;
    u8 padBC[0x18];
    void *collisionData;
    u8 padD8[0x18];
    s16 rotationY;
    u8 padF2[0xC];
    s16 rotationX;
    u8 pad100[0x6A];
    s16 collisionMode;
    u8 pad16C[0x3C];
    u16 flags;
    u8 pad1AA[0x209];
    u8 collisionTimer;
    u8 pad3B4[2];
    s16 collisionCountA;
    s16 collisionCountB;
} HitResolveVehicle;

extern f32 D_800841F0;
extern u32 func_80001620(s32 soundId);
extern void func_8000309C(void *handle, u8 volume);

typedef struct HitResolveRotation {
    s16 x;
    s16 y;
    s16 z;
} HitResolveRotation;

extern void mathOneFloatYPR(HitResolveRotation *rotation, AnimVec3f *vector);

/*
 * Bare-pragma reconstruction from Mickey's collision response assembly.
 * The public JFG hit.c family supplies role context only; Mickey fixes every
 * field offset, call identity and arithmetic association below.
 *
 * Plateau: 431 of 445 words, 420 differing from +0x38, frame 0xB8 -- the
 * target's. The frame came from carrier count, not from a spill: every
 * declared f32 in this TU reserves a home whether or not it is
 * register-coloured, so the six scalars whose live ranges end before the
 * response tail carry the tail's own values instead of being declared twice.
 * What remains is a real 14-word code deficit, not an allocation difference;
 * audit the impulse and effect-position groups against the target before any
 * further allocator reading.
 */
#ifdef NON_MATCHING
void func_80055104(HitCopyState *first, HitCopyState *second, f32 scale) {
    HitCopySource *firstSource;
    HitCopySource *secondSource;
    HitResolveVehicle *firstVehicle;
    HitResolveVehicle *secondVehicle;
    HitResolveMass *mass;
    void *firstCollision;
    void *secondCollision;
    HitResolveRotation rotation;
    AnimVec3f rotated;
    AnimVec3f direction;
    AnimVec3f effectPosition;
    f32 firstMass;
    f32 secondMass;
    f32 distance;
    f32 relativeVelocity;
    f32 impulse;
    f32 firstScale;
    f32 secondScale;

    firstVehicle = (HitResolveVehicle *) first->target;
    firstSource = first->source;
    mass = (HitResolveMass *) TrapDanglingJump(firstVehicle);
    firstMass = mass->mass;
    secondSource = second->source;
    secondVehicle = (HitResolveVehicle *) second->target;
    mass = (HitResolveMass *) TrapDanglingJump(secondVehicle);
    secondMass = mass->mass;
    direction.x = secondSource->current.x - firstSource->current.x;
    direction.y = secondSource->current.y - firstSource->current.y;
    direction.z = secondSource->current.z - firstSource->current.z;
    distance = sqrtf((direction.x * direction.x) +
                     (direction.y * direction.y) +
                     (direction.z * direction.z));
    direction.x /= distance;
    direction.y /= distance;
    direction.z /= distance;
    relativeVelocity =
        ((firstVehicle->velocity.x - secondVehicle->velocity.x) * direction.x) +
        ((firstVehicle->velocity.y - secondVehicle->velocity.y) * direction.y) +
        ((firstVehicle->velocity.z - secondVehicle->velocity.z) * direction.z);
    impulse = (D_800841F0 * relativeVelocity) /
              ((1.0f / firstMass) + (1.0f / secondMass));
    firstScale = impulse / firstMass;
    firstVehicle->velocity.x += firstScale * direction.x;
    firstVehicle->velocity.y += firstScale * direction.y;
    firstVehicle->velocity.z += firstScale * direction.z;
    rotation.x = -(firstVehicle->rotationY + firstVehicle->rotationX);
    rotation.y = -*(s16 *) ((u8 *) first + 2);
    rotation.z = -*(s16 *) ((u8 *) first + 4);
    rotated = firstVehicle->velocity;
    mathOneFloatYPR(&rotation, &rotated);
    firstVehicle->rotatedZ = rotated.z;
    firstVehicle->rotatedX = rotated.x;
    secondScale = impulse / secondMass;
    secondVehicle->velocity.x -= secondScale * direction.x;
    secondVehicle->velocity.y -= secondScale * direction.y;
    secondVehicle->velocity.z -= secondScale * direction.z;
    rotation.x = -(secondVehicle->rotationY + secondVehicle->rotationX);
    rotation.y = -*(s16 *) ((u8 *) second + 2);
    rotation.z = -*(s16 *) ((u8 *) second + 4);
    rotated = secondVehicle->velocity;
    mathOneFloatYPR(&rotation, &rotated);
    secondVehicle->rotatedZ = rotated.z;
    secondVehicle->rotatedX = rotated.x;
    secondScale = first->position.y - firstSource->previous.y;
    firstScale = first->position.x - firstSource->previous.x;
    impulse = first->position.z - firstSource->previous.z;
    firstSource->previous.x =
        (firstVehicle->velocity.x * scale) + firstSource->current.x;
    firstSource->previous.y =
        (firstVehicle->velocity.y * scale) + firstSource->current.y;
    firstSource->previous.z =
        (firstVehicle->velocity.z * scale) + firstSource->current.z;
    first->position.x = firstSource->previous.x + firstScale;
    first->position.y = firstSource->previous.y + secondScale;
    first->position.z = firstSource->previous.z + impulse;
    secondScale = second->position.y - secondSource->previous.y;
    firstScale = second->position.x - secondSource->previous.x;
    impulse = second->position.z - secondSource->previous.z;
    secondSource->previous.x =
        (secondVehicle->velocity.x * scale) + secondSource->current.x;
    secondSource->previous.y =
        (secondVehicle->velocity.y * scale) + secondSource->current.y;
    secondSource->previous.z =
        (secondVehicle->velocity.z * scale) + secondSource->current.z;
    second->position.x = secondSource->previous.x + firstScale;
    second->position.y = secondSource->previous.y + secondScale;
    second->position.z = secondSource->previous.z + impulse;

    firstCollision = (void *) TrapDanglingJump(firstVehicle->collisionData);
    secondCollision = (void *) TrapDanglingJump(secondVehicle->collisionData);
    if (((firstVehicle->collisionMode != 0) ||
         ((firstCollision == NULL) && (secondCollision != NULL))) &&
        (TrapDanglingJump(second, secondVehicle) != 0)) {
        firstVehicle->collisionCountA++;
        secondVehicle->collisionCountB++;
        if (*func_80028F54() == 5) {
            TrapDanglingJump(second);
        }
        TrapDanglingJump(first, second);
    }
    if (((secondVehicle->collisionMode != 0) ||
         ((secondCollision == NULL) && (firstCollision != NULL))) &&
        (TrapDanglingJump(first, firstVehicle) != 0)) {
        firstVehicle->collisionCountB++;
        secondVehicle->collisionCountA++;
        if (*func_80028F54() == 5) {
            TrapDanglingJump(first);
        }
        TrapDanglingJump(second, first);
    }

    firstVehicle->collisionTimer = 0x64;
    secondVehicle->collisionTimer = 0x64;
    if (relativeVelocity > 4.0f) {
        distance = distance * 0.5f;
        effectPosition.x =
            (direction.x * distance) + firstSource->current.x;
        effectPosition.y =
            (direction.y * distance) + firstSource->current.y;
        effectPosition.z =
            (direction.z * distance) + firstSource->current.z;
        firstMass = (f32) func_80001620(7);
        secondMass = (relativeVelocity / 20.0f) * firstMass;
        if (firstMass < secondMass) {
            secondMass = firstMass;
        }
        if (firstVehicle->soundHandle != NULL) {
            func_800031E8(firstVehicle->soundHandle);
        }
        func_80002FE0(7, effectPosition.x, effectPosition.y,
                      effectPosition.z, 4,
                      &firstVehicle->soundHandle);
        func_8000309C(firstVehicle->soundHandle, (u8) secondMass);
        if (!(firstVehicle->flags & 1)) {
            rumbleStart(firstVehicle->playerIndex, 0x32, 0.4f);
        }
        if (!(secondVehicle->flags & 1)) {
            rumbleStart(secondVehicle->playerIndex, 0x32, 0.4f);
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_80055104.s")
#endif
/* Mickey-local collision response reconstructed from its resident ABI. */
void func_800557F8(HitCopyState *first, HitCopyState *second, f32 unused) {
    s32 priority;
    HitCollisionLink *secondTarget;
    HitCollisionVehicle *firstVehicle;
    HitCopySource *source;
    s32 soundHandle;
    s32 timer;

    secondTarget = (HitCollisionLink *) second->target;
    priority = 4;
    firstVehicle = (HitCollisionVehicle *) first->target;
    source = second->source;
    if ((firstVehicle->unk16A == 0) && (firstVehicle->unk168 == 0)) {
        TrapDanglingJump(first, firstVehicle);
        /* Inert aid preserving IDO's target v0 allocation; tracked in
         * docs/cleanup-queue.md. */
        if (1) {
        }
        timer = 0x258;
        firstVehicle->unk158 = -0x7FFD;
        firstVehicle->unk15A = timer;
        firstVehicle->unk15C = timer;
        firstVehicle->unk150 = 10.0f;
        TrapDanglingJump(secondTarget->state, first);
        /* Inert aid preserving IDO's target v0 allocation; tracked in
         * docs/cleanup-queue.md. */
        if (1) {
        }
        ((HitCollisionVehicle *) secondTarget->state->target)->unk3B6++;
        firstVehicle->unk3B8++;
        if (*func_80028F54() == 5) {
            TrapDanglingJump(first);
        }
        soundHandle = firstVehicle->soundHandle;
        firstVehicle->unk185 = 0;
        firstVehicle->unk188 = 0.0f;
        if (soundHandle != 0) {
            func_800031E8(soundHandle);
        }
        if (!(firstVehicle->flags1A8 & 1)) {
            rumbleStart(firstVehicle->playerIndex, 0x46, 0.75f);
        }
    } else {
        func_80002FE0(0x26E, source->current.x, source->current.y,
                      source->current.z, priority, NULL);
    }
    second->position.x = source->current.x;
    second->position.y = source->current.y;
    second->position.z = source->current.z;
    source->previous.x = source->current.x;
    source->previous.y = source->current.y;
    source->previous.z = source->current.z;
    TrapDanglingJump(second, 1);
}
/* Declaration order is load-bearing: uopt homes declared locals at descending
 * stack offsets in declaration order, and only firstVehicle (sp+0x48, the
 * call-crossing spill) and firstSource (sp+0x3C) surface in the object, so
 * firstVehicle must be the second declaration and firstSource the fifth
 * (uoptlist frame-ladder-guided; the volatile pad holds the 0x40 slot). */
void func_80055970(HitCopyState *first, HitCopyState *second, f32 unused) {
    HitCollisionNormalLink *secondTarget;
    HitCollisionVehicle *firstVehicle;
    HitCopySource *secondSource;
    volatile s32 stackPad;
    HitCopySource *firstSource;
    f32 deltaX;
    f32 deltaY;
    f32 deltaZ;
    f32 distance;

    secondTarget = (HitCollisionNormalLink *) second->target;
    secondSource = second->source;
    firstSource = first->source;
    firstVehicle = (HitCollisionVehicle *) first->target;
    if (TrapDanglingJump(first, firstVehicle) != 0) {
        TrapDanglingJump(secondTarget->state, first, firstVehicle);
        /* Inert allocation aid retained by exact C; tracked in
         * docs/cleanup-queue.md. */
        if (1) {
        }
        ((HitCollisionVehicle *) secondTarget->state->target)->unk3B6++;
        firstVehicle->unk3B8++;
        if (*func_80028F54() == 5) {
            TrapDanglingJump(first);
        }
    } else {
        func_80002FE0(0x26E, secondSource->current.x,
                      secondSource->current.y, secondSource->current.z,
                      4, NULL);
    }
    deltaX = second->position.x - secondSource->previous.x;
    deltaY = second->position.y - secondSource->previous.y;
    deltaZ = second->position.z - secondSource->previous.z;
    secondSource->previous.x = secondSource->current.x;
    secondSource->previous.y = secondSource->current.y;
    secondSource->previous.z = secondSource->current.z;
    second->position.x = secondSource->previous.x + deltaX;
    second->position.y = secondSource->previous.y + deltaY;
    second->position.z = secondSource->previous.z + deltaZ;
    deltaX = secondSource->current.x - firstSource->current.x;
    deltaY = secondSource->current.y - firstSource->current.y;
    deltaZ = secondSource->current.z - firstSource->current.z;
    distance = sqrtf((deltaX * deltaX) + (deltaY * deltaY) +
                     (deltaZ * deltaZ));

    secondTarget->unk14 = deltaX / distance;
    secondTarget->unk18 = deltaY / distance;
    secondTarget->unk1C = deltaZ / distance;
    TrapDanglingJump(second, 0xE);
}

/* The first collision callback returns no value. Its typed weak alias removes
 * the generic trap placeholder's phantom s32 return web; the build restores
 * the measured TrapDanglingJump relocation identity without changing bytes. */
#pragma weak hitCopyFirstTrap = TrapDanglingJump
extern void hitCopyFirstTrap(HitCopyState *state, HitCollisionVehicle *vehicle);
void func_80055B24(HitCopyState *first, HitCopyState *second, f32 unused) {
    HitCollisionNormalLink *secondTarget;
    HitCopySource *secondSource;
    HitCollisionVehicle *firstVehicle;
    HitCopySource *firstSource;
    s32 soundHandle;
    f32 deltaX;
    f32 deltaY;
    f32 deltaZ;
    f32 distance;

    secondTarget = (HitCollisionNormalLink *) second->target;
    secondSource = second->source;
    firstSource = first->source;
    firstVehicle = (HitCollisionVehicle *) first->target;
    if ((firstVehicle->unk16A == 0) && (firstVehicle->unk168 == 0)) {
        hitCopyFirstTrap(first, firstVehicle);
        {
            s32 timer;

            timer = 0x258;
            firstVehicle->unk158 = -0x7FFD;
            firstVehicle->unk15A = timer;
            firstVehicle->unk15C = timer;
        }
        firstVehicle->unk150 = 10.0f;
        TrapDanglingJump(secondTarget->state, first);
        /* Inert allocation aid retained by exact C; tracked in
         * docs/cleanup-queue.md. */
        if (1) {
        }
        ((HitCollisionVehicle *) secondTarget->state->target)->unk3B6++;
        firstVehicle->unk3B8++;
        if (*func_80028F54() == 5) {
            TrapDanglingJump(first);
        }
        soundHandle = firstVehicle->soundHandle;
        firstVehicle->unk185 = 0;
        firstVehicle->unk188 = 0.0f;
        if (soundHandle != 0) {
            func_800031E8(soundHandle);
        }
        if (!(firstVehicle->flags1A8 & 1)) {
            rumbleStart(firstVehicle->playerIndex, 0x46, 0.75f);
        }
    } else {
        func_80002FE0(0x26E, secondSource->current.x,
                      secondSource->current.y, secondSource->current.z,
                      4, NULL);
    }
    second->position.x = secondSource->current.x;
    second->position.y = secondSource->current.y;
    second->position.z = secondSource->current.z;
    secondSource->previous.x = secondSource->current.x;
    secondSource->previous.y = secondSource->current.y;
    secondSource->previous.z = secondSource->current.z;

    deltaX = secondSource->current.x - firstSource->current.x;
    deltaY = secondSource->current.y - firstSource->current.y;
    deltaZ = secondSource->current.z - firstSource->current.z;
    distance = sqrtf((deltaX * deltaX) + (deltaY * deltaY) +
                     (deltaZ * deltaZ));

    secondTarget->unk1C = deltaX / distance;
    secondTarget->unk20 = deltaY / distance;
    secondTarget->unk24 = deltaZ / distance;
    TrapDanglingJump(second, 6);
}
void func_80055D08(HitCopyState *first, HitCopyState *second, f32 unused) {
    HitCopySource *firstSource;
    HitCopySource *secondSource;
    HitCopyTarget *target;
    f32 deltaX;
    f32 deltaY;
    f32 deltaZ;
    f32 distance;

    firstSource = first->source;
    first->position.x = firstSource->current.x;
    first->position.y = firstSource->current.y;
    first->position.z = firstSource->current.z;
    firstSource->previous.x = firstSource->current.x;
    firstSource->previous.y = firstSource->current.y;
    firstSource->previous.z = firstSource->current.z;

    secondSource = second->source;
    deltaX = second->position.x - secondSource->previous.x;
    deltaY = second->position.y - secondSource->previous.y;
    deltaZ = second->position.z - secondSource->previous.z;
    secondSource->previous.x = secondSource->current.x;
    secondSource->previous.y = secondSource->current.y;
    secondSource->previous.z = secondSource->current.z;
    second->position.x = secondSource->previous.x + deltaX;
    second->position.y = secondSource->previous.y + deltaY;
    second->position.z = secondSource->previous.z + deltaZ;

    deltaX = secondSource->current.x - firstSource->current.x;
    deltaY = secondSource->current.y - firstSource->current.y;
    deltaZ = secondSource->current.z - firstSource->current.z;
    distance = sqrtf((deltaX * deltaX) + (deltaY * deltaY) +
                     (deltaZ * deltaZ));

    target = second->target;
    target->unk14 = deltaX / distance;
    target->unk18 = deltaY / distance;
    target->unk1C = deltaZ / distance;
    TrapDanglingJump(first, 1, second);
    TrapDanglingJump(second, 0x12);
}
void func_80055E50(HitCopyState *first, HitCopyState *second, f32 unused) {
    HitCopySource *firstSource;
    HitCopySource *secondSource;
    HitCopyTarget *target;
    f32 deltaX;
    f32 deltaY;
    f32 deltaZ;
    f32 distance;

    firstSource = first->source;
    first->position.x = firstSource->current.x;
    first->position.y = firstSource->current.y;
    first->position.z = firstSource->current.z;
    firstSource->previous.x = firstSource->current.x;
    firstSource->previous.y = firstSource->current.y;
    firstSource->previous.z = firstSource->current.z;

    secondSource = second->source;
    second->position.x = secondSource->current.x;
    second->position.y = secondSource->current.y;
    second->position.z = secondSource->current.z;
    secondSource->previous.x = secondSource->current.x;
    secondSource->previous.y = secondSource->current.y;
    secondSource->previous.z = secondSource->current.z;

    deltaX = secondSource->current.x - firstSource->current.x;
    deltaY = secondSource->current.y - firstSource->current.y;
    deltaZ = secondSource->current.z - firstSource->current.z;
    distance = sqrtf((deltaX * deltaX) + (deltaY * deltaY) +
                     (deltaZ * deltaZ));

    target = second->target;
    target->unk1C = deltaX / distance;
    target->unk20 = deltaY / distance;
    target->unk24 = deltaZ / distance;
    TrapDanglingJump(first, 1, second);
    TrapDanglingJump(second, 0xA);
}

/* IDO's local allocation follows declaration order here; retain secondZ before
 * secondY so the volatile value keeps its exact stack home. */
void func_80055F64(HitCopyState *first, HitCopyState *second, f32 unused) {
    HitCopySource *firstSource;
    HitCopySource *secondSource;
    HitCopyTarget *secondTarget;
    HitCopyTarget *firstTarget;
    f32 deltaX;
    f32 deltaY;
    f32 deltaZ;
    f32 distance;
    f32 firstX;
    f32 secondX;
    volatile f32 secondZ;
    f32 secondY;

    firstSource = first->source;
    firstX = firstSource->current.x;
    deltaX = first->position.x - firstSource->previous.x;
    deltaY = first->position.y - firstSource->previous.y;
    deltaZ = first->position.z - firstSource->previous.z;
    firstSource->previous.x = firstX;
    firstSource->previous.y = firstSource->current.y;
    firstSource->previous.z = firstSource->current.z;
    first->position.x = firstX + deltaX;
    first->position.y = firstSource->previous.y + deltaY;
    first->position.z = firstSource->previous.z + deltaZ;

    secondSource = second->source;
    second->position.x = secondSource->current.x;
    second->position.y = secondSource->current.y;
    second->position.z = secondSource->current.z;
    secondX = *(volatile f32 *)&secondSource->current.x;
    secondY = *(volatile f32 *)&secondSource->current.y;
    secondSource->previous.x = secondX;
    secondSource->previous.y = secondY;
    secondZ = *(volatile f32 *)&secondSource->current.z;
    secondSource->previous.z = secondZ;

    deltaX = secondX - firstSource->current.x;
    deltaY = secondY - firstSource->current.y;
    deltaZ = secondZ - firstSource->current.z;
    distance = sqrtf((deltaX * deltaX) + (deltaY * deltaY) +
                     (deltaZ * deltaZ));

    secondTarget = second->target;
    secondTarget->unk1C = deltaX / distance;
    secondTarget->unk20 = deltaY / distance;
    secondTarget->unk24 = deltaZ / distance;
    firstTarget = first->target;
    firstTarget->unk14 = -secondTarget->unk1C;
    firstTarget->unk18 = -secondTarget->unk20;
    firstTarget->unk1C = -secondTarget->unk24;
    TrapDanglingJump(first, 0x12, second);
    TrapDanglingJump(second, 6);
}
void func_800560D0(HitCopyState *first, HitCopyState *second, f32 unused) {
    HitCopyTarget *firstTarget;
    HitCopyTarget *secondTarget;
    HitCopySource *firstSource;
    HitCopySource *secondSource;
    f32 deltaX;
    f32 deltaY;
    f32 deltaZ;
    f32 distance;

    firstSource = first->source;
    deltaX = first->position.x - firstSource->previous.x;
    deltaY = first->position.y - firstSource->previous.y;
    deltaZ = first->position.z - firstSource->previous.z;
    firstTarget = first->target;
    secondTarget = second->target;
    secondSource = second->source;
    firstSource->previous.x = firstSource->current.x;
    firstSource->previous.y = firstSource->current.y;
    firstSource->previous.z = firstSource->current.z;
    first->position.x = firstSource->previous.x + deltaX;
    first->position.y = firstSource->previous.y + deltaY;
    first->position.z = firstSource->previous.z + deltaZ;

    deltaX = second->position.x - secondSource->previous.x;
    deltaY = second->position.y - secondSource->previous.y;
    deltaZ = second->position.z - secondSource->previous.z;
    secondSource->previous.x = secondSource->current.x;
    secondSource->previous.y = secondSource->current.y;
    secondSource->previous.z = secondSource->current.z;
    second->position.x = secondSource->previous.x + deltaX;
    second->position.y = secondSource->previous.y + deltaY;
    second->position.z = secondSource->previous.z + deltaZ;

    deltaX = secondSource->current.x - firstSource->current.x;
    deltaY = secondSource->current.y - firstSource->current.y;
    deltaZ = secondSource->current.z - firstSource->current.z;
    distance = sqrtf((deltaX * deltaX) + (deltaY * deltaY) +
                     (deltaZ * deltaZ));

    firstTarget->unk14 = deltaX / distance;
    firstTarget->unk18 = deltaY / distance;
    firstTarget->unk1C = deltaZ / distance;
    secondTarget->unk14 = -firstTarget->unk14;
    secondTarget->unk18 = -firstTarget->unk18;
    secondTarget->unk1C = -firstTarget->unk1C;
    TrapDanglingJump(first, 6);
    TrapDanglingJump(second, 0x12);
}
void func_80056274(HitCopyState *first, HitCopyState *second, f32 unused) {
    HitCopyTarget *firstTarget;
    HitCopyTarget *secondTarget;
    HitCopySource *firstSource;
    HitCopySource *secondSource;
    f32 deltaX;
    f32 deltaY;
    f32 deltaZ;
    f32 distance;

    firstSource = first->source;
    secondTarget = second->target;
    secondSource = second->source;
    firstTarget = first->target;
    first->position.x = firstSource->current.x;
    first->position.y = firstSource->current.y;
    first->position.z = firstSource->current.z;
    firstSource->previous.x = firstSource->current.x;
    firstSource->previous.y = firstSource->current.y;
    firstSource->previous.z = firstSource->current.z;

    second->position.x = secondSource->current.x;
    second->position.y = secondSource->current.y;
    second->position.z = secondSource->current.z;
    secondSource->previous.x = secondSource->current.x;
    secondSource->previous.y = secondSource->current.y;
    secondSource->previous.z = secondSource->current.z;

    deltaX = secondSource->current.x - firstSource->current.x;
    deltaY = secondSource->current.y - firstSource->current.y;
    deltaZ = secondSource->current.z - firstSource->current.z;
    distance = sqrtf((deltaX * deltaX) + (deltaY * deltaY) +
                     (deltaZ * deltaZ));

    firstTarget->unk1C = deltaX / distance;
    firstTarget->unk20 = deltaY / distance;
    firstTarget->unk24 = deltaZ / distance;
    secondTarget->unk1C = -firstTarget->unk1C;
    secondTarget->unk20 = -firstTarget->unk20;
    secondTarget->unk24 = -firstTarget->unk24;
    TrapDanglingJump(first, 6, firstTarget);
    TrapDanglingJump(second, 0xA);
}
#ifdef NON_MATCHING
/*
 * PROVENANCE: JFG's public assembly-only hitVectorCheck establishes the
 * collision role and broad case ordering. This typed body is reconstructed
 * from Mickey's target, its m2c dataflow, and Mickey's neighboring collision
 * helpers; Mickey's bytes remain authoritative.
 */
s32 func_800563B4(s32 object, AnimCollisionShape *first, s32 value,
                  AnimCollisionShape *second, AnimCollisionResult *result) {
    AnimVec3f axis;
    AnimVec3f direction;
    AnimVec3f point;
    AnimVec3f endpoint;
    AnimVec3f minimum;
    AnimVec3f maximum;
    f32 length;
    f32 radius;
    f32 near;
    f32 far;
    f32 normalX;
    f32 normalY;
    f32 normalZ;
    f32 projection;
    s32 face;
    s32 status;

    direction.x = first->vector.x;
    direction.y = first->vector.y;
    radius = first->radius + second->radius;
    direction.z = first->vector.z;
    status = 0;
    length = (direction.z * direction.z) +
             ((direction.x * direction.x) +
              (direction.y * direction.y));

    if (second->shape == 0) {
        if (length > 0.0f) {
            length = sqrtf(length);
            direction.x /= length;
            direction.y /= length;
            direction.z /= length;
        }
        if (func_80012574(&first->position, &direction, &second->edge,
                          radius, &near, &far) != 0) {
            if ((near >= 0.0f) && (near <= length)) {
                status = 1;
                point.x = (direction.x * near) + first->position.x;
                point.y = (direction.y * near) + first->position.y;
                point.z = (direction.z * near) + first->position.z;
                normalX = (point.x - second->edge.x) / radius;
                normalY = (point.y - second->edge.y) / radius;
                near /= length;
                normalZ = (point.z - second->edge.z) / radius;
            } else if ((first->flags & 2) && (near < 0.0f) && (far > 0.0f)) {
                status = 2;
            }
        }
    } else if (second->shape == 1) {
        if (length > 0.0f) {
            length = sqrtf(length);
            direction.x /= length;
            direction.y /= length;
            direction.z /= length;
        }
        axis.x = 0.0f;
        axis.z = 0.0f;
        axis.y = 1.0f;
        if (func_80012234(&first->position, &direction, &second->position,
                          &axis, radius, &near, &far) != 0) {
            if ((near >= 0.0f) && (near <= length)) {
                point.x = (direction.x * near) + first->position.x;
                point.y = (direction.y * near) + first->position.y;
                point.z = (direction.z * near) + first->position.z;
                projection =
                    (((point.x - second->position.x) * axis.x) +
                     ((point.y - second->position.y) * axis.y) +
                     ((point.z - second->position.z) * axis.z)) /
                    ((axis.z * axis.z) +
                     ((axis.x * axis.x) + (axis.y * axis.y)));
                if ((-second->height <= projection) &&
                    (projection <= second->height)) {
                    status = 1;
                    normalX =
                        (point.x - ((axis.x * projection) +
                                    second->position.x)) / radius;
                    normalY =
                        (point.y - ((axis.y * projection) +
                                    second->position.y)) / radius;
                    normalZ =
                        (point.z - ((axis.z * projection) +
                                    second->position.z)) / radius;
                    near /= length;
                }
            } else if ((first->flags & 2) && (near < 0.0f) && (far > 0.0f)) {
                status = 2;
            }
        }
        if (status == 0) {
            endpoint.x = second->position.x - (axis.x * second->height);
            endpoint.y = second->position.y - (axis.y * second->height);
            endpoint.z = second->position.z - (axis.z * second->height);
            if (func_80012574(&first->position, &direction, &endpoint,
                              radius, &near, &far) != 0) {
                if ((near >= 0.0f) && (near <= length)) {
                    status = 1;
                    point.x = (direction.x * near) + first->position.x;
                    point.y = (direction.y * near) + first->position.y;
                    point.z = (direction.z * near) + first->position.z;
                    normalX = (point.x - endpoint.x) / radius;
                    normalY = (point.y - endpoint.y) / radius;
                    normalZ = (point.z - endpoint.z) / radius;
                    near /= length;
                } else if ((first->flags & 2) && (near < 0.0f) &&
                           (far > 0.0f)) {
                    status = 2;
                }
            }
        }
        if (status == 0) {
            endpoint.x = (axis.x * second->height) + second->position.x;
            endpoint.y = (axis.y * second->height) + second->position.y;
            endpoint.z = (axis.z * second->height) + second->position.z;
            if (func_80012574(&first->position, &direction, &endpoint,
                              radius, &near, &far) != 0) {
                if ((near >= 0.0f) && (near <= length)) {
                    status = 1;
                    point.x = (direction.x * near) + first->position.x;
                    point.y = (direction.y * near) + first->position.y;
                    point.z = (direction.z * near) + first->position.z;
                    normalX = (point.x - endpoint.x) / radius;
                    normalY = (point.y - endpoint.y) / radius;
                    normalZ = (point.z - endpoint.z) / radius;
                    near /= length;
                } else if ((first->flags & 2) && (near < 0.0f) &&
                           (far > 0.0f)) {
                    status = 2;
                }
            }
        }
    } else if (second->shape == 2) {
        minimum.x = (second->edge.x - second->radius) - first->radius;
        minimum.y = (second->edge.y - second->height) - first->height;
        minimum.z = (second->edge.z - second->radius) - first->radius;
        maximum.x = second->edge.x + second->radius + first->radius;
        maximum.y = second->edge.y + second->height + first->height;
        maximum.z = second->edge.z + second->radius + first->radius;
        face = func_800131AC(&first->position, &direction, &minimum, &maximum,
                             &near, &far);
        if ((face != 0) && (near >= 0.0f) && (near <= 1.0f)) {
            switch (face) {
                case 1:
                    normalY = 0.0f;
                    normalZ = 0.0f;
                    status = 1;
                    normalX = -1.0f;
                    break;
                case 2:
                    normalX = 1.0f;
                    normalY = 0.0f;
                    normalZ = 0.0f;
                    status = 1;
                    break;
                case 3:
                    normalX = 0.0f;
                    normalZ = 0.0f;
                    status = 1;
                    normalY = -1.0f;
                    break;
                case 4:
                    normalX = 0.0f;
                    normalY = 1.0f;
                    normalZ = 0.0f;
                    status = 1;
                    break;
                case 5:
                    normalX = 0.0f;
                    normalY = 0.0f;
                    status = 1;
                    normalZ = -1.0f;
                    break;
                case 6:
                    normalX = 0.0f;
                    normalY = 0.0f;
                    normalZ = 1.0f;
                default:
                    status = 1;
                    break;
            }
        } else if (first->flags & 2) {
            if ((minimum.x <= first->position.x) &&
                (first->position.x <= maximum.x) &&
                (minimum.y <= first->position.y) &&
                (first->position.y <= maximum.y) &&
                (minimum.z <= first->position.z) &&
                (first->position.z <= maximum.z)) {
                status = 2;
            }
        }
    }

    if (status == 1) {
        result->object = object;
        result->value = value;
        near -= D_8008420C;
        if (near < 0.0f) {
            near = 0.0f;
        }
        result->fraction = near;
        result->normal.x = normalX;
        result->normal.y = normalY;
        result->normal.z = normalZ;
    }
    return status;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_800563B4.s")
#endif

f32 func_8002A8BC(s32 angle);
f32 func_8002A8C0(s32 angle);

/*
 * Plateau: 229/229 instructions, frame 0x70, 177 differing words from +0x24.
 * Computing the contact dots as their own statement, then
 * (unk6C + 1.0f) * dots / (1.0f / mass), closed the one-word size deficit:
 * 1.0f/mass and (unk6C + 1.0f) now sit side by side after the dots.
 *
 * Next lever: delay materializing 25.0f until after the three squared
 * velocity products. ugen currently emits that constant first among the
 * magnitude ops, which occupies a ring temp and rotates the FP ring for the
 * rest of the procedure. Mag-as-local, L97, L144 on the magnitude, L144 on
 * retained, an unassociated sum, and deleting the pre-branch velocity
 * carriers were all measured; copy-forwarding collapses the mag local back
 * into the compare.
 */
#ifdef NON_MATCHING
void func_80056DD8(HitCopyState *first, HitCopyState *second,
                   AnimVec3f *normal, f32 timeStep) {
    HitCopyTarget *target;
    HitCopySource *firstSource;
    HitCopySource *secondSource;
    /* vectorX/Y/Z and scalar are shared by the two mutually exclusive arms:
     * the velocity triple and its magnitude above, the previous-position
     * triple and the plane dot product below. The target's 0x70 frame homes
     * exactly this many f32 locals. */
    f32 vectorX;
    f32 vectorY;
    f32 vectorZ;
    f32 scalar;
    f32 impulse;
    f32 correction;
    f32 cosine;
    f32 sine;
    f32 offsetX;
    f32 offsetY;
    f32 offsetZ;
    f32 displacement;
    volatile f32 retained;

    target = first->target;
    vectorX = target->velocity.x;
    vectorY = target->velocity.y;
    vectorZ = target->velocity.z;
    firstSource = first->source;
    secondSource = second->source;
    if (((vectorZ * vectorZ) +
         ((vectorX * vectorX) + (vectorY * vectorY))) > 25.0f) {
        f32 mass;

        mass = ((HitCopyTarget *) TrapDanglingJump(target))->unk4;
        vectorX = target->velocity.x;
        vectorY = target->velocity.y;
        vectorZ = target->velocity.z;
        /* Dots first so (unk6C + 1.0f) and 1.0f/mass sit side by side. */
        impulse = (normal->z * vectorZ) +
                  ((vectorX * normal->x) + (vectorY * normal->y));
        impulse = ((secondSource->unk6C + 1.0f) * impulse) / (1.0f / mass);
        correction = impulse / mass;
        retained = impulse;
        target->velocity.x = vectorX - (correction * normal->x);
        target->velocity.y = vectorY - (correction * normal->y);
        target->velocity.z = vectorZ - (correction * normal->z);
        scalar = sqrtf((target->velocity.z * target->velocity.z) +
                          ((target->velocity.x * target->velocity.x) +
                           (target->velocity.y * target->velocity.y)));
        target->magnitude80 = scalar;
        target->magnitude84 = scalar;
        target->direction.x = target->velocity.x / scalar;
        target->direction.y = target->velocity.y / scalar;
        target->direction.z = target->velocity.z / scalar;
        target->unk181 = 1;
        target->unk4 = 0.0f;
        target->unk8 = 0.0f;
        target->unk88 = D_80084210;
        firstSource->unk63 = 1;
        secondSource->unk63 = 1;
        secondSource->unk64 = scalar;
        cosine = -func_8002A8C0(*(s16 *) first);
        sine = -func_8002A8BC(*(s16 *) first);
        target->unk90 = (normal->z * cosine) - (normal->x * sine);
        target->unk8C = (normal->z * sine) + (cosine * normal->x);
        offsetY = first->position.y - firstSource->previous.y;
        offsetX = first->position.x - firstSource->previous.x;
        offsetZ = first->position.z - firstSource->previous.z;
        firstSource->previous.x =
            (target->velocity.x * timeStep) + firstSource->current.x;
        firstSource->previous.y =
            (target->velocity.y * timeStep) + firstSource->current.y;
        firstSource->previous.z =
            (target->velocity.z * timeStep) + firstSource->current.z;
        first->position.x = firstSource->previous.x + offsetX;
        first->position.y = firstSource->previous.y + offsetY;
        first->position.z = firstSource->previous.z + offsetZ;
    } else {
        scalar = (normal->z * firstSource->current.z) +
              ((firstSource->current.x * normal->x) +
               (firstSource->current.y * normal->y));
        retained = -scalar;
        vectorZ = firstSource->previous.z;
        vectorY = firstSource->previous.y;
        vectorX = firstSource->previous.x;
        displacement = D_80084214 -
                       (((normal->z * vectorZ) +
                         ((normal->x * vectorX) +
                          (normal->y * vectorY))) - scalar);
        offsetY = first->position.y - vectorY;
        offsetZ = first->position.z - vectorZ;
        offsetX = first->position.x - vectorX;
        firstSource->previous.x = vectorX + (displacement * normal->x);
        firstSource->previous.y = vectorY + (displacement * normal->y);
        firstSource->previous.z = vectorZ + (displacement * normal->z);
        first->position.x = firstSource->previous.x + offsetX;
        first->position.y = firstSource->previous.y + offsetY;
        first->position.z = firstSource->previous.z + offsetZ;
        firstSource->unk62 = 1;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_80056DD8.s")
#endif
/* PROVENANCE: JFG efd5abb's src/hit.c leaves hitGetInelasticVelocity as an
 * assembly fallback; its 0.0484 masked similarity supplies no donor C body.
 * Mickey's fields, behavior, and compiled bytes remain authoritative.
 *
 * Matched 2026-09-10. The last words closed on two levers, neither of them an
 * expression tree:
 *
 * - Frame cells. With N cells the frame is align8(4N) and the last cell homes
 *   at align8(4N) - 4N, so a home of 4 needs N odd. Seven declarations plus
 *   the compiler's own temporaries give N = 9; adding a declared carrier for
 *   the doubled value makes it ten and drops that home to 0, which is five
 *   words. Spelling the doubling as `-doubled * 2.0f` keeps seven
 *   declarations and uopt rewrites the multiply by two into the same sum,
 *   so one temporary serves both the negation and the doubling.
 * - Operand weight (workbench law L92). The x-axis product's two float
 *   operands land on the side their weight chooses, not the side they are
 *   written: `normalX * (...)` and `(...) * normalX` are byte-identical. An
 *   explicit `(f32)` cast on `normalX` changes that operand's weight and moves
 *   it left, which is field-guide lever 54 read on a float multiply rather
 *   than an address sum. The same cast on the right operand is inert, and so
 *   is a unary `+`.
 */
void func_8005716C(HitCopyState *state, void *unused, AnimVec3f *normal,
                   f32 timeStep) {
    HitCopyTarget *target;
    HitCopySource *source;
    f32 velocityX;
    f32 velocityY;
    f32 velocityZ;
    f32 doubled;
    f32 normalX;

    target = state->target;
    velocityX = state->velocity.x / target->unk4;
    velocityY = state->velocity.y / target->unk4;
    velocityZ = state->velocity.z / target->unk4;
    source = state->source;
    doubled = 0.0f;
    if (!target->unk0) {
        target->unk0 = 1;
    }
    target->unk4 *= D_80084218;

    doubled = (normalX = normal->x);
    doubled = (normal->z * velocityZ) +
              ((velocityX * doubled) + (velocityY * normal->y));
    state->velocity.x = (((f32) normalX * (-doubled * 2.0f)) + velocityX) * target->unk4;
    state->velocity.y = ((normal->y * (-doubled * 2.0f)) + velocityY) * target->unk4;
    state->velocity.z = ((normal->z * (-doubled * 2.0f)) + velocityZ) * target->unk4;

    state->position.x = source->current.x;
    state->position.y = source->current.y;
    state->position.z = source->current.z;
    source->previous.x = source->current.x + (state->velocity.x * timeStep);
    source->previous.y = source->current.y + (state->velocity.y * timeStep);
    source->previous.z = source->current.z + (state->velocity.z * timeStep);
}
void func_800572AC(HitCopyState *state, void *unused, AnimVec3f *position,
                   f32 unusedFloat) {
    f32 currentX;
    f32 deltaX;
    f32 deltaY;
    f32 deltaZ;
    HitCopySource *source;
    HitCopyTarget *target;

    source = state->source;
    currentX = source->current.x;
    target = state->target;
    deltaX = state->position.x - source->previous.x;
    deltaY = state->position.y - source->previous.y;
    deltaZ = state->position.z - source->previous.z;
    source->previous.x = currentX;
    source->previous.y = source->current.y;
    source->previous.z = source->current.z;
    state->position.x = currentX + deltaX;
    state->position.y = source->previous.y + deltaY;
    state->position.z = source->previous.z + deltaZ;
    target->unk14 = position->x;
    target->unk18 = position->y;
    target->unk1C = position->z;
    TrapDanglingJump(state, 0x16);
}
void func_80057350(HitCopyState *state, void *unused, AnimVec3f *position,
                   f32 unusedFloat) {
    HitCopySource *source;
    HitCopyTarget *target;

    source = state->source;
    target = state->target;
    state->position.x = source->current.x;
    state->position.y = source->current.y;
    state->position.z = source->current.z;
    source->previous.x = source->current.x;
    source->previous.y = source->current.y;
    source->previous.z = source->current.z;
    target->unk1C = position->x;
    target->unk20 = position->y;
    target->unk24 = position->z;
    TrapDanglingJump(state, 0xE);
}

/*
 * Mickey-led overlap response reconstruction; the nearest external skeleton
 * is only 0.085 similar and supplies no usable donor body.
 *
 * Three levers took this from a 203-word structure mismatch to exact:
 *   - the squared combined radius is its own statement, so IDO emits the
 *     multiply next to the sum and schedules it against the deltas;
 *   - the three-axis overlap test is a pre-tested `for`, not a `do`/`while`:
 *     the bottom-tested form lets uopt fold the zero subscript into the two
 *     strength-reduced base pointers and loses the two `addu` seeds;
 *   - the overlap denominator is carried in a named local, which is what puts
 *     it in a colour register rather than a block temp.
 * The TU's `-Wab,-r4300_mul` selection is what settles the last two words.
 */
void func_800573C8(HitOverlapState *state, HitOverlapVolume *other,
                   HitOverlapState *trigger, HitOverlapVolume *volume) {
    volatile f32 stackPad;
    f32 secondMin[3];
    f32 secondMax[3];
    f32 firstMin[3];
    f32 firstMax[3];
    f32 combinedRadius;
    f32 deltaX;
    f32 deltaY;
    f32 deltaZ;
    s32 index;
    s32 intersects;
    HitOverlapVehicle *vehicle;

    intersects = 0;
    if (volume->shape == 0) {
        combinedRadius = other->radius + volume->radius;
        combinedRadius = combinedRadius * combinedRadius;
        deltaX = volume->position.x - other->position.x;
        deltaY = volume->position.y - other->position.y;
        deltaZ = volume->position.z - other->position.z;
        if (((deltaX * deltaX) + (deltaY * deltaY) + (deltaZ * deltaZ)) <
            combinedRadius) {
            intersects = 1;
        }
    } else if (volume->shape == 1) {
        combinedRadius = other->radius + volume->radius;
        combinedRadius = combinedRadius * combinedRadius;
        deltaX = volume->position.x - other->position.x;
        deltaY = (volume->position.y - volume->height) - other->position.y;
        deltaZ = volume->position.z - other->position.z;
        if (((deltaX * deltaX) + (deltaY * deltaY) + (deltaZ * deltaZ)) <
            combinedRadius) {
            intersects = 1;
        }
    } else if (volume->shape == 2) {
        intersects = 1;
        firstMin[0] = other->position.x - other->radius;
        firstMin[1] = other->position.y - other->height;
        firstMin[2] = other->position.z - other->radius;
        firstMax[0] = other->position.x + other->radius;
        firstMax[1] = other->position.y + other->height;
        firstMax[2] = other->position.z + other->radius;
        secondMin[0] = volume->position.x - volume->radius;
        secondMin[1] = volume->position.y - volume->height;
        secondMin[2] = volume->position.z - volume->radius;
        secondMax[0] = volume->position.x + volume->radius;
        secondMax[1] = volume->position.y + volume->height;
        secondMax[2] = volume->position.z + volume->radius;
        for (index = 0; (index < 3) && (intersects != 0); index++) {
            if ((firstMin[index] < secondMin[index]) &&
                (firstMax[index] < secondMin[index])) {
                goto no_intersection;
            }
            if ((secondMax[index] < firstMin[index]) &&
                (secondMax[index] < firstMax[index])) {
no_intersection:
                intersects = 0;
            }
        }
    }
    if (intersects != 0) {
        if (state->kind44 == 1) {
            vehicle = state->vehicle;
            if (vehicle->unk16A != 0) {
                vehicle->unk16A = 0;
            }
            if (volume->position.y < volume->unk1C) {
                deltaX = (other->position.y - state->position.y) +
                         other->height;
                vehicle->overlap54 =
                    ((volume->position.y - volume->height) -
                     state->position.y) / deltaX;
                if (vehicle->overlap54 < 0.0f) {
                    vehicle->overlap54 = 0.0f;
                }
            }
            if (trigger->kind44 == 0x52) {
                if (vehicle->target338 == NULL) {
                    vehicle->target338 = trigger;
                    vehicle->timer18D = 0x1E;
                }
            } else {
                vehicle->timer18D = 0x1E;
            }
            volume->active = 1;
        } else if (state->kind44 == 0x40) {
            TrapDanglingJump(state, 1);
        } else if (state->kind44 == 0x39) {
            TrapDanglingJump(state, 5);
        } else if (state->kind44 == 0x3A) {
            TrapDanglingJump(state, 5);
        }
    }
}
/*
 * PROVENANCE: adapted from JFG's src/hit.c hitPlayer assembly. Mickey's ROM
 * establishes the entity cutoff, resident structures, and final code here.
 */
/*
 * Matched 2026-09-12 from 14 masked words by three edits, each measured:
 *   - the entity-kind byte is read through its own pointer, which gives the
 *     two range tests one coloured web instead of a ring temp and puts the
 *     whole integer ring back in phase (14 -> 6);
 *   - the sort is an index-based bubble sort, so strength reduction rebuilds
 *     the array cursor inside the outer loop instead of hoisting it and
 *     copying (the retained cursor form never restarted the pass, which is a
 *     different sort at the same instruction count);
 *   - `playerCount` sits one slot further down the declaration list, which is
 *     the whole stack-home ladder, and the inner `do` shares a physical line
 *     with the offset reset (L59).
 */
s32 func_8005776C(f32 x, f32 y, f32 z, f32 radius, s32 useXZ,
                  HitCopyState **nearby) {
    f32 distances[8];
    s8 *targetKind;
    HitCopyState **players;
    HitCopyState *player;
    HitCopyState **nearbyEntry;
    f32 deltaY;
    f32 distanceSquared;
    f32 currentDistance;
    s32 found;
    s32 playerCount;
    s32 remaining;
    s32 nearbyOffset;
    s32 index;

    found = 0;
    radius *= radius;
    players = func_80005750(&playerCount);
    if (playerCount > 0) {
        do {
            player = *players++;
            targetKind = (s8 *) player->target;
            if ((*targetKind >= 0) && (*targetKind < 6)) {
                f32 deltaX;
                f32 deltaZ;

                deltaX = player->position.x - x;
                deltaZ = player->position.z - z;
                distanceSquared = (deltaX * deltaX) + (deltaZ * deltaZ);
                if (useXZ == 0) {
                    deltaY = player->position.y - y;
                    distanceSquared += deltaY * deltaY;
                }
                if (distanceSquared < radius) {
                    distances[found] = sqrtf(distanceSquared);
                    nearby[found] = player;
                    found++;
                }
            }
            playerCount--;
        } while (playerCount > 0);

        remaining = found - 1;
        if (remaining > 0) {
            do {
                index = 0;
                /* L59: the inner `do` shares a physical line with the offset
                 * reset so as1's line tie-break emits the strength-reduced
                 * limit before the reset, as the ROM does. */
                nearbyOffset = 0; do {
                    nearbyEntry = (HitCopyState **)
                        ((u8 *) nearby + nearbyOffset);
                    if (distances[index + 1] < distances[index]) {
                        currentDistance = distances[index];
                        player = nearbyEntry[0];
                        distances[index] = distances[index + 1];
                        nearbyEntry[0] = nearbyEntry[1];
                        distances[index + 1] = currentDistance;
                        nearbyEntry[1] = player;
                    }
                    index++;
                    nearbyOffset += sizeof(*nearby);
                } while (index < remaining);
                remaining--;
            } while (remaining != 0);
        }
    }
    return found;
}
/*
 * PROVENANCE: adapted from JFG's src/fmvInit.c. Mickey's ROM establishes the
 * resource ID, globals, structure layout, and final compiler output here.
 */
void fmvInit(void) {
    FmvPlayer *player;
    s32 i;

    D_800D76D0[0] = piRomLoad(0x41);
    player = D_800D76D8;

    i = 2;
    while (i--) {
        player->unk0 = -1;
        player->unk14 = 0;
        player->unk18 = 0;
        player->unk1C = 0;
        player->unk20 = 0;
        player++;
    }
}

/* PLATEAU-HANDOFF:func_80051364:start
 * symbol: func_80051364
 * score: 94 differing words
 * frame: 0x48
 * relocations: 49
 * first-mismatch: +0x0
 * summary: hypothesis=an address keep that dies before the path loops; spellings=bare pointer inert at 102, store barrier scored 104, load-only barrier kept at 94; stall=frame still 0x48 and the command shift still follows the PAL math
 * PLATEAU-HANDOFF:func_80051364:end
 */


/* PLATEAU-HANDOFF:func_80054B3C:start
 * symbol: func_80054B3C
 * score: 374 differing words
 * frame: 0xD8
 * relocations: 3
 * first-mismatch: +0x8
 * summary: Rewritten as ordinary C; the frame now matches the target's 0xD8 and the candidate is 380 of 370 words, so the residual is ten words of surplus code plus register roles rather than allocation.
 * PLATEAU-HANDOFF:func_80054B3C:end
 */

/* PLATEAU-HANDOFF:func_80056DD8:start
 * symbol: func_80056DD8
 * score: 177 differing words
 * frame: 0x70
 * relocations: 8
 * first-mismatch: +0x24
 * summary: Size now exact at 229 words and frame 0x70; impulse dots-then-divide closed the missing word. Residual is the early 25.0f materialization rotating the FP ring from +0x24.
 * PLATEAU-HANDOFF:func_80056DD8:end
 */

/* PLATEAU-HANDOFF:func_80055104:start
 * symbol: func_80055104
 * score: 420 differing words
 * frame: 0xB8
 * relocations: 23
 * first-mismatch: +0x38
 * summary: Frame now matches at 0xB8 and the first fourteen words are exact; candidate is 431 of 445 words, so the deficit is real missing code rather than allocation.
 * PLATEAU-HANDOFF:func_80055104:end
 */

/* PLATEAU-HANDOFF:func_800563B4:start
 * symbol: func_800563B4
 * score: 637 differing words
 * frame: 0xD8
 * relocations: 11
 * first-mismatch: +0x1C
 * summary: Re-measured under the TU's -Wab,-r4300_mul selection and unchanged; candidate is 609 of 649 words with the exact frame, so the deficit is missing radius/vector work rather than allocation.
 * PLATEAU-HANDOFF:func_800563B4:end
 */

/* PLATEAU-HANDOFF:func_80053868:start
 * symbol: func_80053868
 * score: 1169 differing words
 * frame: 0xF8
 * relocations: 59
 * first-mismatch: +0xC
 * summary: 457 aligned exact words; exact frame; 60 words short. Callback constant force recovers 48 words; vector region remains allocation/peeling.
 * PLATEAU-HANDOFF:func_80053868:end
 */

/* PLATEAU-HANDOFF:func_800517E0:start
 * symbol: func_800517E0
 * score: 1782 differing words
 * frame: 0x160
 * relocations: 245
 * first-mismatch: +0x0
 * summary: 288 aligned exact words; 83 words short. Ordinary halfword carrier retained; addressed-home and divisor-force experiments stay diagnostic.
 * PLATEAU-HANDOFF:func_800517E0:end
 */
