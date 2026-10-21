/*
 * Behaviour driver for func_80054B3C (src/main/anim.c), the swept-sphere
 * test between two collision shapes, used by tools/behaviour_check.py:
 *
 *   tools/behaviour_check.py CANDIDATE.c --driver tools/behaviour_drivers/anim_collision.c \
 *       --reference src/main/anim.c
 *
 * Synthetic inputs only. Each case fills two AnimCollisionShape blocks and a
 * result block with a sentinel byte pattern, sets the fields the function
 * reads, calls it once and prints the return value and the result fields as
 * exact bit patterns (a field the function did not write keeps the
 * sentinel), plus a digest of both shapes (the function must not write them).
 *
 * Families:
 *   edge     the flags bit 2 on neither, either or both shape; touching
 *            spheres at exactly the summed radius and one ulp either side;
 *            equal step vectors (quadA == 0); a tangent sweep (discriminant
 *            exactly b*b) and one ulp either side of it; first roots at
 *            exactly 0 and 1 and just outside; the second, 83-wider test
 *            failing and passing; the edge-point fallback hit and missed.
 *   sweep    one-dimensional approaches on integer grids, where every
 *            intermediate is exact, through all of the above boundaries.
 *   random   positions, steps and radii on a 1/8 grid and as raw floats,
 *            random flags, including zero and negative radii.
 *   near     steps a few ulps apart, where rounding makes quadA negative and
 *            the arms exact inputs cannot reach become reachable.
 *   special  signed zeros, infinities, NaNs and huge values in each field.
 */
#define BEHAVIOUR_AUTO_STUBS 1
#define BEHAVIOUR_BIG_ENDIAN 1

#include "behaviour_rt.h"
#include BEHAVIOUR_TU

static AnimCollisionShape *gA;
static AnimCollisionShape *gB;
static AnimCollisionResult *gR;
static int gCase;

static f32 bits_to_f32(u32 bits)
{
    union {
        u32 u;
        f32 f;
    } v;
    v.u = bits;
    return v.f;
}

static f32 ulp(f32 x, int dir)
{
    union {
        f32 f;
        u32 u;
    } v;
    v.f = x;
    if (x == 0.0f) {
        return bits_to_f32(dir > 0 ? 1u : 0x80000001u);
    }
    if ((x > 0.0f) == (dir > 0)) {
        v.u++;
    } else {
        v.u--;
    }
    return v.f;
}

static void reset_shapes(void)
{
    u8 *a = (u8 *) gA, *b = (u8 *) gB, *r = (u8 *) gR;
    size_t i;
    for (i = 0; i < sizeof(AnimCollisionShape); i++) {
        a[i] = 0xA5;
        b[i] = 0x5A;
    }
    for (i = 0; i < sizeof(AnimCollisionResult); i++) {
        r[i] = 0xC3;
    }
}

static void set_vec(AnimVec3f *v, f32 x, f32 y, f32 z)
{
    v->x = x;
    v->y = y;
    v->z = z;
}

/* Shape: position p, step s (the vector field), edge e = p + s unless given. */
static void shape(AnimCollisionShape *c, u16 flags, f32 px, f32 py, f32 pz, f32 sx, f32 sy, f32 sz, f32 radius)
{
    c->flags = flags;
    set_vec(&c->position, px, py, pz);
    set_vec(&c->vector, sx, sy, sz);
    set_vec(&c->edge, px + sx, py + sy, pz + sz);
    c->radius = radius;
}

static void check(const char *family)
{
    uint64_t h;
    s32 ret;
    s32 object = 0x1000 + gCase;
    s32 value = 0x2000 - gCase;

    gCase++;
    ret = func_80054B3C(object, gA, value, gB, gR);
    h = bh_mix64(bh_digest(gA, sizeof(AnimCollisionShape)), bh_digest(gB, sizeof(AnimCollisionShape)));
    bh_call_begin(family);
    bh_arg_i("s32", ret);
    bh_arg_i("s32", gR->object);
    bh_arg_i("s32", gR->value);
    bh_arg_f32(gR->fraction);
    bh_arg_f32(gR->normal.x);
    bh_arg_f32(gR->normal.y);
    bh_arg_f32(gR->normal.z);
    bh_note("shapes %016llx", (unsigned long long) h);
    bh_call_end();
}

static u16 flag_variant(int k)
{
    static const u16 FLAGS[4] = {0x0000, 0x0002, 0xFFFD, 0xFFFF};
    return FLAGS[k & 3];
}

