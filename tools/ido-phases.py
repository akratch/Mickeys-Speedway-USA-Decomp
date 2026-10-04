#!/usr/bin/env python3
"""
ido-phases.py -- a drop-in replacement for tools/ido/cc that can drive the
individual IDO phases (cfe -> uopt -> ugen -> as1) with per-phase overrides.

Why: tools/ido/cc's driver cannot express two things Mickey's bytes need.
  1. Running `uopt` at -O1.  The driver only runs uopt at -O2 and above, so
     "optimised by uopt at -O1" is unreachable through it.
  2. Per-phase -O levels generally (e.g. uopt -O3 with everything else -O2);
     mixed levels have phase-specific precedence. UOPT suppresses -O3
     whenever -O2 is also present, regardless of option order.
(`-fp32regs` does NOT need this script -- plain `cc -Wc,-mips3 -Wc,-fp32regs`
works, because -W ISA options are appended after the driver's own.)

How: it runs the real `cc -v` once to obtain the driver's own, exact phase
command lines, then re-executes those command lines against its own temporary
files with the requested overrides applied.  Nothing about the flag
translation is reimplemented here, so it cannot drift from the driver.

Usage: exactly like tools/ido/cc, plus any number of:
    -Xphase,<phase>,<option>...   phase in {cfe,uopt,ugen,as1}
    -Xphase,uopt,+               force a uopt stage to exist even if the
                                 driver chose not to run one
Example (src/libultra/setglobalintmask.c):
    ido-phases.py -c <cflags> -O1 -mips2 -32 \
        -Xphase,uopt,+ -Xphase,uopt,-O1 -o out.o in.c
"""
import os
import re
import shlex
import subprocess
import sys
import tempfile

# tools/ido/ is gitignored (proprietary toolchain binaries), so this script
# lives one level up, beside it rather than in it.
IDO = os.environ.get(
    "IDO_DIR", os.path.join(os.path.dirname(os.path.abspath(__file__)), "ido")
)
PHASES = ("cfe", "uopt", "ugen", "as1")

argv = sys.argv[1:]
overrides = {p: [] for p in PHASES}
force = set()
passthru = []
for a in argv:
    if a.startswith("-Xphase,"):
        _, phase, opt = a.split(",", 2)
        if phase not in PHASES:
            sys.exit("ido-phases: unknown phase %r" % phase)
        if opt == "+":
            force.add(phase)
        else:
            overrides[phase].append(opt)
    else:
        passthru.append(a)

# ---- 1. ask the real driver what it would do -------------------------------
with tempfile.TemporaryDirectory(prefix="idophases") as td:
    probe_o = os.path.join(td, "probe.o")
    probe = list(passthru)
    if "-o" in probe:
        probe[probe.index("-o") + 1] = probe_o
    else:
        probe += ["-o", probe_o]
    r = subprocess.run(
        [os.path.join(IDO, "cc"), "-v"] + probe,
        capture_output=True,
        text=True,
    )
    if r.returncode != 0:
        sys.stderr.write(r.stdout + r.stderr)
        sys.exit(r.returncode)
    lines = {}
    for line in (r.stdout + r.stderr).splitlines():
        m = re.match(r"\s*/usr/lib/(cfe|uopt|ugen|as1)\s+(.*)", line)
        if m:
            lines[m.group(1)] = shlex.split(m.group(2))
    for p in ("cfe", "ugen", "as1"):
        if p not in lines:
            sys.exit("ido-phases: driver did not run %s; cannot proceed" % p)

# ---- 2. rebuild the phase command lines against our own temp files ---------
def drop_v(t):
    return [x for x in t if x != "-v"]


VALUED = ("-G", "-o", "-t", "-temp", "-loopunroll")


def positionals(tokens):
    """Indices of true operands: not options, not an option's value."""
    out = []
    skip = False
    for i, x in enumerate(tokens):
        if skip:
            skip = False
            continue
        if x in VALUED:
            skip = True
            continue
        if x.startswith("-"):
            continue
        out.append(i)
    return out

