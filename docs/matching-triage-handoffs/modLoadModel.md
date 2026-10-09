<!-- plateau-handoff:modLoadModel:start -->
### `modLoadModel` plateau handoff

- source: `src/main/models.c`
- score: 0/401 words, promoted
- frame: 0x78
- relocations: 46
- first mismatch: none
- summary: Matched. DKR object_model_init shape: ASSETCACHE shift macros for the cache scan, VERSION_79 rollback flags as s8 locals, struct fields for the relocated pointers, a separate counter for the nested display-list loop, and the target's declaration order.

#### 2026-10-02, lane x-models: ROM-exact (365 to 0, promoted)

The inherited body was m2c output over raw offsets. DKR's object_model_init
(src/object_models.c, VERSION_79) is the same routine: model-id range check,
cache scan with a reference count bump, the free-list/new-slot pair with its
rollback flags, size from the compressed header plus sizeof(ObjectModel)
(0x80), asset load, inflate, pointer relocation, texture load loop, batch
texture-index check, the extra Mickey steps (nested display lists, instance
builder by the 0x8000 bit) and the shared failure tail. Measured steps:

- Port alone (struct fields on ObjectModel, which grew named fields and its
  0x80 size in include/game/models.h): 395 masked at size delta -32.
- DKR's ASSETCACHE_ID/PTR shift macros for the cache index: the scan keeps
  its sll/addu per iteration instead of a strength-reduced pointer, 386 at -20.
- A separate counter for the nested display-list loop: the shared i had made
  one long web that took s0 and pushed the model pointer to s1; split, i is
  caller-saved and spilled exactly where the target spills it. 50 masked,
  all immediates (frame homes), size delta 0.
- Declaration order for the home ladder, with one unreferenced s32 (L99):
  0 at delta 0.

gmake verify printed the expected SHA1 with the GLOBAL_ASM branch removed.

<!-- plateau-handoff:modLoadModel:end -->
