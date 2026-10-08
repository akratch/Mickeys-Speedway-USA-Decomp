#include "PR/ultratypes.h"

typedef struct Overlay79Object Overlay79Object;
typedef struct Overlay79Node Overlay79Node;
typedef struct Overlay79Spawned Overlay79Spawned;

typedef struct Overlay79Vector {
    f32 x;
    f32 y;
    f32 z;
} Overlay79Vector;

struct Overlay79Node {
    s32 active;
    Overlay79Node *next;
    u8 pad8[0x5C];
    Overlay79Spawned *state;
};

struct Overlay79Object {
    u8 pad0[0xC];
    Overlay79Vector position;
    u8 pad18[0x4C];
    Overlay79Node *node;
};

struct Overlay79Spawned {
    u8 pad0[0x3C];
    s32 field3C;
    s32 field40;
};

typedef struct Overlay79SpawnDesc {
    s16 objectId;
    u8 kind;
    u8 flags;
    s16 x;
    s16 y;
    s16 z;
    s16 angle;
    u8 padC[0xC];
} Overlay79SpawnDesc;

extern s32 overlay79RandomReloc(s32 lower, s32 upper);
extern Overlay79Spawned *overlay79SpawnReloc(Overlay79SpawnDesc *desc, s32 count);
extern void overlay79EmitAtReloc(s32 id, f32 x, f32 y, f32 z, s32 arg4, s32 arg5);
extern void overlay79FinishReloc(Overlay79Object *object);
extern s32 overlay79FindNearby(Overlay79Vector *position, f32 distance);
extern void overlay79EmitReloc(s32 id, s32 arg1);
extern void overlay79TriggerReloc(void);

extern s32 gOverlay79CounterReloc;
extern u8 gOverlay79FlagsReloc[];

/* Bounded reproof (2026-08-29): configured -O2 -mips2 -32 C remains exactly
 * 123 instructions / 492 bytes with frame 0x48, 111/123 raw and runtime-
 * normalized words, 12 register-field differences, and first mismatch +0xC8.
 * Its executable-text SHA-256 is d8b80e23d14ea9c2. All 119 flag identities
 * were attempted; seven O2/MIPS-II rows tie V0 and none is exact. A fidelity-
 * clean proc-0 globalcolor trace identifies separate v1/v0 linked-state webs.
 * A distinct linked-state local and branch-local linked-state scopes both
 * regress to frame 0x50 and 30 differences; a distinct next-node carrier
 * regresses to frame 0x50 and 43 differences. No strict gain authorized a
 * combination or generic batch. Earlier source-faithful lifetime/access forms
 * likewise remained at 12 or regressed.
 *
 * A follow-on trace-led pass spent 20 stock configured builds (two V0 proofs
 * and 18 new representation, ABI, expression, scoped-constant, and declaration-
 * order forms) plus one fidelity-clean instrumented ugen capture. The trace
 * attributes the tail to a one-slot temp-FIFO phase: the candidate allocates
 * t1/t2 for the flags update and t3 for the alternate emit, while the target
 * requires t2/t3 and t4. The linked-state loads remain an independent v1/v0
 * global-color outcome. Every new form was byte-flat at 12 differences or
 * regressed to 14-42; the scoped emit-id local was exact-sized but regressed
 * to 33. No JFG function supplies a body, ABI, or allocator mechanism here.
 *
 * A resumed bounded pass (2026-08-30) proved the missing temp-FIFO mechanism.
 * Preserving the u8 width explicitly as `(value & 0xFF) | 4` consumes one
 * backend temp while the redundant mask emits no instruction. The retained C
 * remains exactly 123 words / 492 bytes with frame 0x48 and first mismatch
 * +0xC8, improves to 119/123 raw and runtime-normalized words, and reproduces
 * the complete integer temp lane. An opaque `void *` state slot and a zero-
 * offset typed state-slot wrapper were independently byte-flat on top of that
 * gain. The sole executable residual is now the four-site v1/v0 linked-state
 * pool web; no source-faithful coalescing form was found within the narrow
 * follow-up cap.
 *
 * The owned +0x1290..+0x147C range has no target padding. IDO's four trailing
 * alignment bytes are outside the function and production trims only them.
 * The C object emits all 15 shipped runtime tuples. Fresh ordinary preflight
 * proves every offset/type but resolves only one candidate identity and fails
 * closed on the other 14. A prior identity-canonical diagnostic comparison
 * proves all 15 identities/addends; it is evidence, not promotion machinery.
 * The assembled target has 13 static records because the runtime table adds the
 * already-zero flags pair. The committed filter/rebind metadata remains
 * diagnostic and is not production POSTPROCESS, so linked C proof is absent.
 * ORT 1297 and resident relocation 139 establish func_8000AEEC+0x43C as the
 * sole inbound; no cross-overlay inbound exists. Pinned DKR v77/v80 and JFG
 * scans are negative. Preserve the fallback and resume only with a newly
 * proved natural v0/v1 linked-state coalescing form; do not repeat the flag
 * lattice, temp-FIFO lever, state-slot type forms, or a generic batch.
 *
 * Naturalness pass (lane c-1, 2026-10-08): the empty `if (object->position.y)`
 * and the `if (1)` that opened the else arm were byte-inert and are gone. The
 * empty `if (1)` between the `field3C` store and the emit call is load-bearing:
 * without it the linked-state load after the call takes v1 instead of v0 at
 * both sites (4 words), and storing through `node->next->state` directly
 * instead of reusing `spawned` costs 26 words. */
void func_overlay_079_F0001290_18CE230(Overlay79Object *object, s32 arg1)
{
  Overlay79Node *node;
  Overlay79SpawnDesc desc;
  Overlay79Spawned *spawned;
  node = object->node;
  if (node->active != 0)
  {
    desc.objectId = 0x14B;
    desc.kind = 0xC;
    desc.flags = 0;
    desc.x = object->position.x;
    desc.y = object->position.y;
    desc.z = object->position.z;
    desc.angle = overlay79RandomReloc(-0x7FFF, 0x8000);
    spawned = overlay79SpawnReloc(&desc, 1);
    if (spawned != 0)
    {
      spawned->field3C = 0;
      if (1)
      {
      }
      overlay79EmitAtReloc(0x277, object->position.x, object->position.y, object->position.z, 4, 0);
    }
    spawned = node->next->state;
    spawned->field40 = 0;
    overlay79FinishReloc(object);
  }
  else
  {
    if (overlay79FindNearby(&object->position, 900.0f) != 0)
    {
      desc.objectId = 0x14B;
      desc.kind = 0xC;
      desc.flags = 0;
      desc.x = object->position.x;
      desc.y = object->position.y;
      desc.z = object->position.z;
      desc.angle = overlay79RandomReloc(-0x7FFF, 0x8000);
      spawned = overlay79SpawnReloc(&desc, 1);
      if (spawned != 0)
      {
        spawned->field3C = 0;
        if ((++gOverlay79CounterReloc) == 0x14)
        {
          overlay79EmitReloc(0x27C, 0);
          gOverlay79FlagsReloc[1] = (gOverlay79FlagsReloc[1] & 0xFF) | 4;
          overlay79TriggerReloc();
        }
        else
        {
          overlay79EmitAtReloc(0x277, object->position.x, object->position.y, object->position.z, 4, 0);
        }
      }
      spawned = node->next->state;
      spawned->field40 = 0;
      overlay79FinishReloc(object);
    }
  }
}
