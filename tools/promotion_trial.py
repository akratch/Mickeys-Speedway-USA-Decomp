#!/usr/bin/env python3
"""Linked-ROM promotion trial for every NON_MATCHING candidate.

The permuter scores a scratch object, and for overlay functions that scratch
can never reach zero: the splat target spells every call relocation with a
placeholder symbol and the real build resolves calls through the overlay
relocation table, so identical instruction words still score as mismatches.
The only oracle that counts is the linked ROM. This tool, for each candidate:

  1. splices the candidate over its `#ifdef NON_MATCHING ... #endif` wrapper
     (exactly what a promotion does), rebuilds the ROM, and byte-compares it
     with the baserom;
  2. maps every differing byte to the function's own ROM range or outside it;
  3. restores the source and records the result.

Classes reported per function:
  exact          ROM byte-identical: the candidate can be promoted as is
  text-exact     no difference inside the function's range, some outside it
                 (data/bss layout or relocation-table collateral: an
                 ownership fix, not code work)
  text-differs   N words differ inside the function's range
  rom-size       the promoted module is a different size than the ROM's, so
                 everything behind it slid; the cause carries the signed byte
                 delta and `in_range_words` still counts the function's own
                 differing words. An ownership carve, not code work.
  build-error    the promoted tree did not build

Run it in a lane, never in the canonical worktree: it rewrites source files
one at a time and rebuilds ~20 s per candidate.

    tools/promotion_trial.py [--overlay N] [--function NAME] [--limit N]
                             [--resume] [--jobs 6]

Results accumulate in build/promotion-trial.json and .txt.

Acceptance procedure (needs a baserom, so no CI test can stand in for it).
The trial's own soundness is checked with a candidate whose quality is not in
question: an overlay function that is already matched.

  1. pick a matched, single-function overlay TU, e.g.
     src/overlays/o063/overlay63Initialize.c;
  2. in a lane only, wrap its body as
     `#ifdef NON_MATCHING <body> #else #pragma GLOBAL_ASM("...") #endif`,
     naming the splat auto-name path
     asm/nonmatchings/overlays/o063/overlay63Initialize/func_overlay_063_F0000000_18C2B88.s;
  3. `tools/overlay_atlas.py --write` (the manifest records per-row
     `nonmatching`) and `gmake extract`, so the fallback .s exists;
  4. `tools/promotion_trial.py --function overlay63Initialize`.

It MUST report `exact  in=0  out=0`. Anything else is a fault in the harness,
not in a candidate: the tree it just built is the tree that verifies. Restore
the file, the manifest and `gmake extract` afterwards.

tests/test_overlay_atlas.py::TrialProjectionTests covers the part of that
procedure a unit test can reach without ROM data.
"""

from __future__ import annotations

import argparse
import dataclasses
import json
import os
import re
import subprocess
import sys
import time
from pathlib import Path
from typing import Optional

sys.path.insert(0, str(Path(__file__).resolve().parent))
import permute_batch as pb  # noqa: E402
import reloc_surface as rs  # noqa: E402

ROOT = pb.ROOT
BASEROM = ROOT / "baseroms" / "mickey.us.z64"
# Compare the raw linked image. Building .z64 would additionally execute the
# generated host n64crc helper; a promotion trial needs only compile, link,
# objcopy, and byte comparison, and the canonical .bin includes the same full
# image bytes before that no-op checksum gate.
ROM = ROOT / "build" / "mickey.us.bin"
ROM_TARGET = str(ROM.relative_to(ROOT))
OUT_JSON = ROOT / "build" / "promotion-trial.json"
OUT_TXT = ROOT / "build" / "promotion-trial.txt"
LINK_SYMS = ROOT / "overlay_undefined_syms.us.txt"
# `generate --write` also rewrites the declared resident-call rebinds; a trial
# restores both, or a candidate's new rule would outlive the candidate.
REBINDS = ROOT / "mk" / "overlay_resident_rebinds.generated.mk"
ATLAS_TOOL = ROOT / "tools" / "overlay_atlas.py"
MANIFEST = ROOT / "config" / "overlays.us.json"
YAML = ROOT / "mickey.us.yaml"