static void edge_cases(void)
{
    static const f32 RADII[4] = {0.0f, 1.0f, 2.5f, 3.0f};
    int fa, fb, k, d;

    bh_scenario("edge flags/touch");
    /* Spheres 5 apart (3-4-0), radii summing to 5: touching exactly. */
    for (fa = 0; fa < 4; fa++) {
        for (fb = 0; fb < 4; fb++) {
            for (d = -1; d <= 1; d++) {
                f32 rb = d == 0 ? 3.0f : ulp(3.0f, d);
                reset_shapes();
                shape(gA, flag_variant(fa), 0.0f, 0.0f, 0.0f, 1.0f, 2.0f, 0.5f, 2.0f);
                shape(gB, flag_variant(fb), 3.0f, 4.0f, 0.0f, 1.0f, 2.0f, 0.5f, rb);
                check("touch");
                /* Equal steps: quadA is 0 and only the edge test is left. */
                reset_shapes();
                shape(gA, flag_variant(fa), 0.0f, 0.0f, 0.0f, 7.0f, -1.0f, 2.0f, 2.0f);
                shape(gB, flag_variant(fb), 30.0f, 40.0f, 0.0f, 7.0f, -1.0f, 2.0f, rb);
                gB->edge = gA->edge;
                gB->edge.x += 3.0f;
                gB->edge.y += 4.0f;
                check("quadA0");
            }
        }
    }
    bh_scenario("edge tangent");
    /*
     * Relative step (1, 0, 0), relative start (-10, 3, 0): A = 1, B = -20,
     * C = 109. Tangent at r*r = 9: 4A(C - r*r) = 400 = B*B exactly.
     */
    for (k = 0; k < 4; k++) {
        for (d = -1; d <= 1; d++) {
            f32 r = d == 0 ? 3.0f : ulp(3.0f, d);
            f32 sa = (f32) k;
            reset_shapes();
            shape(gA, 0, -10.0f, 3.0f, 0.0f, sa + 1.0f, 0.0f, 0.0f, r - RADII[k] * 0.0f);
            shape(gB, 0, 0.0f, 0.0f, 0.0f, sa, 0.0f, 0.0f, 0.0f);
            check("tangent");
            /* Split the radius between the shapes. */
            reset_shapes();
            shape(gA, 0, -10.0f, 3.0f, 0.0f, sa + 1.0f, 0.0f, 0.0f, 1.0f);
            shape(gB, 0, 0.0f, 0.0f, 0.0f, sa, 0.0f, 0.0f, d == 0 ? 2.0f : ulp(2.0f, d));
            check("tangent");
        }
    }
    bh_scenario("edge roots");
    /*
     * Head-on along x: A at -D moving +S, B fixed at 0, radii summing to R.
     * The first root is (D - R) / S: 0 at D = R, 1 at D = R + S.
     */
    for (k = 0; k < 4; k++) {
        static const f32 R[4] = {1.0f, 2.0f, 4.0f, 10.0f};
        static const f32 S[4] = {1.0f, 4.0f, 8.0f, 32.0f};
        f32 rr = R[k], ss = S[(k + 1) & 3];
        f32 ds[8];
        int i;
        ds[0] = rr;
        ds[1] = ulp(rr, 1);
        ds[2] = ulp(rr, -1);
        ds[3] = rr + ss;
        ds[4] = ulp(rr + ss, 1);
        ds[5] = ulp(rr + ss, -1);
        ds[6] = rr + ss * 0.5f;
        ds[7] = rr + ss + 10.0f; /* beyond the step: the 83-wider test decides */
        for (i = 0; i < 8; i++) {
            int fl;
            for (fl = 0; fl < 2; fl++) {
                reset_shapes();
                shape(gA, flag_variant(fl), -ds[i], 0.0f, 0.0f, ss, 0.0f, 0.0f, rr * 0.25f);
                shape(gB, 0, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, rr * 0.75f);
                check("roots");
                /* Same, with the edge point moved inside / outside the reach. */
                gA->edge.x = -ds[i] + (fl ? 0.0f : 100.0f);
                check("roots-edge");
            }
        }
    }
}

static void sweeps(void)
{
    int d, s, r, y;

    bh_scenario("sweep");
    for (d = -4; d <= 24; d++) {
        for (s = -8; s <= 8; s += 2) {
            for (r = 0; r <= 12; r += 3) {
                for (y = 0; y <= 6; y += 3) {
                    reset_shapes();
                    shape(gA, (u16) ((d + s) & 2), (f32) -d, (f32) y, 0.0f, (f32) s, 0.0f, 0.0f, (f32) r * 0.5f);
                    shape(gB, (u16) (r & 2), 0.0f, 0.0f, 0.0f, 0.0f, (f32) (s & 3), 0.0f, (f32) r * 0.5f);
                    check("sweep");
                }
            }
        }
    }
}

static f32 grid(void)
{
    return (f32) ((s32) (bh_rand() % 1025u) - 512) / 8.0f;
}

static f32 raw_float(void)
{
    u32 r = bh_rand();
    /* Exponent limited to keep most values finite and moderate. */
    return bits_to_f32((r & 0x807FFFFFu) | ((0x70u + ((r >> 23) % 24u)) << 23));
}