T = tempfile.mkdtemp(prefix="idophases")
try:
    cfe = drop_v(lines["cfe"])
    # cfe's stdout was redirected by the driver; shlex.split keeps ">" and the
    # path as separate tokens, so strip them and redirect ourselves.
    if ">" in cfe:
        i = cfe.index(">")
        cfe = cfe[:i] + cfe[i + 2 :]
    # the shared symbol table (-XS for cfe, -t for the rest)
    sym = os.path.join(T, "sym")
    cfe = [("-XS" + sym) if x.startswith("-XS") else x for x in cfe]
    B = os.path.join(T, "cfe.B")

    ugen = drop_v(lines["ugen"])
    as1 = drop_v(lines["as1"])

    def retarget(tokens, mapping):
        out = []
        for x in tokens:
            out.append(mapping.get(x, x))
        return out

    # positional operands: ugen "<in> -o <out> -t <sym> -temp <tmp>"
    ugen_in = ugen[positionals(ugen)[0]]
    ugen_out = ugen[ugen.index("-o") + 1]
    as1_in = as1[positionals(as1)[0]]
    G = os.path.join(T, "ugen.G")

    have_uopt = "uopt" in lines
    if have_uopt:
        uopt = drop_v(lines["uopt"])
        pos = [uopt[i] for i in positionals(uopt)]
        uopt_in, uopt_out, uopt_tmp = pos[0], pos[1], pos[-1]
        O = os.path.join(T, "uopt.O")
        uopt = retarget(
            uopt,
            {uopt_in: B, uopt_out: O, uopt_tmp: os.path.join(T, "uo.tmp")},
        )
        uopt = [sym if x == uopt[uopt.index("-t") + 1] else x for x in uopt]
        ugen = retarget(ugen, {ugen_in: O})
    elif "uopt" in force:
        # Synthesise the stage the driver refused to run.  Shape is taken from
        # the driver's own uopt line in the -O2 case:
        #   uopt -G <n> <isa> -EB -g<n> -O<n> <in> <out> -t <sym> <tmp>
        pos = set(positionals(ugen))
        keep = []
        skip = False
        for i, x in enumerate(ugen):
            if skip:
                skip = False
                continue
            if x in ("-o", "-t", "-temp"):
                skip = True
                continue
            if x == "-G":
                keep += [x, ugen[i + 1]]
                skip = True
                continue
            if i in pos:
                continue
            keep.append(x)
        O = os.path.join(T, "uopt.O")
        uopt = (
            [os.path.join(IDO, "uopt")]
            + keep
            + [B, O, "-t", sym, os.path.join(T, "uo.tmp")]
        )
        ugen = retarget(ugen, {ugen_in: B})
        ugen = [O if x == B else x for x in ugen]
    else:
        uopt = None
        ugen = retarget(ugen, {ugen_in: B})

    ugen = retarget(ugen, {ugen_out: G})
    ugen = [sym if x == ugen[ugen.index("-t") + 1] else x for x in ugen]
    ugen[ugen.index("-temp") + 1] = os.path.join(T, "ug.tmp")
    as1 = retarget(as1, {as1_in: G})
    as1 = [sym if x == as1[as1.index("-t") + 1] else x for x in as1]
    # the driver wrote to its probe temp; send as1 to the caller's real -o
    real_o = passthru[passthru.index("-o") + 1] if "-o" in passthru else "a.out"
    as1[as1.index("-o") + 1] = real_o

    def bin_of(tokens, phase):
        t = list(tokens)
        if t[0].startswith("/usr/lib/") or t[0] == phase:
            t[0] = os.path.join(IDO, phase)
        elif not t[0].endswith(phase):
            t = [os.path.join(IDO, phase)] + t
        return t

    cfe = bin_of([os.path.join(IDO, "cfe")] + cfe, "cfe")
    ugen = bin_of([os.path.join(IDO, "ugen")] + ugen, "ugen")
    as1 = bin_of([os.path.join(IDO, "as1")] + as1, "as1")
    if uopt is not None and not uopt[0].endswith("uopt"):
        uopt = [os.path.join(IDO, "uopt")] + uopt

    # ---- 3. append requested phase options -------------------------------
    # Precedence is phase-specific: UOPT's -O2 vetoes -O3 in either order.
    # Preserve existing exact invocations; appending is not replacement.
    def run(tokens, phase, stdout=None):
        cmd = tokens + overrides[phase]
        if os.environ.get("IDO_PHASES_VERBOSE"):
            sys.stderr.write(" ".join(shlex.quote(x) for x in cmd) + "\n")
        rc = subprocess.run(cmd, stdout=stdout).returncode
        if rc != 0:
            sys.exit(rc)

    with open(B, "wb") as f:
        run(cfe, "cfe", stdout=f)
    if uopt is not None:
        run(uopt, "uopt")
    run(ugen, "ugen")
    run(as1, "as1")
finally:
    import shutil

    shutil.rmtree(T, ignore_errors=True)
