<!-- plateau-handoff:func_overlay_054_F0000000_189ECA0:start -->
### `func_overlay_054_F0000000_189ECA0` plateau handoff

- source: `src/overlays/o054/overlay54Initialize.c`
- score: 127 differing words
- frame: 0x78
- relocations: 112
- first mismatch: +0xF0
- summary: One word long. Extra instruction is the unfilled delay of the context-equals-3 branch. L90 rewrites increment-then-i-lt-4 to not-equal-4; the target keeps slti 4 after increment. Delay fill is reachable only on inverted polarity.

#### Re-open under L90 and the one-instruction family (2026-09-19, lane/w22-o054)

Retained baseline remeasured on 5639d0a3: 972 bytes, size delta +4 (244 vs 243), masked 127, raw 144, frame 0x78 with an identical 12-slot ladder. Identity gate: instrumented `cc` `.text` is byte-identical to stock. `CDX_PROC=0` (one procedure, 50 p1 decisions, no p2). `--every-colour` was not run; it refuses until size 0.

The extra word is the nop delay of `bne ctx,3,else`. Target fills that slot with the lo-add of `gOverlay54Current` (D_668), having emitted the hi-half immediately before the branch, and rematerialises the same address in the then-arm after the nested allocate. Else uses the hoisted pointer. Candidate else and then both use at-style stores (`lui at` / `sw 0(at)`), so as1 has no addiu to put in the delay.

L90, measured not asserted:

- `i++ < 3` (retained) keeps slti, but against 3, with increment after the compare.
- `i++; while (i < 4)`, `++i < 4`, and `i++; while (i <= 3)` all rewrite to `li 4; bne s0,at`. Same length, wrong opcode. This is L90's basic-IV normalisation.
- `for (i = 0; i < 4; i++)` also rewrites (no slti), masked 127.
- A derived pointer exit (`src0 != base + 0x80`) scores 125 at the same +4, but replaces slti with a pointer bne. Trap 12: a better number on the wrong test. Not adopted.
- Pointer-difference `< 0x80` costs +4 words and still leaves the delay nop.
- Target has both the zero-move of i and slti-against-4 after increment. That pair is exactly the one L90 says a basic IV cannot emit.

One-instruction family, each eliminated:

- Missing copy of i, of the external word, and of `&gOverlay54Current`: flat at 127, or forwarded (L102). L104 with a redefined bss local still forwards into `sw 0x668(at)`.
- Fallthrough / goto with target polarity (`if != 3 goto else`) is byte-identical to the if/else. Invert (`if != 3` else-first) fills the delay with `lui at` and goes size -4, masked 140, with beq polarity. Wrong arm is the fallthrough; not adopted.
- Rematerialised address: BSS+0x668 stores and a pointer slot both forward (L102). An or-with-zero lock-break produces lui+addiu and a filled delay, but as branch-likely plus a duplicated else store, size +8 to +12.
- L144: dropping volatile and accessing locals through `&locals` is byte-identical to volatile on this function. The sentinel still reloads from `sp+0x44`; the delay stays nop.
- L97 `if (1) {}` before the if, `if (1) { slot = &current; }`, and `do { slot = &current; } while (0)`: 128 or flat, delay still nop.
- L131 / nested-call split into sequential temps: flat 127, confirming the prior closure.
- L111 third node via a hoisted word copy: no delay fill.

Decision variable that blocks: uopt forwards `&gOverlay54Current` into at-style stores, so the compare block never contains the lui+addiu pair whose last pick would fill the delay on bne polarity. Invert proves the delay is fillable, but only by flipping which arm falls through. L90's increment-then-slti-4 shape is still unreachable from a basic IV. Next lever is a source form that keeps a D_668 pointer temp in the compare block without forwarding, without bnel, and without inverting the branch, or a non-basic IV that keeps slti 4 after increment.

<!-- plateau-handoff:func_overlay_054_F0000000_189ECA0:end -->
