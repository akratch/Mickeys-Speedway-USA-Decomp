# Clean-Room Policy

This project aims to be a fully public, legally distributable matching
decompilation. To keep it publishable:

## Never committed to this repository
- ROM images or partial ROM dumps (any region/version)
- Extracted assets, disassembly (`asm/`), or binary blobs derived from a ROM
- Proprietary compiler binaries (IDO) or SDK binaries

## Prohibited knowledge sources
Code, symbol names, comments, or structure may NOT be derived from:
- Any leaked Rare/Nintendo source code
- The leaked Star Fox Adventures debug build (July 2002) or its symbols
- The leaked Dinosaur Planet build or its symbols
- Any other unreleased/leaked build of any game

Contributors who have studied leaked Rare source code should not
contribute matched C implementations.

**Narrow exception, binary file-format facts only:** the *public*
Dinosaur Planet decompilation project (its documentation and tooling,
built from the 2000-12-01 development-cartridge dump released by Forest
of Illusion on 2021-02-20, not the leaked Rare source tree) may be
consulted purely to understand the binary layout of Rare's DLL/overlay
reloc-table format (field offsets, relocation types, table structure),
since Mickey's overlay system uses the same mechanism. This exception
never extends to symbol names, code, or comments, only structural/
format facts about how that dumped binary is laid out. See
`docs/adr/0008-provenance.md` for the full discussion, including why
dp64's own naming is not covered by this exception.

