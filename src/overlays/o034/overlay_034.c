#include "overlays/overlay_034.h"

Overlay34Record *gOverlay34Records = 0;
Overlay34Record **gOverlay34Pointers = 0;
s32 gOverlay34Count = 0;
s32 gOverlay34ActiveCount = 0;
f32 gOverlay34Value10 = 0.275f;

/*
 * Overlay 34: one translation unit in ROM order (ADR 0006). All eight
 * functions are ORT exports.
 */

/* The byte count is a call-crossing common subexpression rather than a named
 * local: that puts its caller-save home one word above a declared local's,
 * which is what fixes the four sp+0x1C sites. */
void overlay34InitStorage(s32 count) {
    s32 *word;
    s32 countdown;
    s32 remaining;

    gOverlay34Records = mmAlloc(count * (s32)sizeof(Overlay34Record), 0x87);
    word = (s32 *)gOverlay34Records;
    remaining = (count * (s32)sizeof(Overlay34Record)) >> 2;
    countdown = remaining - 1;
    if (remaining != 0) {
        do {
            *word++ = 0;
        } while (countdown--);
    }

    gOverlay34Pointers = mmAlloc(count * (s32)sizeof(*gOverlay34Pointers), 0x87);
    word = (s32 *)gOverlay34Pointers;
    remaining = (count * (s32)sizeof(*gOverlay34Pointers)) >> 2;
    countdown = remaining - 1;
    if (remaining != 0) {
        do {
            *word++ = 0;
        } while (countdown--);
    }
    gOverlay34Count = count;
}

void overlay34SetValue10(f32 value) {
    gOverlay34Value10 = value;
}

/* Matched 2026-09-17, lane w3-o034. 125/125 words, frame 0x30, 12 relocs.
 * uopt closes a straight-line block after twenty cfe Ulod of non-veqv isvars.
 * Stores through the declared `candidate` spent that budget at byte12, so
 * short16 opened the next block and the pair could not share the ROM order.
 * Rebinding `record = candidate` after the resource call gives the fill
 * pointer an assigned tree with no ILOD; forwarding then skips the base
 * Ulods, the whole fill including short16 and byte12 stays in one block,
 * and height (one block, save 4) outranks the resource copy (save 3) without
 * the self-defs that previously bought that ranking. */
Overlay34Record *overlay34CreateRecord(Overlay34Input *input) {
    Overlay34Record *record;
    Overlay34Record *current;
    s32 width;
    s32 height;
    Overlay34Record *candidate;
    s32 index;

    candidate = NULL;
    if (gOverlay34ActiveCount < gOverlay34Count) {
        if (gOverlay34Count > 0) {
            index = 0;
            do {
                record = &gOverlay34Records[index];
                current = record;
                if (current->active == 0) {
                    candidate = current;
                    break;
                }
                index++;
            } while (index < gOverlay34Count);
        }
        if (candidate != NULL) {
            candidate->resource = texLoadTexture(input->resourceId);
            if (candidate->resource != NULL) {
                record = candidate;
                width = (record->resource->width - 1) << 5;
                height = (record->resource->height - 1) << 5;
                record->byte00 = 0x40;
                record->byte01 = 0;
                record->short04 = width;
                record->short06 = 0;
                record->byte02 = 1;
                record->short08 = width;
                record->short0A = height;
                record->byte03 = 2;
                record->short0C = 0;
                record->short0E = 0;
                record->byte10 = 0x40;
                record->byte11 = 1;
                record->short14 = width;
                record->short16 = height;
                record->byte12 = 2;
                record->short18 = 0;
                record->short1A = 0;
                record->byte13 = 3;
                record->short1C = 0;
                record->short1E = height;
                record->word24 = input->word1C;
                record->word28 = input->word28;
                record->word2C = input->word20;
                record->word30 = input->word2C;
                record->word34 = input->word24;
                record->word38 = input->word30;
                record->byte3C = 0;
                record->byte3D = input->mode * 6;
                record->x1 = input->x;
                record->y1 = input->y;
                record->z1 = input->z;
                record->x2 = input->x;
                record->y2 = input->y;
                record->z2 = input->z;
                record->depth = -input->depth;
                mathOneFloatPY(input, record->direction);
                record->value = input->value;
                record->active = 1;
                gOverlay34Pointers[gOverlay34ActiveCount] = record;
                gOverlay34ActiveCount++;
            }
        }
    }
    return candidate;
}

/* Matched 2026-09-12, lane p9-tight. Twelve words to zero. The plateau's
 * `shadow` local was an artefact: the four `move $a1,$v1` the old handoff was
 * trying to buy are the post-decrement temp of `while (remaining--)`, emitted
 * once in each loop's guarded entry and once at each rotated loop top, and
 * they are one web. A source-level `shadow` cannot join that web because the
 * two ranges overlap -- `shadow` is live to the call while the temp is live to
 * the bottom test -- so every spelling of the copy gave the temp its own
 * register. Deleting `shadow` and writing both loops as `while (remaining--)`
 * puts the temp in $a1 and reproduces all four copies for free; the condition
 * must be bare, since `remaining-- != 0` materialises a boolean and emits
 * `sltu` where the target has `move`. texFreeTexture therefore takes one
 * argument: $a1 at the call is this dead temp, not a second parameter. */
void overlay34RemoveRecord(Overlay34Record *record) {
    Overlay34Record **slot;
    s32 remaining;

    slot = gOverlay34Pointers;
    remaining = gOverlay34ActiveCount;
    while (remaining--) {
        if (*slot == record) {
            while (remaining--) {
                *slot = slot[1];
                slot++;
            }
            if (record->resource != NULL) {
                texFreeTexture(record->resource);
            }
            record->active = 0;
            gOverlay34ActiveCount--;
            return;
        }
        slot++;
    }
}