# A relocation the link cannot resolve is reported by ld as one of these.
UNDEF_RE = re.compile(r"undefined reference to [`'\"]([A-Za-z_][A-Za-z0-9_]*)")
MARKER_RE = re.compile(r"PROMOTION-TRIAL: ([^:]+): (.*)")
# /* ALIGNED overlay_027.c.o: 47/47 site(s), 1 shifted */ -- the relocation
# surface's own report that this object's sites were matched to the shipped
# records by order rather than by identical offsets.
ALIGNED_RE = re.compile(r"ALIGNED (\S+): (\d+)/(\d+) site\(s\), (\d+) shifted")
TRUNC_RE = re.compile(r"relocation truncated to fit: (\S+)")
# A splat auto-name in the resident address space: func_8002997C, D_80003634.
RESIDENT_RE = re.compile(r"^(func|D)_8[0-9A-F]{7}$")
LOAD_LIMIT = 12.0


def wait_for_load(label: str) -> None:
    """Keep the occupied workstation below the campaign load ceiling."""
    while True:
        raw = subprocess.check_output(["sysctl", "-n", "vm.loadavg"],
                                      text=True)
        match = re.search(r"[-+]?\d+(?:\.\d+)?", raw)
        if match is None:
            raise RuntimeError(f"cannot parse vm.loadavg: {raw!r}")
        load = float(match.group(0))
        if load <= LOAD_LIMIT:
            print(f"{label}: load {load:.2f}", flush=True)
            return
        print(f"{label}: load {load:.2f} > {LOAD_LIMIT:.0f}; waiting",
              flush=True)
        time.sleep(20)


@dataclasses.dataclass
class Trial:
    func: str
    c_file: str
    overlay: Optional[int]
    rom_start: Optional[int]
    rom_end: Optional[int]
    klass: str = "unknown"
    in_range_words: int = 0
    out_of_range_bytes: int = 0
    first_in_range: Optional[int] = None
    first_out_of_range: Optional[int] = None
    seconds: float = 0.0
    aligned_sites: int = 0
    shifted_sites: int = 0
    error: Optional[str] = None
    cause: Optional[str] = None
    diffs: Optional[list] = None  # [(fn_offset, target_word, built_word, reloc)] for in-range words


def overlay_ranges() -> dict[str, tuple[int, int]]:
    """source stem -> (rom_start, rom_end) for every overlay ownership row."""
    atlas = json.loads((ROOT / "config" / "overlays.us.json").read_text())
    out: dict[str, tuple[int, int]] = {}
    for module in atlas["modules"]:
        base = int(module["rom"]["start"], 16)
        for row in module["text_ownership"]:
            if row.get("type") != "c":
                continue
            start = base + int(row["offset"], 16)
            out[row["source"]] = (start, start + int(row["size"], 16))
    return out


def resident_ranges() -> dict[str, tuple[int, int]]:
    """function -> (rom_start, rom_end) from the linked ELF (resident only)."""
    elf = ROOT / "build" / "mickey.us.elf"
    objdump = ROOT / "tools" / "binutils" / "mips64-elf-objdump"
    if not elf.is_file():
        return {}
    text = subprocess.run([str(objdump), "-t", str(elf)], capture_output=True, text=True).stdout
    out: dict[str, tuple[int, int]] = {}
    for line in text.splitlines():
        if " F " not in line:
            continue
        tok = line.split()
        if len(tok) < 4:
            continue
        name, size_hex, section = tok[-1], tok[-2], tok[-3]
        if not section.startswith(".text"):
            continue
        try:
            vram = int(tok[0], 16)
            size = int(size_hex, 16)
        except ValueError:
            continue
        # Resident text is loaded contiguously from ROM 0x1000 at 0x80000400.
        rom = vram - 0x80000400 + 0x1000
        out[name] = (rom, rom + size)
    return out


