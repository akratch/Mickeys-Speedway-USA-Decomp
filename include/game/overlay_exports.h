#ifndef GAME_OVERLAY_EXPORTS_H
#define GAME_OVERLAY_EXPORTS_H

/*
 * The overlay exports resident code calls (docs/overlays.md section 5.1).
 *
 * Resident C calls each one by name. The build then renames every such
 * undefined reference to TrapDanglingJump in the compiled resident object
 * (tools/trap_names.py, driven by config/overlay_funcs_to_trap.us.txt), so the
 * ROM keeps its shipped `jal TrapDanglingJump` and the runtime linker's main
 * relocation table still routes each site to the function named here.
 * `gmake verify` proves that every call site names the export its table row
 * targets.
 *
 * A prototype is the overlay's own C definition with every overlay-private
 * or untyped pointer type spelled `void *` (a `void **`, or a byte pointer the
 * definition walks, included): the resident TUs cannot see those types, and a
 * pointer's spelling does not change the call.
 *
 * An old-style declaration, `T name();`, marks a call the shipped resident
 * code makes without a prototype in scope, because the definition's prototype
 * does not reproduce it. The tag says why:
 *
 *   [arity]   the resident call passes a different number of arguments from
 *             the number the definition takes;
 *   [types]   the resident passes an integer where the definition takes a
 *             pointer, or the reverse (IDO warning 709 or 712 under the
 *             prototype);
 *   [codegen] the prototype converts an argument the shipped call passes as
 *             it is (an int for the definition's f32 or narrow parameter).
 *
 * Such a disagreement is usually one side's reconstruction, and the tag stays
 * only with the evidence that the original made the call that way. One does:
 * func_overlay_029_F00010C4_187E374. The other 116 tagged declarations were
 * reconciled against the definitions' argument-register use, all
 * byte-identical (docs/overlays.md section 5.1).
 */

#include "PR/ultratypes.h"

/* overlay 1 */
void overlay1LoadBuildRecords(void);
void overlay1BuildObjectMappings(s32 unused);
void func_overlay_001_F0002B4C_184EF2C(s32 elapsed);
void func_overlay_001_F0003258_184F638(void);
void overlay1InitializeGaugeObjects(void);
void overlay1AssignRecordIndex(void *object, void *owner);
void overlay1TransitionState(void *obj, void *state, s32 updateRate);
void overlay1UpdateObjectPhysics(void *object, s32 updateRate);
void overlay1StartTimerCallbacks(void *object, s32 amount);
void overlay1CopyBytes(void *object, void *init);
void overlay1UpdateRangeFlags(void *object, s32 updateRate);
void overlay1InitMotion(void *object, void *init);
void overlay1ConsumeNearbyPending(void *object, s32 updateRate);
void overlay1InitRange(void *object, void *init);
void overlay1SearchNearby(void *object, s32 updateRate);

/* overlay 2 */
void func_overlay_002_F0001DF8_1858BF0(void *object, void *input);
void overlay2CopyColor(void *object, u8 *color);

/* overlay 4 */
void overlay4InitializeObjectMotion(void *object, void *config);
void overlay4UpdateObjectMotion(void *object, s32 updateRate);

/* overlay 6 */
void overlay6Empty(void);
void overlay6Noop(void *cameras, s32 cameraCount);

/* overlay 7 */
void func_overlay_007_F0000324_185C1AC(void *displayList, s32 elapsed);
void overlay7DispatchModes(void *first, void *second);
void overlay7CommitSelection(s32 selection);
void overlay7InitPool(void);

/* overlay 8 */
void *overlay8GetIndexed(void *object);
void func_overlay_008_F0000058_185DDB0(void *owner, s32 updateRate);
void func_overlay_008_F0000894_185E5EC(void *owner, void *state, s32 updateRate);
s32 func_overlay_008_F0000E88_185EBE0(void *peer, void *record);
void func_overlay_008_F00042A8_1862000(void *actor, void *owner, f32 update);

/* overlay 9 */
void func_overlay_009_F0000000_1866678(void *object, s32 steps);
void func_overlay_009_F00010B4_186772C(void *out, void *owner, f32 stepsFloat);