static void randoms(void)
{
    int i;

    bh_scenario("random grid");
    bh_srand(54);
    for (i = 0; i < 3000; i++) {
        f32 ra = (bh_rand() & 7) == 0 ? -grid() * 0.125f : (f32) (bh_rand() % 129u) / 8.0f;
        f32 rb = (bh_rand() & 7) == 1 ? 0.0f : (f32) (bh_rand() % 129u) / 8.0f;
        reset_shapes();
        shape(gA, (u16) bh_rand(), grid(), grid(), grid(), grid() * 0.5f, grid() * 0.5f, grid() * 0.5f, ra);
        shape(gB, (u16) bh_rand(), grid(), grid(), grid(), grid() * 0.5f, grid() * 0.5f, grid() * 0.5f, rb);
        if ((i & 15) == 0) {
            gB->vector = gA->vector;
        }
        if ((i & 31) == 1) {
            gA->edge = gB->edge;
        }
        check("grid");
    }
    bh_scenario("random raw");
    for (i = 0; i < 3000; i++) {
        reset_shapes();
        shape(gA, (u16) bh_rand(), raw_float(), raw_float(), raw_float(), raw_float(), raw_float(), raw_float(),
              raw_float());
        shape(gB, (u16) bh_rand(), raw_float(), raw_float(), raw_float(), raw_float(), raw_float(), raw_float(),
              raw_float());
        gA->edge.x = raw_float();
        gB->edge.z = raw_float();
        check("raw");
    }
}

/*
 * Steps a few ulps apart: quadA is a difference of rounded products, so it
 * can come out a tiny negative number, where the wider second test can fail
 * and the second root can pass 1 -- arms no exact input reaches.
 */
static void near_equal(void)
{
    int i;

    bh_scenario("near-equal steps");
    bh_srand(0xE9);
    for (i = 0; i < 4000; i++) {
        union {
            f32 f;
            u32 u;
        } s[6];
        int k;
        for (k = 0; k < 3; k++) {
            u32 r = bh_rand();
            s[k].u = (r & 0x807FFFFFu) | ((0x7Fu + (r >> 23) % 20u) << 23);
            s[k + 3].u = s[k].u + (u32) ((s32) (bh_rand() % 17u) - 8);
        }
        reset_shapes();
        shape(gA, (u16) (bh_rand() & 2), grid(), grid(), grid(), s[0].f, s[1].f, s[2].f,
              (f32) (bh_rand() % 65u) / 4.0f);
        shape(gB, 0, grid(), grid(), grid(), s[3].f, s[4].f, s[5].f, (f32) (bh_rand() % 65u) / 4.0f);
        if (i & 1) {
            gA->edge = gB->edge;
        }
        check("near");
    }
    /*
     * One input a float32 model of the function found for the arm the
     * random families miss: quadA rounds negative, the first root falls
     * outside [0, 1], the edge points coincide, and the 83-wider
     * discriminant is not below b*b. (The other arm left unreached, a
     * second root above 1 after a first root within [0, 1], cannot happen:
     * every step from the discriminant to the root is monotonic, so the
     * wider sphere's root is never later.)
     */
    reset_shapes();
    shape(gA, 0, 48.875f, -27.75f, -36.0f, bits_to_f32(3316135877u), bits_to_f32(3322510110u),
          bits_to_f32(1147198691u), 6.75f);
    shape(gB, 0, 49.75f, -14.125f, -45.0f, bits_to_f32(3316135875u), bits_to_f32(3322510113u),
          bits_to_f32(1147198698u), 7.75f);
    set_vec(&gA->edge, 24.125f, -32.875f, -37.375f);
    gB->edge = gA->edge;
    check("near-wide-fails");
}

static void specials(void)
{
    static const u32 SPECIAL[8] = {
        0x00000000u, 0x80000000u, 0x7F800000u, 0xFF800000u,
        0x7FC00000u, 0x7F7FFFFFu, 0x00000001u, 0x3F800000u,
    };
    int field, k;

    bh_scenario("special");
    for (field = 0; field < 20; field++) {
        for (k = 0; k < 8; k++) {
            f32 *f;
            reset_shapes();
            shape(gA, 0, -10.0f, 1.0f, 0.5f, 6.0f, 0.25f, 0.0f, 2.0f);
            shape(gB, 0, 0.0f, 0.0f, 0.0f, -2.0f, 0.0f, 0.5f, 1.5f);
            switch (field % 10) {
                case 0: f = &(field < 10 ? gA : gB)->position.x; break;
                case 1: f = &(field < 10 ? gA : gB)->position.y; break;
                case 2: f = &(field < 10 ? gA : gB)->position.z; break;
                case 3: f = &(field < 10 ? gA : gB)->vector.x; break;
                case 4: f = &(field < 10 ? gA : gB)->vector.y; break;
                case 5: f = &(field < 10 ? gA : gB)->vector.z; break;
                case 6: f = &(field < 10 ? gA : gB)->edge.x; break;
                case 7: f = &(field < 10 ? gA : gB)->edge.y; break;
                case 8: f = &(field < 10 ? gA : gB)->edge.z; break;
                default: f = &(field < 10 ? gA : gB)->radius; break;
            }
            *f = bits_to_f32(SPECIAL[k]);
            check("special");
        }
    }
}

int main(void)
{
    bh_reset();
    gA = bh_alloc(sizeof(AnimCollisionShape));
    gB = bh_alloc(sizeof(AnimCollisionShape));
    gR = bh_alloc(sizeof(AnimCollisionResult));
    edge_cases();
    sweeps();
    randoms();
    near_equal();
    specials();
    bh_scenario("globals");
    bh_dump_globals();
    return 0;
}