void overlay34ResetStorage(void) {
    if (gOverlay34Pointers != NULL) {
        if (gOverlay34ActiveCount != 0) {
            do {
                overlay34RemoveRecord(*gOverlay34Pointers);
            } while (gOverlay34ActiveCount != 0);
        }
        mmFree(gOverlay34Records);
        mmFree(gOverlay34Pointers);
    }
    gOverlay34Count = 0;
    gOverlay34ActiveCount = 0;
    gOverlay34Records = NULL;
    gOverlay34Pointers = NULL;
}

/* PROVENANCE: JFG's public assembly-only
 * src/overlays/o10/overlay_10.c:sparkUpdate supplied structural context for
 * the shared update/remove scaffold. This body and its field layout were
 * reconstructed and matched against Mickey's own bytes. */
void overlay34UpdateRecords(s32 updateRate) {
    Overlay34MotionRecord *record;
    s32 index;
    s32 offset;
    u8 timer;
    f32 velocityY;

    if (updateRate != 0) {
        index = 0;
        offset = 0;
        if (gOverlay34ActiveCount > 0) {
            do {
                index++;
                record = *(Overlay34MotionRecord **)((u8 *)gOverlay34Pointers + offset);
                offset += 4;
                timer = record->timer + updateRate;
                record->timer = timer;
                if ((timer & 0xFF) < record->lifetime) {
                    s32 remaining = updateRate - 1;

                    record->previousX = record->x;
                    record->previousY = record->y;
                    record->previousZ = record->z;
                    if (updateRate != 0) {
                        do {
                            record->x += record->velocityX;
                            velocityY = record->velocityY;
                            record->y += record->velocityY;
                            record->z += record->velocityZ;
                            record->velocityY = velocityY - gOverlay34Value10;
                        } while (remaining--);
                    }
                } else {
                    overlay34RemoveRecord(record);
                    index--;
                    offset -= 4;
                }
            } while (index < gOverlay34ActiveCount);
        }
    }
}

/* Pinned DKR v77/v80 and JFG searches found no exact donor. */
void overlay34InterpolateColor(s32 position, s32 length, const u8 *start,
                               const u8 *end, u8 *output) {
    if (position > length) {
        position = length;
    }
    position = (position << 16) / length;
    output[0] = start[0] +
                ((s32)((end[0] - start[0]) * (u32)position) >> 16);
    output[1] = start[1] +
                ((s32)((end[1] - start[1]) * (u32)position) >> 16);
    output[2] = start[2] +
                ((s32)((end[2] - start[2]) * (u32)position) >> 16);
    output[3] = start[3] +
                ((s32)((end[3] - start[3]) * (u32)position) >> 16);
}

/* Fresh pinned DKR v77/v80 and JFG scans found no Overlay 34 donor. */

/* L160 indexed draw; split record->frame spellings; position |= 0 is L100
 * weight so position outranks length. Do-while fill closed two structural
 * first-loop words. Comma-assign of frame plus else position -= half puts
 * the copy in the branch delay without the pre-test v0/v1 swap. Five unused
 * pointers plus distances[59] (L99+L112, net-zero block) drop the colour
 * homes onto +0x184/+0x188. Scalars declared above the array pull its home
 * down 24 bytes into the spilltemps gap, onto +0x80. */
void overlay34SortAndDraw(void *commands, void *vertices) {
    void *pad0;
    void *pad1;
    void *pad2;
    void *pad3;
    void *pad4;
    u32 color1;
    u32 color2;
    f32 swapDistance;
    s32 i;
    s32 j;
    s32 half;
    s32 position;
    s32 length;
    f32 distances[59];
    Overlay34DrawRecord *record;

    j = 0;
    if (gOverlay34ActiveCount > 0) {
        do {
            record = (Overlay34DrawRecord *)gOverlay34Pointers[j];
            distances[j] = camGetProjZ(record->x1, record->y1, record->z1);
            j++;
        } while (j < gOverlay34ActiveCount);
    }

    i = gOverlay34ActiveCount;
    while (i--) {
        for (j = 0; j < i; j++) {
            if (distances[j + 1] < distances[j]) {
                swapDistance = distances[j];
                record = (Overlay34DrawRecord *)gOverlay34Pointers[j];
                distances[j] = distances[j + 1];
                gOverlay34Pointers[j] = gOverlay34Pointers[j + 1];
                distances[j + 1] = swapDistance;
                gOverlay34Pointers[j + 1] = (Overlay34Record *)record;
            }
        }
    }

    j = 0;
    if (gOverlay34ActiveCount > 0) {
        do {
            record = (Overlay34DrawRecord *)gOverlay34Pointers[j];
            j++;
            if (record != NULL) {
                length = record->frameCount;
                half = length >> 1;
                if (position = record->frame, record->frame < half) {
                    position |= 0;
                    overlay34InterpolateColor(position, half,
                                              (u8 *)&record->color0A,
                                              (u8 *)&record->color1A,
                                              (u8 *)&color1);
                    overlay34InterpolateColor(position, half,
                                              (u8 *)&record->color0B,
                                              (u8 *)&record->color1B,
                                              (u8 *)&color2);
                } else {
                    position -= half;
                    length -= half;
                    overlay34InterpolateColor(position, length,
                                              (u8 *)&record->color1A,
                                              (u8 *)&record->color2A,
                                              (u8 *)&color1);
                    overlay34InterpolateColor(position, length,
                                              (u8 *)&record->color1B,
                                              (u8 *)&record->color2B,
                                              (u8 *)&color2);
                }
                func_800084C4(commands, vertices, record->resource, record,
                              &record->x1, &record->x2, record->value,
                              color1, color2, 0x200);
            }
        } while (j < gOverlay34ActiveCount);
    }
}