def splice(item: pb.QueueItem) -> Optional[str]:
    """Promote the candidate in place; return the original text (or None)."""
    original = item.c_file.read_text()

    def replace_block(m: re.Match) -> str:
        fn = pb.FUNC_DEF_RE.search(m.group("body"))
        if fn and fn.group("name") == item.func:
            return m.group("body")
        return m.group(0)

    new_text, n = pb.NON_MATCHING_BLOCK_RE.subn(replace_block, original)
    if n == 0 or new_text == original:
        return None
    item.c_file.write_text(new_text)
    return original


def explain_in_range(item: pb.QueueItem, rng: tuple[int, int], tu_text_offset: int,
                     inside: list[int]) -> list:
    """For each differing in-range word: the target word, the built word and,
    if the promoted object carries a relocation at that site, its symbol and
    addend. Must run before the source is restored (the object is the
    candidate's)."""
    rom = ROM.read_bytes()
    base = BASEROM.read_bytes()
    rel = item.c_file.relative_to(ROOT)
    obj = ROOT / "build" / rel.parent / (rel.name + ".o")
    relocs: dict[int, str] = {}
    if obj.is_file():
        try:
            elf = rs.Elf(obj)
            syms = elf.symbols()
            for sec, off, typ, si in elf.relocations(target=r".*"):
                if sec == ".text":
                    name = syms[si][0] if si < len(syms) else "?"
                    relocs[off] = f"{typ} {name}"
        except Exception:  # noqa: BLE001
            pass
    out = []
    for w in sorted({d & ~3 for d in inside})[:16]:
        fn_off = w - rng[0]
        site = fn_off + tu_text_offset
        out.append((fn_off, base[w:w + 4].hex(), rom[w:w + 4].hex(), relocs.get(site)))
    return out


def trial_source(item: pb.QueueItem) -> str:
    """The TU stem this trial promotes, e.g. overlays/o001/overlay_001_head."""
    return str(item.c_file.relative_to(ROOT / "src")).removesuffix(".c")


def tu_text_offset(item: pb.QueueItem) -> int:
    """Offset of this TU's .text within its module (its lowest ownership row)."""
    atlas = json.loads((ROOT / "config" / "overlays.us.json").read_text())
    stem = str(item.c_file.relative_to(ROOT / "src")).removesuffix(".c")
    for module in atlas["modules"]:
        if module["overlay"] != item.overlay:
            continue
        offs = [int(r["offset"], 16) for r in module["text_ownership"] if r.get("source") == stem]
        if offs:
            return min(offs)
    return 0


def build(jobs: int, full_log: bool = False,
          source: Optional[str] = None,
          function: Optional[str] = None) -> tuple[bool, str]:
    """One `gmake` pass with the POSTPROCESS guards in report-and-skip mode.

    PROMOTION_TRIAL turns every digest-guarded normalization into a marker line
    plus a skipped pass (tools/postprocess_guard.py), so a candidate whose
    codegen is the wrong size still reaches the link and still produces a ROM.
    The ROM is not a valid build and is never verified; it exists to be diffed.
    """
    wait_for_load("promotion build")
    env = dict(os.environ, PROMOTION_TRIAL="1")
    if source is not None:
        # The splat stamp's own `overlay_atlas.py --check` re-renders the
        # projection; without the same trial source it renders the carve-less
        # one and fails against the yaml the trial just wrote.
        env["PROMOTION_TRIAL_SOURCE"] = source
    if function is not None:
        env["PROMOTION_TRIAL_FUNCTION"] = function
    r = subprocess.run(["gmake", f"-j{jobs}", ROM_TARGET], cwd=ROOT, capture_output=True,
                       text=True, timeout=1800, env=env)
    log = r.stdout + r.stderr
    return r.returncode == 0, log if full_log else log[-6000:]


