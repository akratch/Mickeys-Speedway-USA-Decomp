# Optimized-debug source identity control

This controlled experiment tests whether debug metadata can supply local names
without changing ordinary optimized compilation. It is a tooling diagnostic,
not a new compiler configuration for matching.

Use the project's pinned IDO 5.3 on this synthetic input:

```c
struct Packet { int count; float sample[4]; };
extern int observe(struct Packet *);
int stack_probe(int input, float scale) {
    struct Packet packet;
    int result;
    packet.count = input;
    packet.sample[0] = scale;
    packet.sample[1] = scale * 2.0f;
    packet.sample[2] = scale + 1.0f;
    packet.sample[3] = scale - 1.0f;
    result = observe(&packet);
    return result + packet.count;
}
```

Compile once with `-O2 -mips2 -32`, then with the same flags plus `-g3`.
Capture the frontend, optimizer and generator with the existing workbench
capture wrapper. Inspect the actual phase arguments and compare stock objects
with `decomp-workbench fidelity`, including relocations and symbols.

The normal frontend runs with `-Xg0`; its symbol-table input to UOPT has none
of the four local/parameter names above. With `-g3`, all four names occur in
that input, but text, relocation and symbol fidelity fail. Equal text size
therefore does not make the names a valid annotation of the stock object.

A second diagnostic changed only the frontend's actual `-Xg0` argument to
`-Xg3`, leaving UOPT and UGEN at `-g0`. It also exposes the four names and
fails executable fidelity. This localizes the changed lowering to the
frontend/debug-input route; simply restoring backend flags is insufficient.
The pass binaries and emitted instructions were not modified. This route
remains diagnostic and must never be substituted for the configured build.

A third control passed `-Wf,-Xg3` through the driver. That object's fidelity
passes, but the captured frontend arguments contain a later `-Xg0` and the
local names remain absent. This is an overridden option, not successful
source attribution. Always inspect the actual invocation.

The private captures preserve source, compiler identities, pass inputs,
arguments, objects and fidelity reports. These results establish one concrete
counterexample to importing debug names indiscriminately. They do not prove
all functions change under debug mode, nor do they establish correspondence
between an identifier and an optimizer or generator temporary.

A producer implementation still needs an explicit source/storage identity
carried through the relevant transformations. A generated UGEN function named
`frame_offset` offers a concrete place to observe frame-relative conversion,
but some callers add another displacement before emission. Its return alone
is neither the final memory operand nor the identity of a declared local.
Observe the actual emitting chain and authenticate ownership separately.