/* overlay 11 */
void overlay11Initialize(s32 argument);
void func_overlay_011_F0000150_1868998(void *displayList, void *matrix, void *vertices,
                                       s32 updateRate);
void overlay11ReleaseCurrentGroup(void);

/* overlay 12 */
void func_overlay_012_F00000C4_186D344(void);
void func_overlay_012_F00003A8_186D628(s32 updateRate);
void func_overlay_012_F0000910_186DB90(void *displayListPtr, void *matrixPtr,
                                       void *verticesPtr);

/* overlay 13 */
void overlay13ProcessActive(s32 arg);
void overlay13DrawActive(void *commands, void *mtx, void *vertices);

/* overlay 14 */
void func_overlay_014_F0000000_186F8D8(void);
void overlay14UpdateTransition(void *context, s32 step);

/* overlay 15 */
void *overlay15GetResource10(void);
void overlay15ReleaseResource10(void);
void overlay15InitStars(s32 count, s32 xRange, s32 yRange, s32 zRange, u32 startColor,
                        u32 endColor, s32 colorDivisor);
void overlay15UpdateMovingStars(f32 positionX, f32 positionY, f32 positionZ,
                                s32 updateRate);
void overlay15DrawRain(void *framebuffer, s32 width, s32 height, f32 projectionScale,
                       f32 intensity);

/* overlay 16 */
void overlay16InitializeBuffer(u8 *config);
void overlay16ApplyGradient(s32 *active, void *context, s32 phaseStep);

/* overlay 17 */
void *overlay17CreateChain(void *owner, s32 count, s32 textureId, s32 materialScale,
                           f32 x, f32 y, f32 z, f32 radius, u8 red, u8 green, u8 blue,
                           u8 alpha);
void overlay17ReleaseChain(void *resource);
void overlay17DrawStrip(void *commands, void *strip);

/* overlay 18 */
void overlay18Load(void);
void overlay18Reconfigure(s32 mode);

/* overlay 19 */
void overlay19Dispatch(void *object);

/* overlay 20 */
void overlay20ReleaseTree(void *node);
void overlay20ReleaseHandle(void *state);
void overlay20UpdateObjectResource(void *object, void *config);
void overlay20DrawResource(void *gfxP, void *object);
void overlay20ReleaseEntry(void *owner);
void overlay20MarkNested(void *owner);
void overlay20CreateEntry(void *owner, void *config);
void overlay20RemoveEntry(void *owner);
void overlay20AdvanceEntries(s32 amount);

/* overlay 21 */
void overlay21RegisterPlane(void *object, void *init);
void overlay21ApplyPriorities(s32 planeId, s32 entryCount, void *entries);

/* overlay 22 */
void func_overlay_022_F0000000_1878108(void *object, void *init);
void func_overlay_022_F00002B0_18783B8(void *object, s32 updateRate);
void func_overlay_022_F0000D30_1878E38(void *object, s32 flags);

/* overlay 23 */
void overlay23SpawnAttachments(void *object, void *animation);
void overlay23Init(void *object, void *init);
void overlay23Update(void *object, s32 updateRate);
void overlay23RenderEffect(void *object, void *displayList, void *renderArg0,
                           void *renderArg1);

/* overlay 24 */
void overlay24Init(void *object, void *init);
void overlay24Update(void *object, s32 updateRate);
void overlay24RenderState(void *commands, void *mtx, void *vertices, void *object);

/* overlay 25 */
void overlay25InitializeEffect(void *object, void *init);
void overlay25UpdateEffect(void *object, s32 updateRate);

/* overlay 26 */
void func_overlay_026_F0000000_187A3F8(void *object, void *config);
void func_overlay_026_F00001A0_187A598(void *object, s32 updateRate);
void func_overlay_026_F0000D24_187B11C(void *object, s32 mode);
void func_overlay_026_F0001158_187B550(void *dl, void *drawContext, void *context);

/* overlay 27 */
void overlay27Init(void *object, void *init);
void func_overlay_027_F0000064_187BA3C(void *object, s32 updateRate);
void func_overlay_027_F0000624_187BFFC(void *commands, void *mtx, void *vertices,
                                       void *object);
