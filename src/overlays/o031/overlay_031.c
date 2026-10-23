#include "overlays/overlay_031.h"

/* Retained diagnostic text from the module's initialized storage. */
const char gOverlay31ColourCycleWarning[] =
    "Particle warning: colour cycle table exceeds 255 entries\n";

/*
 * Overlay 31: one translation unit in ROM order (ADR 0006). The ORT exports
 * the particle-asset initializer (+0x2E8), the palette builder (+0x4F8) and
 * the buffer initializer (+0x6B0); the lookup-table builder, the config,
 * record and pool allocators are static, so the shipped calls to them are
 * JUMP records.
 */

static void func_overlay_031_F0000000_187F520(void);
static Overlay31Config *overlay31CreateConfig(s32 value0, void *source,
                                              s32 divisor, s32 multiplier,
                                              s32 count);
static Overlay31Record *overlay31CreateRecords(s32 count);
static Overlay31PoolRecord *overlay31CreatePool(s32 count);

/* Matched 2026-10-02 from an 86-word plateau. What it took: plain counted
 * loops with every vertex index written as an expression of the loop index
 * (uopt builds the level + i cursors itself); the triangle row bound held in
 * a local, which keeps the signed compare the target tests; the record
 * pointers read back from the row table rather than from the allocation's
 * return; the float table written through an f32 cursor with two
 * post-increments, whose extra references rank it above the loop index for
 * s1; and angle cleared before the row store. */
static void func_overlay_031_F0000000_187F520(void) {
    Overlay31IndexRecord *first;
    Overlay31IndexRecord *second;
    f32 *out;
    s32 level;
    s32 i;
    s32 limit;
    s16 angle;
    f32 s;
    f32 c;

    D_8007CA60[0][0] = mmAlloc(0x880, 0x8C);
    first = D_8007CA60[0][0];
    second = D_8007CA60[0][0] + 34;
    for (level = 2; level < 9; level++) {
        D_8007CA60[level - 2][0] = first;
        D_8007CA60[level - 2][1] = second;
        limit = level - 1;
        for (i = 0; i < limit; i++) {
            first->a0 = 0x40;
            first->a1 = i;
            first->a2 = i + 1;
            first->a3 = level + i + 1;
            first->b4 = 0;
            first->b6 = 0;
            first->b8 = 0x200;
            first->bA = 0;
            first->bC = 0x200;
            first->bE = 0x200;
            first->c10 = 0x40;
            first->c11 = i;
            first->c12 = level + i + 1;
            first->c13 = level + i;
            first->d14 = 0;
            first->d16 = 0;
            first->d18 = 0x200;
            first->d1A = 0x200;
            first->d1C = 0;
            first->d1E = 0x200;
            first++;
            second->a0 = 0x40;
            second->a1 = level + i;
            second->a2 = level + i + 1;
            second->a3 = i + 1;
            second->b4 = 0;
            second->b6 = 0;
            second->b8 = 0x200;
            second->bA = 0;
            second->bC = 0x200;
            second->bE = 0x200;
            second->c10 = 0x40;
            second->c11 = level + i;
            second->c12 = i + 1;
            second->c13 = i;
            second->d14 = 0;
            second->d16 = 0;
            second->d18 = 0x200;
            second->d1A = 0x200;
            second->d1C = 0;
            second->d1E = 0x200;
            second++;
        }
        if (level >= 3) {
            first->a0 = 0x40;
            first->a1 = i;
            first->a2 = 0;
            first->a3 = level;
            first->b4 = 0;
            first->b6 = 0;
            first->b8 = 0x200;
            first->bA = 0;
            first->bC = 0x200;
            first->bE = 0x200;
            first->c10 = 0x40;
            first->c11 = i;
            first->c12 = level;
            first->c13 = i + level;
            first->d14 = 0;
            first->d16 = 0;
            first->d18 = 0x200;
            first->d1A = 0x200;
            first->d1C = 0;
            first->d1E = 0x200;
            first++;
            second->a0 = 0x40;
            second->a1 = i + level;
            second->a2 = level;
            second->a3 = 0;
            second->b4 = 0;
            second->b6 = 0;
            second->b8 = 0x200;
            second->bA = 0;
            second->bC = 0x200;
            second->bE = 0x200;
            second->c10 = 0x40;
            second->c11 = i + level;
            second->c12 = 0;
            second->c13 = i;
            second->d14 = 0;
            second->d16 = 0;
            second->d18 = 0x200;
            second->d1A = 0x200;
            second->d1C = 0;
            second->d1E = 0x200;
            second++;
        }
    }

    out = mmAlloc(0x118, 0x8C);
    for (level = 2; level < 9; level++) {
        angle = 0;
        D_8007CA98[level - 2] = out;
        for (i = 0; i < level; i++) {
            s = func_8002A8BC(angle);
            c = func_8002A8C0(angle);
            *out++ = s * 2.0f + c * 2.0f;
            *out++ = s * 2.0f - c * 2.0f;
            angle += (s16)(0xFFFF / level);
        }
    }
}


