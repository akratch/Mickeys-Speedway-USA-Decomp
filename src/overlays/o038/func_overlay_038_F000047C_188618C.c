typedef signed short s16;
typedef signed int s32;
typedef unsigned int u32;
typedef unsigned char u8;
typedef float f32;

typedef struct O38Command { u32 w0, w1; } O38Command;
typedef struct O38Position { f32 x, y, z; } O38Position;
typedef struct O38Transform {
    s16 angle0, angle2, angle4;
    s16 pad06;
    O38Position position;
    f32 scale;
#ifdef O38_TRANSFORM_TAIL
#ifndef O38_TAIL_SIZE
#define O38_TAIL_SIZE 0x10
#endif
    u8 tail[O38_TAIL_SIZE];
#endif
} O38Transform;
typedef struct O38Particle {
    f32 velocity;
    f32 x, y, z;
    f32 directionX, directionY, directionZ;
} O38Particle;
typedef struct O38Pool {
    s32 count, alpha;
    O38Particle particles[20];
} O38Pool;
typedef struct O38Object {
    s16 type, pad02;
    f32 pad04;
    f32 x, y, z;
    f32 scale;
    u8 pad18[0x4C];
    O38Pool *pool;
    void **resource;
} O38Object;
typedef struct O38Camera { u8 pad00[0xC]; f32 x, y, z; } O38Camera;

extern u8 gO38ObjectVertices[];
extern u8 gO38ObjectTriangles[];
extern u8 gO38ParticleVertices[];
extern u8 gO38ParticleTriangles[];
extern void o38ApplyTransform(O38Command **commands, void *context,
                              O38Transform *transform, f32 scale, f32 extra);
extern void o38DrawResource(O38Command **commands, void *resource,
                            s32 mode, s32 flags);
#ifdef O38_FINISH_ONE
extern void o38FinishDraw(O38Command **commands);
#else
extern void o38FinishDraw(O38Command **commands, const void *displayData);
#endif
extern O38Camera *o38GetCamera(void);
extern s32 o38Atan2(f32 y, f32 x);
extern f32 sqrtf(f32 value);

#define EMIT_COLOR(commands, color) do { \
    O38Command *command = *(commands); \
    *(commands) = command + 1; \
    command->w0 = 0xFA000000U; \
    command->w1 = (color); \
} while (0)
#define EMIT_GEOMETRY(commands, vertices, triangles) do { \
    O38Command *command = *(commands); \
    *(commands) = command + 1; \
    command->w0 = (((((((u32)(vertices) & 6U) | 0x20U) & 0xFFU) << 16) | \
                    0x04000000U) | 0x30U); \
    command->w1 = (u32)(vertices); \
    command = *(commands); \
    *(commands) = command + 1; \
    command->w0 = 0x05110020U; \
    command->w1 = (u32)(triangles); \
} while (0)
#define EMIT_SYNC(commands) do { \
    volatile O38Command *command = *(commands); \
    *(commands) = (O38Command *)(command + 1); \
    command->w1 = 0; \
    command->w0 = 0xE7000000U; \
} while (0)
#define EMIT_FINAL_COLOR(commands) do { \
    volatile O38Command *command = *(commands); \
    *(commands) = (O38Command *)(command + 1); \
    command->w1 = 0xFFFFFFFFU; \
    command->w0 = 0xFA000000U; \
} while (0)
#define EMIT_FINAL_COLOR_FRESH(commands) do { \
    volatile O38Command *command = *(commands); \
    *(commands) = (O38Command *)(command + 1); \
    command->w1 = 0xFFFFFFFFU; \
    ((volatile s32 *)command)[0] = -100663296; \
} while (0)

/* Workbench p7: structure-mismatch, 219/219 instructions/frame -232, 93 masked words, first +0xC0.
 * Geometry-word expression association improves structural residual 21->9; command-pointer postincrement regresses.
 * Context/flag and prior packet, pool-cursor, signedness, declaration, and sync levers remain exhausted; FP lanes exact. */
