# Named donor source availability

`tools/donor_source_receipt.py` records one named counterpart query against
committed Git source. Use it before assigning a donor-based packet: a file
containing unrelated C does not establish that the requested counterpart has C.
The existing reopen donor guard checks TU-level availability; this report
provides the narrower counterpart-level evidence without changing that gate.

```sh
python3 tools/donor_source_receipt.py \
  --target-path src/main/anim.c --target-symbol func_800563B4 \
  --reference-repo "$REFERENCE_ROOT/jfg" --reference-ref "$REFERENCE_COMMIT" \
  --reference-path src/hit.c --counterpart hitVectorCheck \
  --basis 'Existing point-of-use provenance supplies a structural correspondence' \
  > build/donor-source-query.json
```

The output pins both committed source blobs and revisions. It reads no dirty
source and emits no source bodies. Reference objects are deliberately not
represented by the current source revision: `object_identity` is unset.
Authenticate any object scan separately using the locked mining surface and
its actual build revision. A correspondence basis is recorded as an assertion,
not silently certified by this tool.

The report distinguishes a direct C definition candidate, C with a fallback,
an exact named `GLOBAL_ASM` fallback, no direct definition, an absent file,
and tracked assembly with unreviewed origin. These are lexical findings.
Conditional compilation, macro-generated definitions and includes are not
resolved. C inside a disabled conditional is reported as present text with
configured availability explicitly unevaluated. A tracked assembly file still
needs independent origin review; being tracked is not proof it is original
source. The target must itself have a direct definition or exact named fallback
in the selected committed file. Indirect targets are outside this tool's scope.

A receipt covers only its named counterpart, path and revision. It never
establishes matrix exhaustion, source equivalence, assignment authorization,
or matching status. Regenerate it when source pins change; do not reuse a
receipt as a freshness or promotion certificate. Receipts remain ignored.

## First bounded application

At integration source `8cefb6117`, JFG source `efd5abb1c796` contains exact
named assembly fallbacks, with no direct C definition, for these queries:

| Mickey target | JFG counterpart | Reference file |
| --- | --- | --- |
| `func_800563B4` | `hitVectorCheck` | `src/hit.c` |
| `shadowGenerate` | `shadowGenerate` | `src/shadows.c` |
| `func_800517E0` | `animseqProcessCommandList` | `src/anim.c` |

The first correspondence comes from existing point-of-use provenance; the
second from the documented shadow family; the third from the existing bounded
donor audit. Their authority is unchanged. These queries eliminate a direct-C
lead only at that revision, not historical source, other permitted projects,
or compiler configurations. They produce no new matching credit. In
particular, unrelated C bitstream helpers in `src/anim.c` do not make the
command-list interpreter available as C.
