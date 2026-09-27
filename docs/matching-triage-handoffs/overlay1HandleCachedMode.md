<!-- plateau-handoff:overlay1HandleCachedMode:start -->
### `overlay1HandleCachedMode` plateau handoff

- source: `src/overlays/o001/overlay_001_tail.c`
- score: 0 differing words
- frame: 0x18
- relocations: 13
- first mismatch: none
- summary: MATCHED. This header claimed an open residual until 2026-09-16, when tools/check_shard_metrics.py reconciled every shard against the ranking; the function had already left the queue. The claim it carried was: inverted callback condition removes one instruction; remaining gap is structural clear-flow and relocation identity

## 2026-09-27: recover omitted ownership credit

The ordinary C implementation was already present; its atlas row was missing.
The coordinator's ownership-maintenance pass proves the 128-byte executable
range at overlay 1 text +0x61F0..+0x6270. A fresh configured full-TU stock
compile, with only its output path redirected and no postprocessing, has the
same owned bytes as the canonical object. Its 13 runtime relocation records
agree in count, offset, type and independently resolved static identity.
The header's older count of eleven was incomplete.

The atlas now records that exact range without changing the C body, compiler
flags, alias recipe or neighbouring ownership. The linked promotion proof
reports 32 exact words, frame 0x18, and 13/13 static relocation identities.
This is 128 bytes of recovered ownership credit, not newly reconstructed C.
No padding is credited. The fresh raw object and comparison receipts remain
in ignored build/phase100-proof.

Validation: fresh configured raw compile and raw/canonical owned-byte check;
raw relocation-surface comparison; promotion-proof; full ROM verify;
cleanroom; check-docs; check-scoreboard; check-overlay-syms; check-tooling.

<!-- plateau-handoff:overlay1HandleCachedMode:end -->
