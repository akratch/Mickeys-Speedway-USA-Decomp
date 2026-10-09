#include "PR/ultratypes.h"

/*
 * PROVENANCE: Mickey-derived from this overlay's owned assembly and runtime
 * relocation tables; pinned DKR/JFG scans found no donor.
 *
 * Bounded reproof (2026-08-29): the policy-clean split-carrier C compiles to
 * the exact 0xF8/62-word extent with frame 0x28. It matches 52/62 raw words
 * and 53/62 relocation-normalized words, with first raw +0x10 and first
 * substantive +0x54. Ten raw sites remain; normalizing the table LO16 leaves
 * nine sites in one descriptor/call-argument carrier web. All 119 compiler
 * configurations were nonexact, with canonical -O2 -mips2 tied for best. One
 * instrumented UOPT trace found seven colored webs and an otherwise exact
 * temporary-register lane. The trace-supported natural form that coalesced
 * descriptor input with the acquisition result compiled byte-identically to
 * this split-carrier body, so there was no strict gain to retain.
 * The owned range is overlay
 * +0x70..+0x168 (ROM 0x18B87C0..0x18B88B8), with no target padding and the next
 * function beginning at +0x168; the separate +0xA1C..+0xA20 padding is not
 * owned. The LOCAL table pair resolves through
 * base +0xA20 plus +0x5A4 to module +0xFC4. The other four records are local
 * JUMPs to overlay59Release at +0x48/+0xD0 and resident ORT 135 calls to
 * texLoadTexture at +0x64/+0x94. The synthetic target
 * object collapses all four zero-field calls to one placeholder and cannot
 * prove their identities. No linked candidate-C proof survives.
 * The s32 three-argument ABI is called only from four sites in
 * overlay59Advance at module +0x418/+0x590/+0x614/+0x66C; it is not exported
 * and has no resident or cross-overlay inbound. The retained structural oracle
 * is negative (best similarity 0.106, with no credible donor). A 2026-09-04
 * pass merged value and handle in both directions, retaining either the u32 or
 * pointer carrier; both compile byte-identically and do not create the target
 * v0 load/call-delay web. The fallback needs a different natural partition.
 */

typedef struct Overlay59Descriptor {
    void *first;
    void *second;
    u8 pad08[8];
} Overlay59Descriptor;

typedef struct Overlay59Entry {
    u8 pad00[0x20];
    void **owner;
    u32 handles[8];
} Overlay59Entry;

typedef struct Overlay59DescriptorGroup {
    void ***descriptors;
    u32 unused;
} Overlay59DescriptorGroup;

extern Overlay59DescriptorGroup gOverlay59DescriptorTables[];
extern void overlay59PrepareReleaseReloc(Overlay59Entry *entry);
extern void *texLoadTexture(s32 textureId);

s32 overlay59PrepareEntry(Overlay59Entry *entry, s32 tableIndex, s32 itemIndex)
{
  void **descriptor;
  void *value;
  u32 handle;
  s32 count;
  s32 result;
  descriptor = ((Overlay59DescriptorGroup *) (((u8 *) gOverlay59DescriptorTables) + 0x5A4))[tableIndex].descriptors[itemIndex];
  result = 1;
  if (descriptor != entry->owner)
  {
    overlay59PrepareReleaseReloc(entry);
    entry->owner = descriptor;
    count = 0;
    value = *descriptor;
    if (value != 0)
    {
      do
      {
        handle = ((u32) value) & 0xFFFFFFFF;
        handle = (u32) texLoadTexture((s32) handle);
        if (handle == 0)
        {
          result = 0;
        }
        else
        {
          entry->handles[count] = handle;
          count++;
        }
        value = descriptor[1];
        if (value != 0)
        {
          handle = ((u32) value) & 0xFFFFFFFFu;
          handle = (u32) texLoadTexture((s32) handle);
          if (handle == 0)
          {
            result = 0;
          }
          else
          {
            entry->handles[count] = handle;
            count++;
          }
        }
        descriptor += 4;
        value = *descriptor;
      }
      while (value != 0);
    }
    if (result == 0)
    {
      overlay59PrepareReleaseReloc(entry);
    }
  }
  return result;
}

/* Prior binding reproof (2026-09-06): the resident acquisition callee consumes an
 * integer texture ID and returns a pointer handle. Explicit casts preserve
 * the existing N64 32-bit carriers. Raw and configured full-TU text remains
 * unchanged; all six relocation identities now agree with the ROM records.
 * The configured resident alias stays distinct from the local release call.
 * This repairs binding evidence only: nine instruction words still differ,
 * and the ordinary ROM proof continues to use the assembly fallback.
 */

/* Matching reproof (2026-09-06): the body above supersedes that plateau.
 * Untouched configured compiler output owns all 248 bytes, with frame 40
 * and all six relocation identities exact. The linked owned range and full
 * ROM are byte-identical. The permuter's zero-valued local on the second
 * failure path was inert and is gone (lane c-5, 2026-10-09). The two
 * `& 0xFFFFFFFF` masks are not: the masked copy is a definition of `handle`
 * from `value`, so the loaded word, the argument and the call result are one
 * web and the load lands in v0 with a `move a0,v0` before each call, as
 * shipped. Without them `value` is propagated into the argument and loads
 * straight into a0 (9 words). `(u32) value`, `(s32) value`, a
 * `(s32) (u32)` round trip, and a `while ((value = descriptor[0]) != 0)` loop
 * reading the call result into `handle` all give that same object.
 */