void overlay27UpdateCoordinates(s32 amount);
s32 overlay27CanUse(void *object);

/* overlay 28 */
void overlay28InitializeWork(void *owner, void *source);
void overlay28UpdateWork(void *owner, s32 updateRate);
void func_overlay_028_F00004D8_187CDA8(void *dl, void *matrixArg, void *owner);

/* overlay 29 */
void func_overlay_029_F000042C_187D6DC(void *object, void *init);
void func_overlay_029_F00005C4_187D874(void *object, s32 updateRate);
/* The original's own mismatch. anim.c's func_80056274 passes a third
 * argument, firstTarget, which uopt keeps in a2 across sqrtf for this call;
 * dropping it changes func_80056274. The definition reads a0 and a1 only, and
 * a third formal would home a2 and grow it. The other eight calls pass two. */
void func_overlay_029_F00010C4_187E374(); /* [arity] */
void overlay29DrawGroups(void *dl, void *drawContext, void *context);

/* overlay 31 */
void overlay31InitializeBuffers(s32 maxTriangle, s32 maxRectangle, s32 maxSprite,
                                s32 maxLine, s32 maxPoint, s32 maxExtra, s32 maxEffects);

/* overlay 33 */
void overlay33InitializeBuffers(void);
void overlay33PresentAndSwap(void);
void overlay33CallB(void);

/* overlay 34 */
void overlay34InitStorage(s32 count);
void overlay34ResetStorage(void);
void overlay34UpdateRecords(s32 updateRate);
void overlay34SortAndDraw(void *commands, void *vertices);

/* overlay 35 */
void func_overlay_035_F0000000_1881CE0(s32 modelId, s32 value1, s32 value2, s32 value3,
                                       s32 value4, s32 value5);

/* overlay 36 */
void overlay36InitObject(void *object, void *entry);
void overlay36FlushQueue(void);
void overlay36UpdateInteractiveEntity(void *entity, s32 elapsed);

/* overlay 37 */
void overlay37Init(void *object, void *init);
void overlay37Update(void *object, s32 ticks);
void overlay37RenderEffect(void *commands, void *renderContext, void *object);
s32 overlay37RecordActive(s32 index);

/* overlay 38 */
void func_overlay_038_F0000000_1885D10(void *object, void *descriptor);
void func_overlay_038_F0000154_1885E64(void *object, s32 ticks);
void func_overlay_038_F000047C_188618C(void *dList, void *context, void *object);

/* overlay 39 */
void overlay39Write(u8 *source, void *mq);
void overlay39Reset(void *mq);
void overlay39Read(u8 *destination);

/* overlay 40 */
void overlay40AddEntry(s32 id, s32 red, s32 green, s32 blue, s32 unused);
void overlay40RemoveEntry(s32 id);
void overlay40UpdateEntries(s32 amount);
void overlay40DrawEntries(void *displayList);
void overlay40SetValues(s32 value0, s32 value1, s32 value2, s32 value3, s32 value4);
void overlay40Interpolate(s32 amount);
void overlay40DrawTintRectangle(void *displayList);
void overlay40FadeRecords(s32 *enabled, void *context, s32 amount);

/* overlay 41 */
void func_overlay_041_F0000000_1887338(s32 amount);
void func_overlay_041_F0000124_188745C(s32 amount);
void func_overlay_041_F0000854_1887B8C(void *input, f32 amount, s32 updateRate,
                                       s32 argument);
void func_overlay_041_F0001650_1888988(void *object, s32 value1, s32 value3, s32 value5,
                                       s32 value7, f32 amount, s32 alternateColors);
void overlay41Ignore(void *object, s32 value1, s32 value2, s32 value3, s32 value4,
                     s32 value5, s32 value6);
void overlay41SpawnItems(s32 rootIndex, s32 count, s32 mode, s32 centerX, s32 centerY,
                         s32 radiusX, s32 radiusY, s32 centerZ, s32 radiusZ);
void func_overlay_041_F000195C_1888C94(s32 value2, s32 timer, s32 value4, s32 value6,
                                       s32 value8, s32 value9, s32 valueA);
