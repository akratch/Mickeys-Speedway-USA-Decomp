# Exception and scheduler contracts

This audit covers the nine accepted `libultra/exceptasm` entries in
[VERIFIED_ASSEMBLY.md](VERIFIED_ASSEMBLY.md). It uses Mickey's retained linked
bytes, its ROM and its current C callers/headers. A read-only comparison of the
nine recorded entry extents, 2,476 bytes, is exact against the same ROM ranges.
Their handwritten classification and matching credit are unchanged. This is a
target contract audit, not a host scheduler implementation or a complete native
readiness claim.

## Entries and continuations

| Entry | Authenticated role and boundary |
|---|---|
| `__osExceptionPreamble` | A 16-byte vector trampoline, copied to four CPU vector locations by [osInitialize](../src/libultra/initialize.c#L63). It transfers to the handler using kernel scratch registers; it is not an ordinary C call. |
| `__osException` | Saves the interrupted thread, decodes exception/interrupt state, delivers events and chooses a context to dispatch. It has no ordinary caller return or stack frame. |
| `send_mesg` | Private event delivery: its input is a byte offset into the event-state table, not an `OSMesgQueue *`. It keeps its return continuation in `s2` and does not preserve incoming `s2`; the enclosing exception has already saved it. |
| `handle_CpU` | Private exception continuation requiring the handler's thread pointer and Cause value. Coprocessor-1 unusable enables CU1 in the saved status, sets the thread's FP flag and retries the same saved PC through dispatch. Other coprocessor numbers take the fault path. |
| `__osEnqueueAndYield` | The C-facing `void(OSThread **)` accepts a queue-head cell or null. It saves a voluntary suspension and dispatches; its C caller resumes at its saved return address only when that thread is selected again. Null omits enqueue. |
| `__osEnqueueThread` | Leaf `void(OSThread **, OSThread *)`: inserts a thread by descending signed priority and records its queue-head cell. It does not mask interrupts or change thread state. |
| `__osPopThread` | Leaf `OSThread *(OSThread **)`: removes and returns the head, without clearing its links or queue field. It has no empty-queue check. |
| `__osDispatchThread` | Pops the run queue, installs the running pointer/state, restores a context and returns from exception to its saved PC. It replaces the stack and execution state; it does not return through its C caller's current return address. |
| `__osCleanupThread` | Installed as a new thread's saved return address. Calls the actual `osDestroyThread(NULL)` and relies on current-thread destruction dispatching without return. |

The ordinary queue declarations are in
[osint.h](../include/PRinternal/osint.h#L31). They do not turn vector entries or
interior continuations into independently callable C functions. The kernel
scratch registers, private continuation registers, interrupt state and context
memory are part of this family boundary.

## Thread image and FP modes

All offsets below are from the target thread base. Target pointers and status
words are 32 bits; saved GPR slots and emitted FP transfers are 64 bits. The
image needs 8-byte alignment for those transfers. This is a big-endian target
memory image; its raw register bits and field offsets are not a host C struct
or a promise about host pointer size.

| Interval | Actual use |
|---|---|
| `0x00..0x20` | Queue/active-list links, signed priority, state/flags, ID, FP-used flag and profiling pointer. The [shared prefix](../include/PR/os_internal.h#L20) agrees. |
| `0x20..0x108` | Saved integer registers: `at`, value/argument/temporary/saved registers, `gp`, `sp`, frame pointer and return address. Kernel scratch registers are excluded. |
| `0x108..0x118` | LO and HI. The exception path saves them; voluntary yield does not promise to preserve caller-saved computation state. |
| `0x118..0x130` | Six 32-bit fields: status, saved PC, Cause, BadVAddr, RCP mask and FCSR. BadVAddr is captured on the fault path. Cause/BadVAddr are diagnostic fields, not restored CPU registers. |
| `0x130..0x230` | Thirty-two 8-byte FP slots, accessed when the thread's FP-used flag is nonzero. Full exception save/dispatch uses all slots; voluntary yield saves `f20..f31` and FCSR. |

An exception saves the interrupted general registers before using them as
kernel scratch. Its early temporary saves use a shared scratch area, not the
interrupted stack; the observed scratch access end is `0x120`. Nested use of
that area is not safe merely because a caller has another C stack.

A voluntary yield preserves the callee-saved integer registers, `gp`, stack,
frame pointer and return address, then saves that return address as the resume
PC. Caller-saved integer/FP slots can remain from an earlier save. Dispatch
restores the selected image, including status/PC and, when enabled, FCSR/FP
slots. This is a suspension/resumption protocol rather than a full snapshot at
every C call.

[osCreateThread](../src/libultra/createthread.c#L45) initializes selected fields,
not every saved register. It sign-extends the 32-bit argument/stack/cleanup
addresses into GPR slots, reserves 16 bytes below the supplied stack top, starts
stopped with FP-used zero and CU1 clear, and initializes FCSR to the source's
flush/invalid-enable/round-nearest setting. The caller supplies storage and
stack lifetime; initialization does not allocate or zero the whole image.

The saved status sets FR for ordinary IDs, but clears it for negative IDs.
This second path is real: [the audio manager creates ID -4](../src/main/audiomgr.c#L182).
The manufacturer-defined FR1 mapping and the documented FR0 limit are set out
below. The ordinary header's SDK version default does not establish one uniform
SDK revision or FP mode for this family.

### Manufacturer-defined modes and the FR0 limit

NEC's primary VR4300 manual, seventh edition (August 2000), defines FR1 as 32
64-bit registers and FR0 as 32 word registers forming 16 doubleword pairs
([sections 7.2.1–7.2.2, pp. 208–210](https://datasheets.chipdb.org/NEC/Vr-Series/Vr43xx/U10504EJ7V0UMJ1.pdf#page=208)).
FR0 even-register doubleword transfers pair the even low word with the adjacent
odd high word. Odd-register LDC1/SDC1 is undefined; the load pseudo-code leaves
an undefined result and the store pseudo-code supplies undefined data
([LDC1, pp. 590–591](https://datasheets.chipdb.org/NEC/Vr-Series/Vr43xx/U10504EJ7V0UMJ1.pdf#page=590),
[SDC1, pp. 603–604](https://datasheets.chipdb.org/NEC/Vr-Series/Vr43xx/U10504EJ7V0UMJ1.pdf#page=603)).
The manufacturer publication is hosted by a public mirror. It provides no
odd-register aliasing, ignore or deterministic trap rule to adopt here.

Applied to Mickey's retained provider, the mode/flag cases are:

| State at the transfer block | Interpretation supported by this audit |
|---|---|
| FP-used zero | The provider skips the FP transfer block. |
| FP-used nonzero, FR1 | Slot `n` at `0x130 + 8*n` carries register `n`'s raw 64 bits. All 32 emitted transfers have a defined register selection. |
| FP-used nonzero, FR0 | Even selections have paired-word meaning; the odd selections prevent claiming a complete defined 32-slot round trip. The image cannot be relabelled as duplicated pairs or harmless spare slots. |

FCR31 also carries compare, rounding, cause, enable, flag and flush state
([section 7.2.4, pp. 211–215](https://datasheets.chipdb.org/NEC/Vr-Series/Vr43xx/U10504EJ7V0UMJ1.pdf#page=211)).
The future provider contract therefore includes mode, FP-used state, raw
register bits, FCSR and the save/resume boundary. Numeric host doubles alone
do not establish preservation. No emulator behavior or simulated execution is
used to fill the undefined case.

The negative audio argument is an explicit mode selector: creation stores ID
4 while clearing FR; priority is a separate argument. Its rationale beyond
that source-proved effect is not documented here. A no-FP precondition is not
established for the audio lifetime. The real
[audio loop](../src/main/audiomgr.c#L187) processes a frame through
[`n_alAudioFrame`](../src/main/audiomgr.c#L356), then receives its completion
message with blocking enabled. The synthesizer's conditional client-update
path calls [_n_timeToSamplesNoRound](../src/libultra/n_synthesizer.c#L279),
which uses CPU FP; the existing linked provider confirms that direct call and
FP instructions. Whether the client path runs and the completion queue is
empty are runtime conditions, not observations from this static audit.

The inspected kernel preserves the current FR during saving, and installs the
selected saved status before restoring FP. Its coprocessor-unusable path sets
CU1/FP-used without normalizing FR. If audio remains in FR0 at its first FP
use, this lazy path reaches the odd-register restore stream before retrying
the instruction; a later blocking receive is not required. This is a
conditional source derivation, not a hardware observation. It does not prove
that every audio callback preserves FR, or that a particular undefined transfer
executes. A complete FR0 codec needs authoritative implementation-specific evidence or
a proved source precondition excluding that case. The primary manual alone
cannot supply either, so this audit stops at that boundary.

### Existing C views are incomplete

The public `OSThread` contains a `0x20`-byte prefix and `context[0x18C]`, a
`0x1AC`-byte target view. The creator's local sixteen-FP-pair view reaches
`0x1B0`; its accessed initialization fields precede the FP payload. Neither is
a complete representation of the provider's conditional `0x230`-byte image.

There is real larger backing in some consumers: the two
[boot thread bases](../src/main/boot.c#L14) are `0x230` apart, and
[the scheduler](../include/game/sched_internal.h#L40) explicitly reserves
`threadAndPad[0x230]`. That does not prove every thread allocation.

The [VI manager](../src/libultra/vimgr.c#L18) owns a short `OSThread` followed
by its stack. Existing compiler debug anchors place the thread at BSS offset
zero and the stack at `0x1B0`; a `0x230` FP-active image reaches into that
adjacent region. A blanket no-FP lifetime is not established: its main loop
calls [__osViSwapContext](../src/libultra/viswapcontext.c#L38), whose conditional
Y-scale update uses FP. VI creation passes ID zero, so its initial mode is FR1;
the FR0 undefined case cannot dismiss this capacity question. Whether the FP
path executes, the complete backing and simultaneous stack live intervals, and
a genuine source view must be reconciled before a repair. This audit proves a capacity/view gap, not live corruption or a lawful
alias. Enlarging the shared type would change an actual owned BSS layout and
is not authorized by the byte-exact creator alone.

## Queues, events and lifetime

`__osEnqueueThread` walks descending signed priorities and inserts after all
existing equal-priority nodes: equal-priority enqueue is FIFO. It reads each
node's first eight bytes, writes the inserted node's next/queue fields and
updates one predecessor link or the four-byte head cell. Its machine scratch
set is `at` and `t6..t9`; pop writes `v0` and `t9`. Neither leaf changes FP or
privileged state, saves a frame, or provides synchronization. The
[tail sentinel](../src/libultra/thread.c#L9) has only a null next pointer and
priority -1. Valid nonnegative priorities and a sentinel-terminated acyclic
queue are preconditions; the primitive does not validate them.

`__osPopThread` requires a real head when the consumer needs a runnable thread.
Dispatching an empty/sentinel-only queue is not a supported idle mechanism.
[Boot](../src/main/boot.c#L35) supplies an idle-priority thread that eventually
loops forever. The inspected normal setup supports that invariant; arbitrary
queue corruption or removal of every runnable thread is not covered.

The queue primitives perform no interrupt masking. The ordinary
[start](../src/libultra/startthread.c), [stop](../src/libultra/stopthread.c),
[yield](../src/libultra/yieldthread.c),
[priority](../src/libultra/setthreadpri.c) and
[message](../src/libultra/recvmesg.c) callers serialize mutations with
`__osDisableInt`/`__osRestoreInt`. A blocked caller retains its mask token in
its own suspended C execution and restores it after resumption. This is a
single serialized kernel-state assumption, not an established host mutex or
thread-local interpretation of the globals.

The exception's private event helper uses the
[registered event table](../src/libultra/seteventmesg.c#L13), 16 entries of two
target words. Its actual callers supply aligned byte offsets within that
span. It reads a registered queue/message pair; null registration or a full
queue silently drops delivery. Otherwise it appends one four-byte opaque
message value to the circular buffer, increments the count, and moves at most
one blocked receiver to the run queue. It does not deep-copy a resource or
transfer buffer ownership.

The [queue layout](../include/PR/os_message.h#L22) is 24 target bytes. Valid
positive capacity, a `capacity * 4` writable message buffer, bounded count and
first index, and a nonoverflowing index sum are caller preconditions. Retail
creation/delivery does not provide general validation. Queue storage, message
buffer and registrations must outlive all interrupt delivery and blocked
users; their ownership remains with callers.

The event helper does not write the awakened receiver's state field before
enqueueing it. Dispatch writes RUNNING when selecting it. Queue membership and
the state field are therefore not interchangeable at every intermediate point.
After normal interrupt processing, a higher-priority ready thread preempts;
otherwise the interrupted thread is put at the front. Equal-priority interrupt
return favors the interrupted thread, unlike explicit FIFO yield. No generic
equal-priority timer timeslice follows from these routines.

## Interrupt state, hooks and cleanup

The vector handler clears live IE/EXL before using shared kernel state. It
saves status/PC/Cause and filters pending CPU/RCP interrupts, acknowledges the
inspected devices and delivers the corresponding registered events. Dispatch
combines saved per-thread interrupt requests with the global mask. The RCP
mask is a six-bit logical field translated through a lookup before the MI
set/clear register is written, not a raw register image to copy back.

Break marks the thread and sends its event before redispatch. Other faults
record the faulted thread, stop it, save BadVAddr and send the fault event.
The coprocessor-unusable continuation only lazily enables coprocessor 1 and
retries; it does not initialize a new FP register payload. Normal handler entry
requires a valid running-thread pointer. This audit does not prove arbitrary
pre-scheduler exception safety or replace the separate counter/cache/TLB and
hardware-event contracts.

Two optional hardware-hook paths load an installed callback and replacement
stack. Their initial pointer/stack cells are zero, but that does not prove
runtime unreachability. The PI hook's nonzero return branches into an interior
continuation of `__osEnqueueAndYield`, which subsequently consumes a thread
pointer and optional queue in argument registers. A plain C return value is
not a complete contract for that path. Registration writers, private live-ins
and callback-stack capacity need a bounded audit before using it as a host
interrupt interface.

Cleanup's actual call target is
[osDestroyThread](../src/libultra/destroythread.c#L10), with a null argument.
It selects the running thread, unlinks its active-list membership and dispatches
when destroying that thread. Dispatch returns to a selected context, not to
cleanup. If this invariant failed and destruction returned, execution would
cross one alignment word into the next provider,
[osCreateThread](../symbol_addrs.us.txt#L208). That is not a valid cleanup return
or a second entry to creation. No storage or stack is freed by these routines;
allocator ownership and safe reuse must be decided after the thread is no
longer running, queued, active or referenced by another subsystem.

## Next prerequisites

1. Reconcile the full conditional context image with actual thread/backing and
   stack ownership, starting with VI and the real FR-clearing audio caller.
   Prove an exact natural C view before changing shared declarations or BSS.
2. Close the FR0 odd-transfer case with authoritative implementation evidence
   or a proved excluding precondition. FR1 register selection is documented;
   FCSR behavior and the actual source lifetime still need preservation proof.
   No generic host floating environment follows from the register map.
3. Close the optional hook registration/private-continuation and stack-lifetime
   paths, or establish their absence in a bounded configuration with source
   evidence. Keep CPU/RCP acknowledgement and event ordering separate from
   choosing a future scheduling provider.

Those are concrete remaining contracts. The queue/continuation audit does not
close all native boundaries or select a portable scheduling replacement.
