# PAL and JPN: measured status (2026-10-22)

Only the US ROM builds today. The PAL and JPN splat configs and symbol files
date from the 2023 upstream import (`eee6f89ab`): stub yamls, `.sha1` files,
and two-entry `symbol_addrs.*.txt`. They were never modernised. This note
records where each version stands, so a regional campaign can start from
measurements rather than from scratch.

## Build status

Both versions fail at the same points:

- **`gmake VERSION=pal|jpn extract`**: splat 0.50.0 rejects the stub yaml
  with `Unrecognized YAML option(s): auto_all_sections`. Before that,
  `overlay_atlas.py --check` passes, but only because it always checks the
  US ROM, whatever `VERSION` is.
- **`gmake VERSION=pal|jpn`**: there is no rule for
  `mickey.{pal,jpn}.ld`.
- **`gmake VERSION=pal|jpn verify`**: there is no rule for
  `config/overlay_funcs_to_trap.{pal,jpn}.txt`.

The baseroms match `mickey.*.sha1` and `tools/verify_baseroms.sh`.

**Hazards before any regional build:**

- `BUILD_DIR` and `.splat-stamp` are not per version. `VERSION=pal`
  reported the US stamp as current, and once a PAL linker script exists, it
  would link the US objects.
- The 13 `#ifdef VERSION_us` guards in `n_synthesizer.c`, `n_event.c`,
  `n_csplayer.c` and `libaudio.h` were inherited from DKR. PAL and JPN both
  contain the US code in all 25 functions of those files, so the guards
  would select the wrong branch.
- `tools/overlay_tables.py` has the US table addresses hard-coded.

## Layout

All three ROMs have 107 overlay modules, 375 main-table records and 2,004
ORT rows. Only the positions move.

| | US | PAL | JPN |
|---|---|---|---|
| Resident code+data end (ROM) | 0x86640 | 0x86990 | 0x86720 |
| BSS size | 0x52D10 | 0x52D20 | 0x52D20 |
| Overlay tables start | 0x1848B70 | 0x184AAA0 | 0x1834690 |
| Overlay text / data | 0x72910 / 0xEF80 | 0x73DD0 / 0x13150 | 0x73B20 / 0xFB70 |
| Modules with text identical to US | | 59 of 107 | 95 of 107 |

## Code identity with US

These counts are US functions found unchanged in the other ROM, with only
their relocated fields masked.

| | Resident functions | Resident bytes | Overlay functions | Overlay bytes |
|---|---|---|---|---|
| PAL | 1,414 of 1,460 (96.8%) | 91.3% | 763 of 915 (83.4%) | 55.5% |
| JPN | 1,454 of 1,460 (99.6%) | 99.6% | 875 of 915 (95.6%) | 81.3% |

Resident data and rodata, compared in 64-byte chunks, are 97.9% identical in
PAL and 99.0% in JPN. Of the 249 resident file boundaries, 242 (PAL) and 243
(JPN) fall on a matched function, so they can be translated mechanically.

**What differs:**

- **PAL:** 198 functions (46 resident, 152 overlay).
  - 83 of them differ only in immediates, the 60→50 Hz rescale:
    60.0f→50.0f in 30 places, 120→100, 30→25, 240→200, 300→250. A shared
    frame-rate macro covers these.
  - Structural changes are in `gameVi.c`, `camera.c`, `anim.c`,
    `charControl.c` and `main.c`, and in overlays 101, 46 (+4.7 KB), 8, 26,
    29, 52, 58 and 60.
- **JPN:** 46 functions.
  - Resident changes are limited to `setLanguage`, `mainThread`,
    `func_80011CDC`, `func_8003A6B0`/`func_8003A700` and the entrypoint's
    literals.
  - Overlay 101 is largely rewritten (20 functions, about 37 KB, similarity
    0.18). There are smaller changes in 58, 60, 62, 63, 11 and 45.

## Plan and rough effort

1. **Per-version build plumbing (about a week).**
   - Per-version `BUILD_DIR` and stamp.
   - Per-version `config/*.json` and trap lists.
   - Remove the US assumption from the build-path tools.
   - Fix the audio guards.
   - Generate each version's yaml and `symbol_addrs` from the function map;
     data addresses come from the relocation pairs inside matched functions.
2. **Byte-identical PAL and JPN (about a week).** Guard the diverged
   functions with `#ifdef VERSION_*`, falling back to each version's own
   extracted assembly (built from the local baserom, never tracked).
3. **C for the diverged functions.**
   - JPN: about 2–4 weeks; overlay 101 is the long pole.
   - PAL: the frame-rate macro handles the 83 constant-only functions in
     days. The remaining ~115 functions (about 170 KB) are roughly 4–8 weeks
     of lane work.

The measurement scripts are kept outside the repository. Their intermediate
outputs hold ROM-derived data and must never be tracked.