void overlay41TickTransitions(s32 step);
void overlay41DrawItem(void *displayList, void *item);

/* overlay 42 */
void overlay42Init(void);
void overlay42DrawCapturedBuffer(void *callbackArgument);
void overlay42Present(void);

/* overlay 43 */
s32 func_overlay_043_F0000000_1889FD0(void *input);
void func_overlay_043_F0000194_188A164(void);
void func_overlay_043_F0000280_188A250(void *owner);
s32 func_overlay_043_F0000324_188A2F4(void *input, s32 useNodes, f32 updateRate);

/* overlay 45 */
void func_overlay_045_F0000764_188CBBC(void *displayList, void *matrix, void *arg2,
                                       void *arg3, s32 mode, s32 updateRate);

/* overlay 46 */
void overlay46InitializeState(void);
void func_overlay_046_F0000120_188E518(s32 amount);
void overlay46ReleaseState(void);
void overlay46InitState(void);
void overlay46UpdateTransition(s32 updateRate);
void overlay46Submit(void);

/* overlay 47 */
void func_overlay_047_F0000000_1890E18(void);
void func_overlay_047_F00009D0_18917E8(void);
void func_overlay_047_F0000B30_1891948(s32 updateRate);

/* overlay 48 */
void overlay48InitializeState(void);
void overlay48UpdateState(s32 updateRate);
void overlay48ReleaseAll(void);

/* overlay 49 */
void overlay49Initialize(void);
void overlay49Update(s32 updateRate);
void refractOutput(void);

/* overlay 50 */
void func_overlay_050_F0000000_1896970(void);
void func_overlay_050_F0000334_1896CA4(void *object, s32 updateRate);
void overlay50Cleanup(void);

/* overlay 51 */
void overlay51Initialize(void);
void func_overlay_051_F00000D0_18999D0(void *object, s32 updateRate);
void overlay51ReleaseState(void);

/* overlay 52 */
void func_overlay_052_F0000000_189A670(void);
void func_overlay_052_F000063C_189ACAC(s32 updateRate);
void overlay52Cleanup(void);

/* overlay 53 */
void overlay53Initialize(void);
void func_overlay_053_F0000240_189DBE8(s32 updateRate);
void overlay53ReleaseResources(void);

/* overlay 54 */
void func_overlay_054_F0000000_189ECA0(void);
void func_overlay_054_F00005AC_189F24C(s32 updateRate);
void overlay54ReleaseResources(void);

/* overlay 55 */
void overlay55Initialize(void);
void func_overlay_055_F000031C_18A1E34(s32 updateRate);
void overlay55ReleaseAll(void);

/* overlay 56 */
void overlay56SetMode(s32 mode);
void func_overlay_056_F00001A0_18A2F18(void *displayList, void *matrixCursor,
                                       s32 updateRate);
void overlay56UnpackColor(s32 index, s32 *red, s32 *green, s32 *blue);

/* overlay 57 */
void func_overlay_057_F0000000_18A3BF8(void);
void func_overlay_057_F0000954_18A454C(s32 updateRate);
void overlay57ReleaseAll(void);

/* overlay 58 */
void func_overlay_058_F0000000_18AF1E8(void);
void overlay58ReleaseResources(void);
void func_overlay_058_F00005FC_18AF7E4(s32 updateRate);

/* overlay 59 */
void overlay59ResetEntries(void);
void overlay59ReleaseAll(void);
void overlay59Advance(s32 steps);

/* overlay 60 */
void func_overlay_060_F0000000_18B9DD8(void);
void func_overlay_060_F0000334_18BA10C(s32 ticks);
void overlay60ReleaseResources(void);

/* overlay 62 */
void overlay62Initialize(void);
void overlay62Update(s32 updateRate);
void overlay62ReleaseAll(void);

/* overlay 63 */
void overlay63Initialize(void);
void overlay63UpdateEffects(s32 updateRate);
void overlay63Release(void);

/* overlay 66 */
s32 overlay66GetCurrent(void);
void func_overlay_066_F0000040_18C64A8(void *commands);