def synthesize_surface() -> tuple[bool, str]:
    """Regenerate overlay_undefined_syms.us.txt from the objects on disk.

    This is the whole point of the integration. A promoted candidate spells its
    cross-module and section references with placeholder externs that have no
    address in this build; the surface supplies each one's stored addend, read
    from the baserom at the site the module's own relocation table names. It
    has to run *after* the candidate compiles and *before* the link, which is
    exactly the window a plain `gmake` does not offer -- so the trial builds,
    regenerates, and builds again. The second pass relinks only.
    """
    r = subprocess.run([sys.executable, str(ROOT / "tools" / "reloc_surface.py"),
                        "generate", "--write", "--quiet"],
                       cwd=ROOT, capture_output=True, text=True, timeout=900)
    return r.returncode == 0, r.stdout + r.stderr


def undefined_sites(c_file: Path, symbol: str) -> set[str]:
    """Section names of every relocation in `c_file`'s object naming `symbol`."""
    rel = c_file.relative_to(ROOT)
    obj = ROOT / "build" / rel.parent / (rel.name + ".o")
    if not obj.is_file():
        return set()
    try:
        elf = rs.Elf(obj)
    except SystemExit:
        return set()
    syms = elf.symbols()
    idx = {i for i, s in enumerate(syms) if s[0] == symbol}
    out = set()
    for sec, _off, _t, si in elf.relocations(target=r".*"):
        if si in idx:
            out.add(sec)
    return out


def classify_failure(log: str, item, surface_log: str,
                     link_log: Optional[str] = None) -> tuple[str, str]:
    """(class, cause) for a build that did not produce a ROM.

    The four classes the relocation surface cannot close are named explicitly,
    because each needs different work and lumping them as `build-error` is what
    made the previous run's 169 failures unreadable.

    `log` is both passes concatenated, because a POSTPROCESS marker is printed
    by the *compile* pass. `link_log` is the pass that ran after the surface was
    regenerated, and every link diagnostic must be read from it alone: the first
    pass links against the stale surface and its "undefined reference" lines
    survive into `log`, so scanning `log` reports symbols the second link
    resolved perfectly well. That misread is what kept six candidates in
    `resident-symbol-missing` after the surface had already valued them.
    """
    markers = MARKER_RE.findall(log)
    for kind, message in markers:
        if item.c_file.stem in message or item.func in message:
            return "text-size-differs", kind
    if markers:
        return "text-size-differs", markers[0][0]

    link = log if link_log is None else link_log
    undef = UNDEF_RE.findall(link)
    if undef:
        # Not a relocation-surface problem at all: a resident symbol the tree
        # does not define yet. Its own address is what the reference wants;
        # there is no addend to synthesize.
        # The surface refuses a resident call it cannot read an addend for,
        # and says why. That refusal is the honest class, so it is tested
        # before the bare "the name is a resident auto-name" fallback below.
        refused = {m for m in undef
                   if re.search(r"\b%s\b: resident " % re.escape(m),
                                surface_log)}
        if refused:
            return "build-error", "resident-call-unreadable (%s)" % \
                ", ".join(sorted(refused)[:3])
        resident = [m for m in undef if RESIDENT_RE.match(m)]
        if resident:
            return "build-error", "resident-symbol-missing (%s)" % \
                ", ".join(sorted(set(resident))[:3])
        # A symbol the synthesizer refused because the candidate's schedule
        # disagrees at the placeholder's own sites: either two sites demand
        # different addends, or the module's table corroborates the symbol but
        # not at any site this object still spells the same way. Either way no
        # consistent addend exists and the tool reports rather than invents.
        diverged = {m for m in undef
                    if re.search(r"\b%s\b: (\d+ distinct values|no corroborated"
                                 r" site)" % re.escape(m), surface_log)}
        if diverged:
            return "build-error", "schedule-divergence-at-site (%s)" % \
                ", ".join(sorted(diverged)[:3])
        unmapped = {m for m in undef
                    if re.search(r"\b%s\b: unmapped site" % re.escape(m),
                                 surface_log)}
        if unmapped:
            return "build-error", "unmapped-site (%s)" % \
                ", ".join(sorted(unmapped)[:3])
        aliased = [m for m in undef if rs.GEN_NAME_RE.match(m)]
        if aliased:
            return "build-error", "alias-coupling (%s)" % \
                ", ".join(sorted(aliased)[:3])
        nontext = [m for m in undef
                   if undefined_sites(item.c_file, m) - {".text"}]
        if nontext:
            return "build-error", "non-text-site (%s)" % \
                ", ".join(sorted(nontext)[:3])
        return "build-error", "unresolved-placeholder (%s)" % \
            ", ".join(sorted(set(undef))[:3])

    trunc = TRUNC_RE.findall(link)
    if trunc:
        return "build-error", "relocation-truncated (%s)" % \
            ", ".join(sorted(set(trunc))[:3])
    if re.search(r"\.c\.o\] Error|cfe: Error|\bError:", log):
        return "build-error", "compile-error"
    return "build-error", "other"


