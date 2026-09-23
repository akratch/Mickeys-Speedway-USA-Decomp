<!-- plateau-handoff:ProcessRelocationEntry:start -->
### `ProcessRelocationEntry` plateau handoff

- source: `src/main/runlink.c`
- score: 126 differing words
- frame: 0x48
- relocations: 25
- first mismatch: +0x0
- summary: Remeasured 2026-09-23: 126 masked at size delta +4, frame 0x48 against 0x40, relocations 25 of 25; a1/a3 after the call stay structural.

First structured handoff. The plateau commit of 2026-08-25 recorded prose
only, in the source: -O2 -mips2 stays 0x4 long with 126 differing words from
+0x0; splitting the pointer lifetime removes s1 and yields the a3 caller
spill but grows the frame; type, pool-order, volatile-home, result-reuse and
register-hint variants miss a1/a3 after the call. This remeasure (preflight,
resolved through the guarded func_80031A30 fallback) reproduces that count.
<!-- plateau-handoff:ProcessRelocationEntry:end -->