#ifdef NON_MATCHING
void func_overlay_038_F000047C_188618C(O38Command **commands, void *context,
                                       O38Object *object)
{
    struct O38Locals {
        O38Transform transform;
        O38Pool *pool;
    } locals;
#define transform locals.transform
#define pool locals.pool
    O38Particle *particle;
#ifdef O38_POOL_CURSOR
    char *poolCursor;
#endif
    O38Camera *camera;
    f32 deltaX, deltaY, deltaZ;
    s32 offset;

#undef pool
    locals.pool = object->pool;
#define pool locals.pool

    transform.angle0 = object->type;
    transform.angle2 = 0x4000;
    transform.angle4 = 0;
    transform.position.x = object->x;
    transform.position.y = object->y;
    transform.position.z = object->z;
    transform.scale = object->scale;
    o38ApplyTransform(commands, context, &transform, 1.0f, 0.0f);
    o38DrawResource(commands, *object->resource, 0x10, 0);
    EMIT_COLOR(commands, 0xFFFFFF00U | (pool->alpha & 0xFF));
    EMIT_GEOMETRY(commands, gO38ObjectVertices, gO38ObjectTriangles);
    EMIT_SYNC(commands);
#ifdef O38_VOLATILE_FINAL
    EMIT_FINAL_COLOR(commands);
#else
    EMIT_COLOR(commands, 0xFFFFFFFFU);
#endif
#ifdef O38_FINISH_ONE
    o38FinishDraw(commands);
#else
    o38FinishDraw(commands, gO38ObjectVertices);
#endif

    camera = o38GetCamera();
    particle = pool->particles;
#ifdef O38_POOL_CURSOR
    poolCursor = (char *)pool;
    for (offset = 0; offset != 0x230;
         offset += sizeof(O38Particle), poolCursor += sizeof(O38Particle)) {
        particle = (O38Particle *)(poolCursor + 8);
#else
    for (offset = 0; offset != 0x230; offset += sizeof(O38Particle), particle++) {
#endif
#ifdef O38_VOLATILE_TEST
        if (object->z <= *(volatile f32 *)&particle->y) {
#else
        if (object->z <= particle->y) {
#endif
            deltaX = particle->x - camera->x;
            deltaY = particle->y - camera->y;
            deltaZ = particle->z - camera->z;
            transform.angle0 = o38Atan2(deltaX, deltaZ);
            transform.angle2 = o38Atan2(-deltaY,
                                        sqrtf(deltaX * deltaX + deltaZ * deltaZ)) + 0x8000;
            transform.angle4 = 0;
            transform.position.x = particle->velocity;
            transform.position.y = particle->x;
            transform.position.z = particle->y;
            transform.scale = particle->z;
            o38ApplyTransform(commands, context, &transform, 1.0f, 0.0f);
            o38DrawResource(commands, object->resource[1], 0x10, 0);
            EMIT_COLOR(commands, 0xFFFFFF00U | (pool->alpha & 0xFF));
            EMIT_GEOMETRY(commands, gO38ParticleVertices, gO38ParticleTriangles);
            EMIT_SYNC(commands);
#ifdef O38_VOLATILE_FINAL
            EMIT_FINAL_COLOR_FRESH(commands);
#else
            EMIT_COLOR(commands, 0xFFFFFFFFU);
#endif
#ifdef O38_FINISH_ONE
            o38FinishDraw(commands);
#else
#ifdef O38_FINISH_PARTICLE_VERTICES
            o38FinishDraw(commands, gO38ParticleVertices);
#else
            o38FinishDraw(commands, object->resource[1]);
#endif
#endif
        }
    }
#undef pool
#undef transform
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o038/func_overlay_038_F000047C_188618C/func_overlay_038_F000047C_188618C.s")
#endif

/* PLATEAU-HANDOFF:func_overlay_038_F000047C_188618C:start
 * symbol: func_overlay_038_F000047C_188618C
 * score: 93 differing words
 * frame: 0xE8
 * relocations: 18
 * first-mismatch: +0xC0
 * summary: Remeasured 2026-09-23: 93 masked at delta 0, frame 0xE8 exact, 18 of 18 relocations; integer command-web order remains, FP lanes exact.
 * PLATEAU-HANDOFF:func_overlay_038_F000047C_188618C:end
 */
