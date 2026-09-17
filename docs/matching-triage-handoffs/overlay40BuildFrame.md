<!-- plateau-handoff:overlay40BuildFrame:start -->
### `overlay40BuildFrame` plateau handoff

- source: `src/overlays/o040/overlay40BuildFrame.c`
- score: 75/81 words
- frame: 0xB0
- relocations: 1
- first mismatch: +0x4
- summary: colour floor is 75; record homes now match at sp+0x40; store schedule remains

#### 2026-09-17, lane w5-o040: exhaustive colour landscape, then record homes

Identity gate: instrumented IDO text, relocations and symbols match the configured object; both score 75, delta 0, first +0x4, frame 0xB0. CDX_PROC is 0 (procindex one row, 13 p1 decisions, 0 p2). The procedure contains a call, so p1 only.

Every-colour same-kind landscape: 56 probes over 13 coloured webs. Zero probes beat 75 at delta 0. 36 accepted forces moved no residual word (confirmed forced equals the requested colour, not -2). Webs 2, 4 and 10 (argument colours) change size when forced off their colour. Webs 16 and 175 have no second same-kind colour. L159 packing is empty. Colour cannot close this; L160's force-scores-0 then delete-the-carrier route does not apply.

Diagnose named stack-home. Declaring the four scalars, then records[8], then s32 scratch[2] with (void)&scratch, lands the eight 12-byte records at sp+0x40 matching the target and holds frame 0xB0. Aligned exact 7 to 13, immediate 1 to 0, structural 54 to 19. Positional masked stays 75: the home-set change opens 17 candidate-only and 17 target-only words (store schedule). Pad-before-records, pad-after-records, and scratch-first orders leave records at the wrong slot. Inlining the named intermediates is 3 words short. Rec0-bottom-first and by-value store orders on the home-correct shape are byte-flat or worse.

Next: store schedule / emission order on this home set. Colour is closed. Prior statement-order evidence on the padded-struct shape is void (L146).
<!-- plateau-handoff:overlay40BuildFrame:end -->