## Permitted knowledge sources
- Analysis of retail ROMs you legally possess
- Published matching-decomp repositories built from retail ROMs
  (Diddy Kong Racing, Jet Force Gemini, Perfect Dark, Banjo-Kazooie,
  Conker's Bad Fur Day): their symbol naming, their documentation, and
  **their C source, including adapting a function body from one**,
  where Mickey's ROM shows the same engine's code. Such repositories are
  themselves clean-room work product built from retail ROMs, so this is
  not a route back to any prohibited source above. Two conditions:
  **(a)** the adapted body must be disclosed with a `PROVENANCE` note at
  the point of use, naming the project and the file it came from, and
  **(b)** Mickey's own ROM decides every disagreement; a borrowed body
  is a starting point to be matched against this game's bytes, never an
  authority over them. `src/main/runlink.c` is the worked example, and
  `docs/modules.md` section 1.3 states how the disclosure is written.
  A **decomp.me scratch** for one of these five games counts as the same
  source under the same two conditions, whether or not it has landed on
  the project's main branch: it is the same retail-derived matching work
  by the same community, only earlier. The `PROVENANCE` note then names
  the game, the scratch slug and its author instead of a file, and
  condition (b) applies unchanged -- a scratch's decomp.me score, and in
  particular a `match_override`, says nothing about Mickey's bytes.
  The same holds for **unmerged pull requests and non-default branches**
  of these projects' public repositories (owner decision, 2026-10-09):
  they are the same CC0 retail-derived work, published earlier. The
  `PROVENANCE` note names the project, the pull request number or branch,
  the commit, and the file; condition (b) applies unchanged.
- Official Nintendo 64 SDK documentation, headers, and library source as
  distributed in existing public decomp projects
- Emulator tracing/debugging of retail ROMs

## Build inputs
The build requires you to supply your own legally dumped ROM
(`baseroms/`), verified by SHA1 before use.

## What the automated gates can and cannot do

`tools/cleanroom_check.sh` runs at commit, at push, and in CI.

**Content detection of arbitrarily-encoded data is undecidable in general.**
Any file can be made to carry ROM bytes in a form no detector anticipates. The
content rules here are calibrated against *mistakes*, not against an adversary,
and they are measured: every threshold is set from the whole of this
repository's history on the pass side and fixtures of real ROM data on the
fail side.

### What the content layer reliably catches

Every line below was measured by running the detectors against a fixture built
from this game's own ROM: 400 real instruction words, or 1600 bytes, per
fixture. The numbers in brackets are `words / spread` as the detectors scored
them, against per-file limits of 192 words **and** 32 distinct high bytes.

- disassembly listings (`asm/*.s`, `objdump` output), upper or lower case
- hexdumps: `xxd` [485/58], `xxd -p` [1146/73], `hexdump -C`, `od`,
  64-column hex lines
- C arrays of ROM words or bytes, including `u`/`UL` type suffixes and
  underscore separators [400/40], and **zero-padded** to 9 digits [836/44]
- machine words in prose, in hex ranges, as dotted quads [400/40], in octal
  [1040/75], in decimal [985/97]
- **hex 16-bit halves, however they are labelled**: bare [385/58],
  comma-separated [385/58], one per line [385/58], as JSON array pairs
  `"w0": ["0x27bd", "0xffe8"]` [375/57], as split `_hi`/`_lo` keys [375/57],
  as `\u27bd\uffe8` escapes [385/58], and as markdown table rows [385/58].
  The gate is **volume, not adjacency**: a file must carry 64 halves pairs
  before any are read as words (see below)
- escaped byte strings (`"\x27\xbd..."`) [400/40]
- base64 [400/40], base64url [386/117], base32 [415/50] and ascii85 [342/167],
  **line-wrapped at any width** (every column from 4 to 40 was measured with
  a faithful hard column wrap, for base64 *and* for ascii85, and every one is
  caught [400/40]), and split across several files. Both wrap floors are 4,
  measured to cost nothing: over every tracked file the tightest margin is
  identical at every floor from 32 down to 4, and no file fires at any of them
- runs whose length is **undecodable for their encoding**: base64 at 1 mod 4,
  base32 at 1/3/6 mod 8, ascii85 at 1 mod 5 [399/40, 413/50, 399/40]
- JSON ledgers of any of the above, single-line or pretty-printed [436/41]
- the two workbench ledgers that caused the original incident [5593/201 and
  263/47]
- a leak spread across many files **when each file is word-dense**: 3 files of
  134 words each score 436 against an aggregate budget of 96

### What it does not catch

Known, measured holes, not speculation. Each was reproduced against the
current detectors.

- **High-byte stripping.** Write each word as six hex digits with the top byte
  dropped and the file scores **zero words, zero spread**; every rule here is
  silent. This is not "an encoding we did not implement": it is a
  one-character change to a form listed above as caught, and the top byte of a
  MIPS text section is nearly constant within an opcode class, so the
  transform is trivially invertible by whoever wrote it. It defeats the
  *statistic*, not the decoder. `spread` is the discriminator carrying the
  per-file rule, and any bijective transform that collapses the high byte
  (masking it, storing it separately, shifting the alignment) takes the rule
  with it.
- **Prose dilution.** A single file carrying **191 `0x`-prefixed words, about
  764 bytes of ROM**, one per padded sentence, passes everything. The ceiling
  is set by the rule, not by a fixture: `WORD_TABLE_COUNT_LIMIT` is 192, so 191
  is the most a file can yield and still pass, and the aggregate ignores it
  because the aggregate's rate floor (12 words/KiB) is a *per-file* rate that
  padding drives arbitrarily low. Eight such files carry roughly **6 KB**
  (8 × 764 = 6112 bytes) with the aggregate budget still reading **0**;
  measured, not inferred.
- **Decimal 16-bit halves.** Hex halves are recombined into words; the same
  split written in decimal is not, and a 400-word fixture scores **0**.
- **Fewer than 64 hex halves pairs in a file.** Halves are gated on volume: a
  file must contain at least 64 pairs (two 4-digit hex runs separated by at
  most 24 characters) before any pair is read as a word. A file carrying 63
  pairs (about 252 bytes of ROM in halves form) is not caught by this route.

  Volume, rather than adjacency, is what separates a dump from prose, and both
  narrower rules were defeated when tried. Pairing at **unbounded distance**
  fused a value on line 18 with one on line 234 and invented high bytes in this
  project's own documents. Requiring a **chain of tightly-adjacent** halves
  killed those phantoms but was defeated by putting a label between the pairs:
  keyed JSON, split `_hi`/`_lo` keys, `\u` escapes and markdown rows all reset
  the chain at every label and scored **zero**. Measured at a 24-character gap,
  the most pairs in any blob in this repository's entire history is **14**,
  while the weakest of nine halves fixture families carries **375**.
- **Digest-shaped strings.** The first 64 distinct 32/40/64/128-character hex
  tokens per file are exempt where they appear inline beside other content,
  because this tree legitimately records hashes and a SHA-256 is
  indistinguishable from ROM data by any content metric. Tokens past the 64th
  count normally, so the exemption is a slope rather than a cliff: measured, a
  file of digest-shaped ROM passes at 64 and 65 tokens and is caught at 88.
  **Deliberately abused that is about 2.7 KiB of ROM per file**; 87 digests is
  the ceiling measured on the standard fixture region (ROM 0x32630), and it is
  **strongly** byte-value dependent, because what actually trips is the spread
  of the words they decode to rather than their count: re-measured over six ROM
  regions the ceiling ranges from 87 to 141 digests, 2.7 KiB to 4.4 KiB. Quote
  it as a range, or as "about 2.7 KiB on the measured region"; a single figure
  is a fixture property, not a property of the rule. The exemption's width is
  the price of removing a discontinuity that made a documentation file
  recording a 65th hash score 520 words: a false positive on legitimate
  growth, sitting directly under the tightest margin in the tree.
- **Deliberate steganography.** Any encoding the normalizer does not
  implement, data hidden in whitespace or in the low bits of otherwise
  plausible numbers, or content split below every threshold on purpose.
- **Anything under a name the workbench redactor does not recognise** (see
  `ledger_redaction._sweep`: it covers target-naming keys in any case at any
  depth in any container, but it does not read string *contents*, so target
  code under an innocuous key name or as a bare list element survives).
- **Absolute filesystem paths in the workbench manifests**, which are a
  publication problem rather than a clean-room one, and are *known and
  deliberately not scrubbed*. Both tracked manifests under
  `.decomp-workbench/campaigns/` record `/Users/<name>/…` in
  `identity_inputs`, and the manifest schema check permits them: `_m_path`
  requires a path, not a relative one. They are not quietly fixed because the
  quiet fix would be worse. `identity` is
  `sha256(json.dumps(identity_inputs, sort_keys=True, separators=(",",":")))`
  (verified, both manifests reproduce their recorded digest exactly), and the
  campaign directory name embeds its first twelve hex characters
  (`SetLinkSlot-ee19dab3a5b2`). Rewriting the paths to relative ones
  invalidates the digest and the directory name together, and re-deriving both
  would produce a record of a campaign that was never run under those inputs.
  **The fix at publication time is to drop `.decomp-workbench/` from the
  published tree, or to re-run the two campaigns from a neutral root and commit
  the manifests they genuinely produce**, not to edit these.

The pattern across all of these: **content detection here is calibrated for
accidents, and the accident case is well covered. Against someone trying, the
per-file rule dies to any transform that flattens `spread`, and the aggregate
dies to padding.** Both are one-line changes. Do not read the list above as a
perimeter.

### What is actually load-bearing

The exact `config/lane-reopen-authorizations.us.json` path has a reviewed
aggregate-budget exemption for schema-validated source/ledger Git hash leaves.
Its reasons and other non-pin content still contribute to the aggregate;
malformed schemas fail closed. This does not extend other word-table exemptions
to the aggregate. Identical bytes at a foreign path remain checked and contribute
the maximum applicable score once per scan unit, regardless of entry order.

The guarantees that do not depend on out-guessing an encoding are structural:

1. **The path whitelist under `.decomp-workbench/`**: only
   `campaigns/*/manifest.json` may ever be tracked, and that file is validated
   against a **path-typed allow-list**: every leaf the workbench writes is
   enumerated with its permitted JSON types and a checker for its value's
   shape, and anything else (unknown key, unexpected type, float where a
   float was not declared, out-of-range number, over-long string, over-long
   array, unexpected nesting) is rejected. A legitimate schema change fails
   the check until someone updates the table, and that is the intended cost.

   The guarantee is a *bound*, not "there is nowhere in it to put a ROM word":
   a sha256 is 32 arbitrary bytes and a manifest legitimately carries several,
   so typing cannot make a hash field carry less than a hash. What the
   validator does is count and cap them (at most 64 source records, so at most
   `4 + 2×64 = 132` digest-typed leaves) and force every other leaf into a
   shape with no room. **Residual capacity: about 4.2 KB, and only for someone
   deliberately writing ROM bytes into fields that are supposed to be hashes.**
   The two real manifests carry 34 and 16 digests. Everything outside those
   leaves is also measured statistically, with the schema-validated digests
   blanked first, so the file answers to both layers.
2. **The `asm/`, `assets/`, `baseroms/`, `expected/` path rules**, and the
   binary and oversize rules, which do not care about encoding at all.
3. **The tool-level fix**: the workbench ledger writer strips every
   target-naming field, at any depth, in any container, replacing it with a
   16-bit salted digest and a masked opcode. It does not read string contents,
   so it is not a guarantee that no instruction text can reach a ledger; it is
   a guarantee that the *schema no longer asks for it*, which is what made the
   original incident automatic. Ledgers stay gitignored regardless.
4. **Policy and review**: `CLAUDE.md`, `docs/CONTRIBUTING.md`, and a human
   reading the diff. The gates exist so a mistake is caught, not so review can
   be skipped.
5. **A server-side ruleset on the remote.** Every layer above is client-side
   and skippable with `--no-verify`, and CI reports after publication rather
   than preventing it. `protect-master` (GitHub ruleset id `20111399`, active
   on `master`) closes part of that gap: it blocks force-push
   (`non_fast_forward`) and branch deletion, which is genuinely
   non-bypassable. It does not, and structurally cannot, restrict *content*:
   GitHub rejected a push ruleset scoped to file path/extension/size with
   "only org-owned repos can have push rules," which this personal fork is
   not. A required status check on a protected branch remains the only layer
   that could block bad content before it lands rather than after; it has not
   been configured, and adopting it would mean routing changes through pull
   requests rather than pushing to `master` directly. See
   `docs/CONTRIBUTING.md`'s "Checks" section, including the corollary that
   force-push being blocked means a future history purge of `master` requires
   temporarily disabling `protect-master` first.

### False positives are a failure of this system too

A gate that fires on legitimate work gets turned off, and `--no-verify` turns
off *everything*. This policy positively encourages adapting function bodies
from the published decomps named above, so source files are expected to carry
hex constants, struct offsets and small instruction citations.

The margins are therefore watched. In the current tracked tree the tightest
non-exempt margin is **2.82×** (`src/main/frontend_37D50.c`, 68 words / spread
29 against limits of 192 / 32, protected by count). `symbol_addrs.us.txt` is
protected by spread rather than count: it carries 1466 words but only four
distinct high bytes, an 8.00× spread margin, which is why the rule is a pair.
**Watch these numbers as the tree grows**: they say whether the gate is still a
safety net or is about to become an obstacle.

Every margin recovered so far came from asking where a file's words actually
came from, not from adjusting a limit. `src/main/runlink.c`, this repository's
own worked example of sanctioned adaptation, once scored 162 words / spread 28
against limits of 192 / 32, about thirty words from firing, and 116 of those
words were base64 false decodes of `#pragma GLOBAL_ASM(...)` paths. The current
discriminators take it to 10 words / spread 4.

If a gate does fire on work you believe is legitimate, the answer is
`CONTENT_EXEMPTIONS` in `tools/cleanroom_detectors.py` (one path, one
detector, a written reason, and a diff someone reviews), **not** `--no-verify`.
The current entries are narrow exceptions for reviewed build/ledger metadata;
each reason is recorded beside the exact path and detector in that table.

No blob count is quoted here on purpose. It changes with every text commit, so
any number written down is stale the moment the next one lands.
`gmake audit-decoders AUDIT_ARGS=--all` prints the count it actually scanned;
that is the figure. It reads slightly *below*
`git rev-list --all | … | sort -u | wc -l`, because the audit skips empty and
non-text blobs; reconcile the two that way rather than assuming one is wrong.

Treat the content detectors as defence in depth against mistakes. They are not
an adversary-proof boundary and must not be described as one.

### Maintaining the detectors

> **A decoder change must be re-audited, never argued.**
> Run `gmake audit-decoders` after touching `normalize_words_by_stage` or
> anything it calls. Do not reason about whether the change is safe — run it.

`tools/cleanroom_detectors.py` decodes a file into candidate 32-bit words and
then measures that stream. Five separate times, what it measured was not in the
file: a `#pragma GLOBAL_ASM` path decoded as base64; symbol names like
`D_80081898` `_`-joined into words; unrelated markdown table cells fused as
16-bit halves; this repository's own `OBJDUMP=…` build variable decoded first
as base64 and then as ascii85; and ragged hex runs read from one end only.
Decoded garbage is uniformly distributed, so each one inflated `spread` (the
metric that decides whether a file looks like ROM), and two of them brought
this tree's own files to within 1.2× of failing the gate on work this policy
encourages. Twice, a fix re-routed the phantom instead of removing it, and
neither case was visible by reading the diff. Hence "run it", not "check it".

What `gmake audit-decoders` asserts:

1. the per-stage word buckets reconstruct `normalize_words`' real output
   exactly (there is one implementation, not a mirrored copy);
2. the decoder set is **closed in all three directions**: every declared stage
   has a ceiling, every ceiling has a stage, and every bucket the normalizer
   actually *produces* is declared. The third is not redundant: words in an
   undeclared bucket are still returned by `normalize_words`, so without it a
   phantom could ship while the audit reported success;
3. the eight *synthetic* decoders contribute **zero** words. Nothing in this
   tree is encoded, so a nonzero count is a false decode by definition, not a
   budget to spend;
4. the two decoders that read genuinely-written numbers (`hex-run`,
   `dec-token`) stay under a generous tripwire that catches a flood rather than
   budgeting normal growth.

**What it does not cover.** The audit looks in the false-*positive* direction
only: a decoder that starts inventing words. It is blind to the opposite
failure, a decoder that quietly *stops* producing words it should, because a
decoder going silent looks exactly like a decoder behaving. Two such defects
have occurred: a base64 run whose length was 1 mod 4 raised inside the decoder
and was swallowed, and a per-line floor meant narrow-wrapped payloads were
never joined. Neither moved a single number in the audit.

**False-negative coverage comes from `gmake check-fixtures`** (64 fixture
families of real ROM words in every encoding, wrapped at every width, each
asserted to still be caught), implemented in
[`tools/false_negative_fixtures.py`](../tools/false_negative_fixtures.py).

The fixtures are **generated at run time from `baseroms/mickey.us.z64` and
never written to disk**, which is the only available design: a fixture that
proves the detectors catch ROM data *is* ROM data, so committing one would
violate this policy and `gmake cleanroom` would correctly reject it. So this
check, like `gmake verify`, cannot run in CI (there is no baserom there), and
it fails rather than skips when the baserom is missing, because a green bar
from a suite that tested nothing is worse than a red one.

It asserts in both directions. Every fixture that carries ROM must be caught;
and every family corresponding to a hole listed above under "What it does not
catch" must still *pass*, so that if the detectors quietly get stronger this
document stops being able to overstate the weakness.

Verified by driving it: reverting either wrap floor from 4 to 32 leaves
`gmake audit-decoders` fully green (every blob in history, no decoder
inventing words), while `gmake check-fixtures` reports 16 missed families
including every base64 wrap from 4 to 31 columns; removing the
undecodable-length trim likewise leaves the audit green while the fixtures
report `b32_len_mod8_is_1` and `b64_wrap27` silently producing nothing. Run
both. Neither replaces the other, and a clean audit is not evidence that
anything still works.

`audit-decoders` runs over tracked files by default (a few seconds) and can
also inspect every blob reachable from every ref with `AUDIT_ARGS=--all`; that
scope grows with campaign history and may take several minutes. It is deliberately
**not** wired into `gmake cleanroom` or the git hooks: those run on every
commit, must stay fast, and must fail only for clean-room reasons. This check
answers a different question (*is the detector still measuring reality?*) and
is aimed at whoever edits a decoder. Making an unrelated red bar block commits
is how a gate ends up bypassed with `--no-verify`.