/* PROVENANCE: Diddy Kong Racing, src/particles.c (init_particle_assets);
 * semantic source-shape analogue only. Mickey's ROM decides every detail. */
void func_overlay_031_F00002E8_187F808(void) {
    s32 i;

    for (i = 0; i < 5; i++) {
        D_800D4120[i] = 0;
    }

    D_8007C8AC = piRomLoad(0x33);
    while ((s32)D_8007C8AC[D_8007C8A4 + 1] != -1) {
        D_8007C8A4++;
    }

    D_8007C8A8 = piRomLoad(0x34);
    for (i = 0; i < D_8007C8A4; i++) {
        D_8007C8AC[i] =
            (Overlay31Item *)(D_8007C8A8 + (s32)D_8007C8AC[i]);
    }

    overlay31BuildPalettes();

    D_8007C8B8 = piRomLoad(0x35);
    while ((s32)D_8007C8B8[D_8007C8B0 + 1] != -1) {
        D_8007C8B0++;
    }

    D_8007C8B4 = piRomLoad(0x36);
    for (i = 0; i < D_8007C8B0; i++) {
        D_8007C8B8[i] =
            (Overlay31Behaviour *)(D_8007C8B4 +
                                   (s32)D_8007C8B8[i]);
        if ((u32)D_8007C8B8[i]->colourLoop != 0xFFFFFFFF) {
            D_8007C8B8[i]->colourLoop =
                func_800056A4(
                    (s32)D_8007C8B8[i]->colourLoop);
        } else {
            D_8007C8B8[i]->colourLoop = NULL;
        }
    }
}

void overlay31BuildPalettes(void) {
    Overlay31Item *item;
    s32 count;
    s32 i;
    s32 j;
    Overlay31PaletteCursor cursor;

    for (i = 0; i < D_8007C8A4; i++) {
        item = D_8007C8AC[i];
        if (item->flags & 0x400) {
            for (j = 0; j < i; j++) {
                if (item->paletteId == D_8007C8AC[j]->paletteId) {
                    item->palette = D_8007C8AC[j]->palette;
                    item->paletteCount = cursor.state->count;
                    if (item->paletteCount == 0) {
                        item->paletteCount = 1;
                    }
                    j = i;
                }
            }

            if (item->palette == 0) {
                u32 *palette;

                initColourCycle(&cursor, item->paletteId);
                count = cursor.state->count;
                if (count == 0) {
                    count = 1;
                }
                if (count >= 0x100) {
                    count = 0xFF;
                }

                item->palette = mmAlloc(count * 4, 0x8C);
                item->paletteCount = count;
                palette = item->palette;
                for (j = 0; j < count; j++) {
                    *palette++ = ((u32)cursor.red << 24) |
                                 ((u32)cursor.green << 16) |
                                 ((u32)cursor.blue << 8) | cursor.alpha;
                    updateColourCycle(&cursor, 1);
                }
            }
        }
    }
}