def alignment_counts(item, surface_log: str) -> tuple[int, int]:
    """(aligned, shifted) sites the surface reported for this candidate's object.

    Reported on every row the aligner touched, so a shifted site is never
    silently read as an exact one: the words it moved still count as
    differences in `in_range_words`, which the ROM comparison alone decides.
    """
    obj = item.c_file.name + ".o"
    for name, aligned, _total, shifted in ALIGNED_RE.findall(surface_log):
        if name == obj:
            return int(aligned), int(shifted)
    return 0, 0


def module_size_delta(overlay: Optional[int]) -> Optional[int]:
    """built module size - the ROM's, in bytes, or None if unknown.

    A promoted candidate that emits its own .rodata/.data with no ownership
    carve makes its module longer or shorter than the ROM's; the linker then
    slides every module behind it and the byte compare reports the whole
    remainder of the overlay region as out-of-range. That is an ownership
    question, not a codegen one, so it gets its own class instead of being
    folded into `text-differs`.
    """
    if overlay is None:
        return None
    elf = ROOT / "build" / "mickey.us.elf"
    objdump = ROOT / "tools" / "binutils" / "mips64-elf-objdump"
    if not elf.is_file() or not objdump.is_file():
        return None
    section = f".overlay_{overlay:03d}"
    text = subprocess.run([str(objdump), "-h", str(elf)],
                          capture_output=True, text=True).stdout
    built = None
    for line in text.splitlines():
        tok = line.split()
        if len(tok) >= 3 and tok[1] == section:
            built = int(tok[2], 16)
            break
    if built is None:
        return None
    atlas = json.loads(MANIFEST.read_text())
    for module in atlas["modules"]:
        if module["overlay"] == overlay:
            return built - int(module["rom"]["size"], 16)
    return None


def rom_diff_offsets() -> list[int]:
    a = ROM.read_bytes()
    b = BASEROM.read_bytes()
    if len(a) != len(b):
        return [-1]
    return [i for i, (x, y) in enumerate(zip(a, b)) if x != y]