/* overlay 68 */
void overlay68CreatePrimary(void);
void overlay68CreateSecondary(void);
void overlay68ReleaseSecondary(void);
void overlay68CreatePayload(void *entry);
void overlay68AttachObject(void *object);
void overlay68UpdateTrail(s32 updateRate);
void overlay68PromoteSecondary(void);
void overlay68InitializeObject(void *object, void *init);
void overlay68UpdateAnimation(void *object, s32 updateRate);
void overlay68DrawSortedEntries(void *displayList, void *mtx, void *vertices, void *object);
void overlay68RebuildSecondaryEntry(s32 kind);
void overlay68ReleaseTertiary(void);
s32 overlay68CheckKind(s32 kind);

/* overlay 69 */
void overlay69Init(void *object, void *entry);
void overlay69UpdateAnchor(void *object, s32 updateContext);
void overlay69DrawSortedGeometry(void *commands, void *renderArg1, void *renderArg2,
                                 void *object);

/* overlay 70 */
void func_overlay_070_F0000000_18C91C8(void *object, void *input);
void func_overlay_070_F00000D8_18C92A0(void *object, s32 ticks);
void func_overlay_070_F0000384_18C954C(void *object);

/* overlay 71 */
void func_overlay_071_F0000000_18C9B20(void *object, void *input, s32 preserveState);
void func_overlay_071_F0000278_18C9D98(void *object);
void overlay71UpdateCoordinates(s32 amount);
void func_overlay_071_F0000870_18CA390(void *commands, void *context, void *object);

/* overlay 72 */
void overlay72Init(void *object, void *config);
void overlay72Update(void *object, s32 updateRate);

/* overlay 73 */
void func_overlay_073_F0000000_18CAAC0(void *object, void *header, s32 preserveState);
void func_overlay_073_F0000190_18CAC50(void *object, s32 updateRate);
void func_overlay_073_F0000D70_18CB830(void *commands, void *context, void *object);

/* overlay 74 */
void overlay74Init(void *object, void *init);
void overlay74Update(void *object, s32 amount);

/* overlay 75 */
void overlay75Init(void *object, void *init, s32 skip);
void overlay75UpdateMovingObject(void *object, s32 updateRate);
void overlay75MarkSlot(s32 slot);

/* overlay 76 */
void overlay76Register(void *object, void *entry);
void overlay76Update(void *object, s32 updateRate);
void overlay76TriggerRandom(void);

/* overlay 77 */
void overlay77Init(void *object, void *init, s32 preserveSequence);
void overlay77Update(void *object, s32 updateRate);
void overlay77EnsureSelection(void);
void overlay77RunCallback(void);

/* overlay 78 */
void overlay78Init(void *object, void *config);
void overlay78Update(void *object, s32 updateRate);

/* overlay 79 */
void func_overlay_079_F0000000_18CCFA0(void *object, void *config, s32 preserveState);
void func_overlay_079_F0000134_18CD0D4(void *object, s32 updateRate);
void overlay79SetLink(void *object, void *entry);
void func_overlay_079_F0001290_18CE230(void *object, s32 unused1);
void overlay79InitState(void *object, void *entry);
void overlay79UpdateTimers(void *object, s32 step);

/* overlay 80 */
void overlay80InitializeContact(void *object, void *init);
void overlay80UpdateContact(void *object, s32 ticks);

/* overlay 81 */
void overlay81Init(void *object, void *init, s32 unused);
void overlay81Update(void *object, s32 updateRate);
void overlay81InitState(void *object, void *init);
void overlay81CheckNearby(void *object, s32 unused);

/* overlay 82 */
void overlay82Init(void *object, void *entry);
void overlay82Update(void *object, s32 updateRate);

/* overlay 83 */
void overlay83BuildBatch(void *parent, void *source);
void overlay83DrawEntries(void *object, s32 updateRate);

/* overlay 84 */
void overlay84InitState(void *object, void *entry);
void overlay84InitializeAndUpdate(void *object, s32 arg);
void overlay84CopyPair(void *dst, void *src);

/* overlay 85 */
void overlay85Configure(void *state, void *config);
void overlay85Update(void *object, s32 step);

