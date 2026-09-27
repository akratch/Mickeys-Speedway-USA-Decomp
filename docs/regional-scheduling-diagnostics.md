# Regional scheduling diagnostics

`tools/regional_schedule.py` compares operation counts and possible execution
paths in two owned MIPS function ranges. It reads existing ELF objects and never
compiles, changes source, authenticates relocation bindings, or awards a match.
The caller must first authenticate the objects, function boundaries and target
ROM ownership through the ordinary matching workflow.

```sh
python3 tools/regional_schedule.py SYMBOL \
  --target-object build/wb/SYMBOL.target.o \
  --candidate-object build_non_matching/src/overlays/oNNN/function.c.o \
  --target-window 0x100:0x200 --candidate-window 0x104:0x208 \
  > build/regional-report.json
```

Both windows use function-relative byte offsets with an exclusive end. Omitting
windows selects the full declared ELF function extent. The tool requires a
unique, positive, aligned extent, but ELF symbol sizes can include alignment
padding. The caller must authenticate executable boundaries and select windows
that exclude padding when appropriate; this tool does not infer those boundaries.
The `owned_size` field reports the declared extent, not independently proved
executable bytes. Object hashes and the selected symbol are included in the JSON report. Reports
are private workbench evidence and must remain ignored.

## What the report measures

The report includes declared function size and initial frame allocation, regional
opcode/family counts, load/store widths, multiply sites, delay-slot execution
conditions, branch-likely alternatives, and the existing `align_symbol`
insertion/deletion counts. Alignment offsets are relative to each selected
window. The other offsets remain function-relative.

Static counts describe emitted instructions, including instructions that might
not execute. Memory widths describe instructions, not guaranteed completed
memory transactions: partial loads/stores and conditional stores retain their
architectural qualifications. Alignment gaps likewise describe the chosen edit
script, not missing semantic operations. Enlarge the region before interpreting
a gap: the operation may already occur elsewhere after scheduling.

The execution report enumerates acyclic traversals from the selected entry.
Ordinary branch delay slots execute on both outcomes. Branch-likely delay slots
execute only on the taken edge. Calls include their delay slots and are assumed
to return; caller register knowledge is discarded across the call. Returns
include their declared delay slot. A `jalr` that discards its link through the
zero register is an unresolved indirect transfer, not an assumed returning call.
FPU data registers and FPU control registers use separate `fN` and `fcrN`
namespaces. Only relocation records targeting `.text` enter the report.
Local relocated branches require a defined
in-section label; external or unresolved branch targets do not inherit their
placeholder instruction's apparent destination.

Loops, indirect jumps, unsupported execution semantics, control transfers inside
a delay slot, missing owned delay slots, and path/step limits make traversal
coverage incomplete. Starting a window inside a delay slot also requires the
missing branch context. In these cases the report emits no finite multiply-count
range. It does not silently count one loop trip as a complete execution.

`--max-paths` and `--max-steps` bound the diagnostic. All branch outcomes are
considered without solving predicates, so complete ranges still overapproximate
feasible paths. Counts concern one selected-entry traversal, assuming calls
return and no exception interrupts execution. Windows do not establish
corresponding entry states or equal loop trip counts across the two objects.

## Multiply distinctions

A branch-likely alternative is reported only when its taken delay-slot multiply
and immediate not-taken fallthrough multiply are identical instructions, the
branch has both outcomes, and its known taken destination skips the fallthrough
site. Both sites then consume the same incoming register values at that branch
fork. Exactly one executes on each outgoing edge of that branch evaluation.
This is an edge-local observation, not a claim that only one multiply executes
in the whole loop or that a differently placed target operation is equivalent.

When both region traversals are complete, the report compares their multiply
count ranges. Disjoint ranges can show that every enumerated traversal on one
side contains more multiply operations than every traversal on the other.
Equal ranges do not show equal operands or equivalent programs. A smaller
window can omit a target operation at a loop destination and therefore create a
misleading cross-window interpretation; inspect the containing CFG and entry
conditions before drawing a source conclusion.

Operand comparison is deliberately narrower than operation counting. Local
immediate constructions and supported bit-preserving transfers provide constant
bits with their defining offsets. Relocated literals, memory contents, entry
register values, wide operands and unsupported computations remain unresolved.
Names and same-offset relocation correlations never become operand identities.
No floating-point arithmetic is evaluated or reassociated. Operand order is
preserved. Equal multiply counts with different known constant inputs are
reported separately from unresolved operands and from matching constant input
sequences. Even matching sequences are not an equivalence proof.

## API and validation

`analyze(words, start=..., end=..., relocations=..., control_targets=...)` reports
one owned word stream. `compare(target_words, candidate_words, ...)` adds the
regional alignment and side-by-side observations. `load_function(path, symbol)`
reads a bounded ELF function and independently defined local control targets.
Relocation maps use the existing ranking representation; they suppress constant
propagation through unresolved link fields. These APIs are diagnostic only and
must not be used as a relocation or promotion acceptance gate.

Synthetic tests cover ordinary and likely delay slots, returns, unresolved and
indirect control, cycles, path bounds, local PC16 ownership, multi-function
objects, literal provenance, call clobbers, wide operands, extra executed
multiplies, and equal counts with different operands. Normal function-sized
object, relocation, linked-range and full-ROM proofs remain mandatory.