def run_trial(item: pb.QueueItem, rng: Optional[tuple[int, int]], jobs: int) -> Trial:
    t = Trial(item.func, item.rel_c_file, item.overlay, rng[0] if rng else None, rng[1] if rng else None)
    start = time.monotonic()
    original = splice(item)
    if original is None:
        t.klass, t.error = "unknown", "could not locate the NON_MATCHING block"
        return t
    surface = LINK_SYMS.read_text()
    rebinds = REBINDS.read_text() if REBINDS.is_file() else None
    source = trial_source(item)
    manifest_original = MANIFEST.read_text() if item.overlay is not None else None
    yaml_original = YAML.read_text() if item.overlay is not None else None
    manifest_trial_changed = False
    yaml_trial_changed = False
    surface_log = ""
    link_log = None
    try:
        if item.overlay is not None:
            trial_yaml = subprocess.run(
                [sys.executable, str(ATLAS_TOOL), "--trial-projection",
                 "--trial-source", source, "--trial-function", item.func],
                cwd=ROOT,
                capture_output=True,
                text=True,
                timeout=900,
            )
            if trial_yaml.returncode != 0:
                t.klass = "build-error"
                t.cause = "ownership-yaml"
                t.error = (trial_yaml.stdout + trial_yaml.stderr)[-1500:]
                return t
            manifest_trial_changed = MANIFEST.read_text() != manifest_original
            yaml_trial_changed = YAML.read_text() != yaml_original
        ok, log = build(jobs, source=source, function=item.func)
        if item.overlay is not None:
            # Compile is done either way; regenerate the surface against the
            # objects that now exist and relink. A candidate that already
            # linked is unaffected when the surface does not change.
            _sok, surface_log = synthesize_surface()
            t.aligned_sites, t.shifted_sites = alignment_counts(item,
                                                                surface_log)
            ok, log2 = build(jobs, source=source, function=item.func)
            link_log = log2
            log = log + log2
        if not ok:
            t.klass, t.cause = classify_failure(log, item, surface_log,
                                                link_log)
            t.error = log[-1500:]
        else:
            diffs = rom_diff_offsets()
            if diffs == [-1]:
                # A skipped normalization can leave a section long enough to
                # move the image's own size; that is the size report, not a
                # separate failure.
                marker = MARKER_RE.findall(log)
                if marker:
                    t.klass, t.cause = "text-size-differs", marker[0][0]
                    t.error = "ROM size differs"
                else:
                    t.klass, t.error, t.cause = ("build-error",
                                                 "ROM size differs", "rom-size")
            elif not diffs:
                t.klass = "exact"
            else:
                inside = [d for d in diffs if rng and rng[0] <= d < rng[1]]
                outside = [d for d in diffs if not (rng and rng[0] <= d < rng[1])]
                t.in_range_words = len({d & ~3 for d in inside})
                if inside and rng is not None:
                    t.diffs = explain_in_range(item, rng, tu_text_offset(item), inside)
                t.out_of_range_bytes = len(outside)
                t.first_in_range = inside[0] if inside else None
                t.first_out_of_range = outside[0] if outside else None
                t.klass = "text-differs" if inside else "text-exact"
                delta = module_size_delta(item.overlay)
                if delta:
                    t.klass, t.cause = "rom-size", f"module {delta:+d} bytes"
                marker = MARKER_RE.findall(log)
                if marker:
                    # A skipped normalization means the linked bytes are not a
                    # real build; the size report is the honest result.
                    t.klass, t.cause = "text-size-differs", marker[0][0]
    finally:
        item.c_file.write_text(original)
        # A carved projection makes splat write extra raw slices
        # (`*_data_rodata_<offset>.bin`) that only that projection names. They
        # are picked up by the Makefile's asset wildcard, so leaving one behind
        # would add bytes to a later canonical link: delete them, and their
        # objects, with the projection that asked for them.
        for extra in ROOT.glob("assets/**/*_data_rodata_*.bin"):
            extra.unlink()
            obj = ROOT / "build" / extra.relative_to(ROOT).with_suffix(".bin.o")
            if obj.is_file():
                obj.unlink()
        projection_changed = False
        if manifest_original is not None and MANIFEST.read_text() != manifest_original:
            MANIFEST.write_text(manifest_original)
            projection_changed = True
        if yaml_original is not None and YAML.read_text() != yaml_original:
            YAML.write_text(yaml_original)
            projection_changed = True
        if projection_changed:
            if yaml_trial_changed or manifest_trial_changed:
                # A temporary ownership projection changes splat's ignored
                # asset slices as well as the YAML.  Re-split the restored
                # canonical YAML before the next trial; mtime resolution is
                # too coarse to rely on the normal stamp prerequisite here.
                #
                # The manifest alone is enough to require it: `prune-asm`
                # deletes the .s files the projection made C-owned, and a
                # projection that moved only the manifest left five o001
                # candidates reporting `compile-error` on a file that no
                # longer existed -- and every candidate after them, because
                # the missing .s belongs to a TU the whole link needs.
                clean_env = dict(os.environ)
                clean_env.pop("PROMOTION_TRIAL", None)
                subprocess.run(
                    ["gmake", "extract"],
                    cwd=ROOT,
                    capture_output=True,
                    text=True,
                    timeout=900,
                    env=clean_env,
                )
        if LINK_SYMS.read_text() != surface:
            LINK_SYMS.write_text(surface)
        if rebinds is not None and REBINDS.read_text() != rebinds:
            REBINDS.write_text(rebinds)
        t.seconds = time.monotonic() - start
    return t