/* overlay 86 */
void overlay86Init(void *object, void *entry);
void func_overlay_086_F0000474_18D22AC(void *object, s32 steps);

/* overlay 87 */
void overlay87InitializeObject(void *object, void *config, s32 preserveState);
void func_overlay_087_F0000128_18D3090(void *object, s32 updateRate);
void overlay87ReleaseCurrent(void);

/* overlay 88 */
void overlay88Init(void *object, void *entry);
void overlay88UpdateAnchor(void *object, s32 updateContext);
/* overlay69DrawSortedGeometry's source, compiled again under this name. */
void overlay88DrawSortedGeometry(void *commands, void *renderArg1, void *renderArg2,
                                 void *object);

/* overlay 89 */
void overlay89InitializeEffect(void *object, void *init);
void overlay89UpdateStateAndParticles(void *object, s32 updateRate);

/* overlay 90 */
void overlay90Initialize(void *owner, void *config);
void func_overlay_090_F00000FC_18D4BF4(void *owner, s32 updateRate);

/* overlay 91 */
void overlay91Init(void *object, void *entry);
void overlay91UpdateTimeline(void *object, s32 elapsed);
void overlay91Render(void *displayList, void *renderContext, void *vertices, void *object);

/* overlay 92 */
void overlay92Init(void *object, void *init);
void func_overlay_092_F0000308_18D6228(void *owner, void *config, void *state,
                                       s32 update);

/* overlay 93 */
void overlay93Init(void *object, void *config);
void overlay93Update(void *object, s32 updateRate);

/* overlay 94 */
void overlay94InitializeController(void *object, void *init);
void overlay94UpdateController(void *object, s32 updateRate);

/* overlay 95 */
void overlay95NoOp(void *object, void *entry);
void overlay95Update(void *object, s32 updateRate);

/* overlay 96 */
void overlay96Unregister(void *volume);
void overlay96BuildVolume(void *object, void *definition);
void *overlay96FindVolume(f32 x, f32 y, f32 z);
s32 overlay96TestBit(u8 *record, s32 index);

/* overlay 97 */
void overlay97InitDirection(void *object, void *config);
void overlay97CopyAngles(void *angles, void *init);
void overlay97InitTransform(void *transform, void *init);
void overlay97InitRadius(void *object, void *init);
void overlay97InitResource(void *object, void *init);
void overlay97InitSelection(void *object, void *init, s32 preserve);
void overlay97AssignState(void *object, void *entry, s32 preserveState);
void overlay97CreateDescriptor(void *object, void *init);
void overlay97InitScale(void *object, void *entryArg);
void overlay97InitBounds(void *object, void *init, s32 preserve);
void overlay97InitPlane(void *object, void *init);

/* overlay 98 */
void overlay98CollectAccepted(s32 count, void *objects);
void overlay98RenderReflections(void *dl, void *matrixHeap, void *vertices);

/* overlay 99 */
void overlay99ReleaseEntries(void);
void overlay99BuildHeightGrid(f32 scale, void *unused, s32 widthMinusOne,
                              s32 heightMinusOne, s32 extra4, s32 extra5);
void func_overlay_099_F0000DDC_18DA38C(void *displayList, void *matrices, void *vertices,
                                       f32 scale, s32 columns, s32 rows, s32 triangles,
                                       s32 stepX, s32 stepY);

/* overlay 100 */
void *overlay100InitializeMotion(f32 x, f32 y, f32 z, s32 count, s32 colorB0, s32 colorB1,
                                 s32 colorB2, s32 colorA0, s32 colorA1, s32 colorA2,
                                 f32 durationSeconds);
void overlay100ReleaseAll(void);
void overlay100ApplyValue(s32 value);
void overlay100ApplyToValue(void *displayList);

/* overlay 101 */
void func_overlay_101_F000C6E8_18E7F08(void);
void overlay101UpdatePresentation(void *context, void *data, void *vertices, s32 step);
void overlay101Cleanup(void);

/* overlay 103 */
s32 overlay103CheckSignature(void);

/* overlay 104 */
s32 overlay104CheckWord(u32 base);

/* overlay 105 */
s32 overlay105CheckWord(u32 base);

#endif