/* PROVENANCE: Diddy Kong Racing, src/particles.c
 * (init_particle_buffers); semantic source-shape analogue only. */
/* Exact 245-word match; declaration order preserves IDO's 0x48-byte frame. */
void overlay31InitializeBuffers(s32 maxTriangle, s32 maxRectangle,
                                s32 maxSprite, s32 maxLine, s32 maxPoint,
                                s32 maxExtra, s32 maxEffects) {
    s32 i;
    s32 vertexCount;
    s32 temp_var;
    s16 *assetBuffer;

    reset_particles();

    if (maxTriangle < 0) {
        maxTriangle = 0x10;
    }
    if (maxRectangle < 0) {
        maxRectangle = 0x10;
    }
    if (maxSprite < 0) {
        maxSprite = 0xD0;
    }
    if (maxExtra < 0) {
        maxExtra = 0xF;
    }
    if (maxLine < 0) {
        maxLine = 0xA0;
    }
    if (maxPoint < 0) {
        maxPoint = 0x40;
    }
    if (maxEffects < 0) {
        maxEffects = 0x20;
    }

    vertexCount = maxTriangle * 3 + maxRectangle * 4 + maxSprite +
                  maxLine * 0x12 + maxPoint * 0x78;
    D_8007C88C = maxLine;
    D_8007C890 = maxPoint;
    D_8007C89C[0] =
        mmAlloc(vertexCount * 2 * sizeof(Overlay31Vertex), 0x8C);
    D_8007C89C[1] =
        (u8 *)D_8007C89C[0] + vertexCount * 10;

    D_800D4120[0] = overlay31CreateConfig(
        (s32)D_8007CAB4, D_8007CAC0,
        3, 1, maxTriangle);
    D_800D4120[1] = overlay31CreateConfig(
        (s32)D_8007CC40, D_8007CC50,
        4, 2, maxRectangle);
    D_800D4120[2] = overlay31CreateConfig(0, 0, 0, 0, maxSprite);
    D_800D4120[3] = overlay31CreateConfig(0, 0, 0, 0, maxExtra);
    D_8007C898 = overlay31CreatePool(maxPoint);
    D_8007C894 = overlay31CreateRecords(D_8007C88C);
    func_overlay_031_F0000000_187F520();

    if (D_8007C884 == 0) {
        assetBuffer = piRomLoad(0x3A);
        D_8007C888 = 0;
        while (assetBuffer[D_8007C888] != -1) {
            D_8007C888++;
        }

        D_8007C884 =
            mmAlloc(D_8007C888 * 4, 0x8C);
        for (i = 0; i < D_8007C888; i++) {
            switch (assetBuffer[i] & 0xC000) {
                case 0x8000:
                    D_8007C884[i] =
                        texLoadSprite(assetBuffer[i] & 0x3FFF, 1);
                    break;
                case 0xC000:
                    D_8007C884[i] =
                        texLoadTexture(assetBuffer[i] & 0x3FFF);
                    break;
                case 0:
                    D_8007C884[i] =
                        modLoadModel(assetBuffer[i], 0);
                    break;
            }
        }
        mmFree(assetBuffer);
    }

    D_8007C8BC =
        mmAlloc(maxEffects * sizeof(Overlay31EffectRecord), 0x8C);
    for (i = 0; i < maxEffects; i++) {
        D_8007C8BC[i].active = 0;
    }
    D_8007C8C0 = maxEffects;
}