def main(argv: list[str]) -> int:
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("--overlay", type=int)
    p.add_argument("--function")
    p.add_argument("--limit", type=int)
    p.add_argument("--resume", action="store_true")
    p.add_argument("--overlays-only", action="store_true")
    p.add_argument("--jobs", type=int, default=6)
    args = p.parse_args(argv)

    if not BASEROM.is_file():
        print("baserom missing", file=sys.stderr)
        return 2
    queue = pb.discover_queue()
    if args.overlay is not None:
        queue = [q for q in queue if q.overlay == args.overlay]
    if args.overlays_only:
        queue = [q for q in queue if q.overlay is not None]
    if args.function:
        queue = [q for q in queue if q.func == args.function]
    results: list[Trial] = []
    if args.resume and OUT_JSON.is_file():
        known = {f.name for f in dataclasses.fields(Trial)}
        for row in json.loads(OUT_JSON.read_text()).get("results", []):
            results.append(Trial(**{k: v for k, v in row.items() if k in known}))
        done = {r.func for r in results if r.klass != "build-error"}
        queue = [q for q in queue if q.func not in done]
    if args.limit:
        queue = queue[: args.limit]

    ov = overlay_ranges()
    res = resident_ranges()
    print(f"trialling {len(queue)} candidate(s)")
    for i, item in enumerate(queue, 1):
        rng = ov.get(item.source or "") if item.overlay is not None else res.get(item.func)
        if rng is None and item.overlay is not None:
            # atlas rows are keyed by the TU's source stem; fall back to any row
            # whose stem ends with the function's own file
            stem = str(item.c_file.relative_to(ROOT / "src")).removesuffix(".c")
            rng = ov.get(stem)
        t = run_trial(item, rng, args.jobs)
        results.append(t)
        align = f"aln={t.shifted_sites}/{t.aligned_sites} " if t.aligned_sites else ""
        print(f"[{i}/{len(queue)}] {t.func:34} {t.klass:16} in={t.in_range_words:<4} out={t.out_of_range_bytes:<5} {align}{t.cause or '':40} ({t.seconds:.0f}s)", flush=True)
        write(results)
    write(results)
    wait_for_load("promotion cleanup build")
    subprocess.run(["gmake", f"-j{args.jobs}", ROM_TARGET], cwd=ROOT, capture_output=True)
    return 0


def write(results: list[Trial]) -> None:
    # One row per function, last write wins: a --resume run re-trials the
    # errored rows and must replace them, not append a second verdict.
    keyed = {}
    for r in results:
        keyed[r.func] = r
    results = list(keyed.values())
    OUT_JSON.parent.mkdir(parents=True, exist_ok=True)
    OUT_JSON.write_text(json.dumps({"generated_by": "tools/promotion_trial.py",
                                    "results": [dataclasses.asdict(r) for r in results]}, indent=2) + "\n")
    lines = [f"{'function':34} {'class':16} {'in':>4} {'out':>6} {'ov':>4} {'shft/aln':>8}  cause"]
    for r in results:
        align = f"{r.shifted_sites}/{r.aligned_sites}" if r.aligned_sites else ""
        lines.append(f"{r.func:34} {r.klass:16} {r.in_range_words:>4} {r.out_of_range_bytes:>6} {str(r.overlay):>4} {align:>8}  {r.cause or ''}")
    counts: dict[str, int] = {}
    for r in results:
        counts[r.klass] = counts.get(r.klass, 0) + 1
    causes: dict[str, int] = {}
    for r in results:
        if r.cause:
            key = re.sub(r" \(.*", "", r.cause)
            causes[key] = causes.get(key, 0) + 1
    lines.append("")
    lines.append(" ".join(f"{k}={v}" for k, v in sorted(counts.items())))
    lines.append(" ".join(f"{k}={v}" for k, v in sorted(causes.items())))
    OUT_TXT.write_text("\n".join(lines) + "\n")


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