static Overlay31Config *overlay31CreateConfig(s32 value0, void *source, s32 divisor, s32 multiplier, s32 count)
{
  Overlay31Config *config;
  s32 maskCount;
  s32 recordBytes;
  s32 rawRecordBytes;
  s32 remainder;
  s32 i;
  s32 width;
  u32 bit;
  maskCount = count >> 5;
  remainder = count & 0x1F;
  if (remainder != 0)
  {
    maskCount++;
  }
  rawRecordBytes = (count * 0x78) + 0x38;
  recordBytes = (rawRecordBytes - (((u32) rawRecordBytes) % 4U)) + 4;
  rawRecordBytes = recordBytes;
  rawRecordBytes += maskCount * 4;
  config = mmAlloc(rawRecordBytes, 0x8C);
  config->count = count;
  config->masks = (u32 *) (((u8 *) config) + recordBytes);
  config->records = (Overlay31ConfigRecord *) (((u8 *) config) + 0x38);
  config->lastMask = maskCount - 1;
  config->reserved28 = 0;
  config->reserved1C = 0;
  config->arg3 = multiplier;
  config->arg2 = divisor;
  config->arg0 = value0;
  if (source != ((void *) 0))
  {
    width = (0x18 / divisor) * multiplier;
    if (!(width < 0x11))
    {
      width = 0x10;
    }
    config->source = source;
    config->source10 = (void *) ((width * 0x10) + ((s32) source));
    config->source20 = (void *) (((width << 2) << 3) + ((s32) source));
  }
  else
  {
    config->source = (void *) 0;
    config->source10 = (void *) 0;
    config->source20 = (void *) 0;
  }
  for (i = 0; i < count; i++)
  {
    config->records[i].state = 0;
    config->records[i].level = 0x80;
  }

  i = 0;
  if (config->lastMask >= 0)
  {
    do
    {
      config->masks[i] = ~0U;
      i++;
    }
    while (config->lastMask >= i);
  }
  bit = 1;
  if (remainder != 0)
  {
    config->masks[config->lastMask] = 0;
    for (i = remainder; i > 0; i--)
    {
      config->masks[config->lastMask] |= bit;
      bit <<= 1;
    }

  }
  return config;
}

/* DKR v77/v80 and JFG contain no exact donor for this record allocator. */
static Overlay31Record *overlay31CreateRecords(s32 count) {
    Overlay31Record *records;
    Overlay31Record *record;
    s32 i;
    s32 j;

    records = mmAlloc(count * sizeof(Overlay31Record), 0x8C);
    record = records;
    for (i = 0; i < count; i++) {
        record->owner = NULL;
        record->active = 0;
        for (j = 0; j < 9; j++) {
            record->blocks[j] = record->data[j];
        }
        record++;
    }
    return records;
}

/* DKR v77/v80 and JFG contain no exact donor for this pool allocator. */
/* Matched 2026-09-17 (lane w6-o031), 11 -> 0 masked words at delta 0,
 * frame 0x38, four relocations, unforced. An inner-loop OR-zero on a
 * last-declared s32 is an L109 probe: it creates a phantom a0 web
 * (save 100.5) so record is forbidden c3 and takes a1, and i takes a2.
 * Declaration order is load-bearing (`s32 i; s32 j;` first, `zero` last).
 * The inert OR-zero is tracked in docs/cleanup-queue.md. */
static Overlay31PoolRecord *overlay31CreatePool(s32 count) {
    s32 i;
    s32 j;
    Overlay31PoolRecord *records;
    Overlay31PoolRecord *record;
    s32 *state;
    s32 zero;

    records = (Overlay31PoolRecord *)mmAlloc(
        count * sizeof(Overlay31PoolRecord), 0x8C);
    record = records;
    zero = 0;

    i = 0;
    if (count > 0) {
        do {
            record->state[15] = 0;
            record->active = 0;
            record->state[0] = 0;
            record->state[1] = 0;
            record->state[2] = 0;

            j = 3;
            state = &record->state[3];
            do {
                zero |= 0;
                j += 4;
                state[1] = 0;
                state[2] = 0;
                state[3] = 0;
                state += 4;
                state[-4] = 0;
            } while (j != 15);
            i++;
            record++;
        } while (i != count);
    }

    D_800D4120[4] = overlay31CreateConfig(0, 0, 0, 0, count * 15);
    return records;
}
