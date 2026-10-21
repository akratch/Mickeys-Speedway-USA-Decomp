/*
 * Host runtime for tools/behaviour_check.py; see behaviour_rt.h.
 *
 * Compiled natively, never through the big-endian transform: it only ever
 * receives values in registers and treats unit memory as bytes (or as
 * native pointer words, which the transform never swaps), so it reads the
 * same thing whichever byte order the unit was built with.
 */
#include "behaviour_rt.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <unistd.h>

/* A fixed hint keeps arena addresses identical across executables, so even
 * a pointer truncated to 32 bits by the code under test stays deterministic.
 * The first allocation sits at the arena base, whose low 32 bits are zero. */
#define BH_ARENA_HINT ((void *) 0x500000000000ULL)
#define BH_ARENA_SIZE ((size_t) 64 << 20)

typedef struct {
    const char *name;
    unsigned char *base;
    size_t size;
    unsigned char *snap;
    uint64_t tag;
} Region;

static unsigned char *arena;
static size_t arena_used;
static size_t arena_peak;
static Region *regions;
static size_t nregions;
static size_t capregions;
static int big_endian;
static int verbose = -1;
static const char *only;
static int muted;
static uintptr_t stack_anchor;
static uint32_t rng = 0x12345678u;
static uintptr_t next_token;

#define BH_COUNTERS 4096
static uint64_t counter_key[BH_COUNTERS];
static uint32_t counter_val[BH_COUNTERS];

static char line[16384];
static size_t line_len;
static int line_args;
static int line_open;

static uint64_t name_tag(const char *s)
{
    uint64_t h = 1469598103934665603ULL;
    while (*s) {
        h = (h ^ (unsigned char) *s++) * 1099511628211ULL;
    }
    return h;
}

static void out(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
static void out(const char *fmt, ...)
{
    va_list ap;
    if (muted) {
        return;
    }
    va_start(ap, fmt);
    vfprintf(stdout, fmt, ap);
    va_end(ap);
}

static void setup(void)
{
    static int done;
    static char buf[1 << 20];
    int here;
    const char *v;
    void *p;

    if (done) {
        return;
    }
    done = 1;
    setvbuf(stdout, buf, _IOFBF, sizeof buf);
    stack_anchor = (uintptr_t) &here;
    v = getenv("BH_VERBOSE");
    verbose = v != NULL && *v != '\0' && *v != '0';
    only = getenv("BH_ONLY");
    if (only != NULL && *only == '\0') {
        only = NULL;
    }
    p = mmap(BH_ARENA_HINT, BH_ARENA_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
    if (p == MAP_FAILED) {
        fprintf(stderr, "behaviour_rt: cannot map the arena\n");
        exit(3);
    }
    if (p != BH_ARENA_HINT) {
        fprintf(stderr, "behaviour_rt: arena not at its fixed address; truncated pointers may vary\n");
    }
    arena = p;
}

static char scenario_label[512];

/* A crash names the scenario it happened in (stdout is lost with it). */
static void on_crash(int sig)
{
    static const char head[] = "behaviour_rt: crashed in scenario: ";
    size_t n = strlen(scenario_label);
    (void) !write(2, head, sizeof head - 1);
    (void) !write(2, scenario_label, n);
    (void) !write(2, "\n", 1);
    signal(sig, SIG_DFL);
    raise(sig);
}

__attribute__((constructor(101))) static void bh_setup_ctor(void)
{
    setup();
    signal(SIGSEGV, on_crash);
    signal(SIGBUS, on_crash);
    signal(SIGFPE, on_crash);
    signal(SIGILL, on_crash);
}

int bh_verbose(void)
{
    setup();
    return verbose;
}

uint32_t bh_hash32(uint32_t x)
{
    x ^= x >> 16;
    x *= 0x7FEB352Du;
    x ^= x >> 15;
    x *= 0x846CA68Bu;
    x ^= x >> 16;
    return x;
}

void bh_srand(uint32_t seed)
{
    rng = bh_hash32(seed ^ 0x9E3779B9u) | 1u;
}

uint32_t bh_rand(void)
{
    rng ^= rng << 13;
    rng ^= rng >> 17;
    rng ^= rng << 5;
    return rng;
}

void bh_fill(void *p, size_t n, uint32_t seed)
{
    unsigned char *q = p;
    size_t i;
    for (i = 0; i < n; i++) {
        q[i] = (unsigned char) (bh_hash32(seed * 0x01000193u + (uint32_t) i) >> 11);
    }
}

void *bh_token(void)
{
    next_token++;
    return (void *) (uintptr_t) (0x7000u + next_token * 0x10u);
}

/* ---- regions ---- */

static Region *find_region(uintptr_t p)
{
    size_t lo = 0, hi = nregions;
    /* regions[] is sorted by base; find the last base <= p. */
    while (lo < hi) {
        size_t mid = (lo + hi) / 2;
        if ((uintptr_t) regions[mid].base <= p) {
            lo = mid + 1;
        } else {
            hi = mid;
        }
    }
    if (lo == 0) {
        return NULL;
    }
    if (p <= (uintptr_t) regions[lo - 1].base + regions[lo - 1].size) {
        return &regions[lo - 1];
    }
    return NULL;
}

void bh_region(const char *name, const void *base, size_t size)
{
    size_t i, at;
    setup();
    for (i = 0; i < nregions; i++) {
        if (regions[i].base == base) {
            return;
        }
    }
    if (nregions == capregions) {
        capregions = capregions ? capregions * 2 : 64;
        regions = realloc(regions, capregions * sizeof *regions);
        if (regions == NULL) {
            exit(3);
        }
    }
    at = nregions;
    while (at > 0 && (uintptr_t) regions[at - 1].base > (uintptr_t) base) {
        at--;
    }
    memmove(&regions[at + 1], &regions[at], (nregions - at) * sizeof *regions);
    regions[at].name = name;
    regions[at].base = (unsigned char *) base;
    regions[at].size = size;
    regions[at].tag = name_tag(name);
    regions[at].snap = malloc(size ? size : 1);
    if (regions[at].snap == NULL) {
        exit(3);
    }
    memcpy(regions[at].snap, base, size);
    nregions++;
}

void bh_set_big_endian(int on)
{
    big_endian = on;
}

static const char *plan_seed(const char *plan, unsigned char **pp, unsigned char *end, int depth)
{
    unsigned char *p = *pp;
    while (*plan && *plan != ')') {
        char kind = *plan++;
        unsigned long n = 0;
        if (kind == 'r' || kind == 'z' || kind == '*') {
            n = strtoul(plan, (char **) &plan, 10);
        }
        if (kind == 'r') {
            unsigned long i;
            if (p + n > end) {
                goto bad;
            }
            for (i = 0; i < n; i++) {
                *p++ = (unsigned char) (bh_rand() >> 7);
            }
        } else if (kind == 'z') {
            if (p + n > end) {
                goto bad;
            }
            memset(p, 0, n);
            p += n;
        } else if (kind == 'f' || kind == 'd') {
            /* A small finite value, so arithmetic on it stays meaningful. */
            double v = ((double) (int) (bh_rand() % 2001u) - 1000.0) / 8.0;
            unsigned char b[8];
            size_t w = kind == 'f' ? 4 : 8, i;
            if (p + w > end) {
                goto bad;
            }
            if (kind == 'f') {
                float f = (float) v;
                memcpy(b, &f, 4);
            } else {
                memcpy(b, &v, 8);
            }
            for (i = 0; i < w; i++) {
                p[i] = big_endian ? b[w - 1 - i] : b[i];
            }
            p += w;
        } else if (kind == '*') {
            unsigned long k;
            const char *after = NULL;
            if (*plan != '(' || n == 0 || depth > 32) {
                goto bad;
            }
            for (k = 0; k < n; k++) {
                after = plan_seed(plan + 1, &p, end, depth + 1);
                if (after == NULL || *after != ')') {
                    goto bad;
                }
            }
            plan = after + 1;
        } else {
            goto bad;
        }
    }
    *pp = p;
    return plan;
bad:
    return NULL;
}

void bh_region_auto(const char *name, void *base, size_t size, const char *plan)
{
    unsigned char *p = base;
    const char *rest;
    setup();
    bh_srand((uint32_t) name_tag(name));
    rest = plan_seed(plan, &p, (unsigned char *) base + size, 0);
    if (rest == NULL || *rest != '\0' || p != (unsigned char *) base + size) {
        fprintf(stderr, "behaviour_rt: seed plan for %s does not cover its %zu bytes\n", name, size);
        exit(3);
    }
    bh_region(name, base, size);
}

/* ---- arena ---- */

void *bh_alloc(size_t size)
{
    void *p;
    setup();
    arena_used = (arena_used + 15) & ~(size_t) 15;
    if (arena_used + size > BH_ARENA_SIZE) {
        fprintf(stderr, "behaviour_rt: arena exhausted\n");
        exit(3);
    }
    p = arena + arena_used;
    arena_used += size;
    if (arena_used > arena_peak) {
        arena_peak = arena_used;
    }
    return p;
}

void bh_reset(void)
{
    size_t i;
    setup();
    memset(arena, 0, arena_peak);
    arena_used = 0;
    /* Restore only what changed: a read-only region is never written. */
    for (i = 0; i < nregions; i++) {
        if (memcmp(regions[i].base, regions[i].snap, regions[i].size) != 0) {
            memcpy(regions[i].base, regions[i].snap, regions[i].size);
        }
    }
    memset(counter_key, 0, sizeof counter_key);
    memset(counter_val, 0, sizeof counter_val);
    next_token = 0;
    rng = 0x12345678u;
}

/* ---- pointers ---- */

const char *bh_ptr(const void *p)
{
    static char ring[8][96];
    static int k;
    char *s = ring[k++ & 7];
    uintptr_t v = (uintptr_t) p;
    Region *r;

    setup();
    if (v == 0) {
        return "null";
    }
    if (v >= (uintptr_t) arena && v <= (uintptr_t) arena + BH_ARENA_SIZE) {
        snprintf(s, 96, "arena+0x%lx", (unsigned long) (v - (uintptr_t) arena));
    } else if ((r = find_region(v)) != NULL) {
        snprintf(s, 96, "%s+0x%lx", r->name, (unsigned long) (v - (uintptr_t) r->base));
    } else if (v < 0x100000000ULL) {
        snprintf(s, 96, "0x%lx", (unsigned long) v);
    } else if (v - stack_anchor + (8u << 20) < (16u << 20)) {
        return "stack";
    } else {
        snprintf(s, 96, "raw:0x%lx", (unsigned long) v);
    }
    return s;
}

/* ---- digests ---- */

static uint64_t mix(uint64_t h, uint64_t w)
{
    h ^= w + 0x9E3779B97F4A7C15ULL + (h << 6) + (h >> 2);
    h *= 0xFF51AFD7ED558CCDULL;
    return h ^ (h >> 33);
}

/* The canonical form of one aligned word: a pointer into the arena or a
 * region becomes (region, offset); anything else is itself. */
static uint64_t canon(uint64_t w, int *was_ptr)
{
    Region *r;
    *was_ptr = 1;
    if (w >= (uintptr_t) arena && w < (uintptr_t) arena + BH_ARENA_SIZE) {
        return mix(0xA4E7A, w - (uintptr_t) arena);
    }
    if (w > 0xFFFFFFFFULL && (r = find_region((uintptr_t) w)) != NULL) {
        return mix(r->tag, w - (uintptr_t) r->base);
    }
    *was_ptr = 0;
    return w;
}

/* The digest is a function of the bytes and of where pointers point, never
 * of where the memory itself sits: bytes are mixed one at a time (so the
 * alignment of the block does not group them differently), and an aligned
 * word that points into the arena or a region is mixed as (region, offset). */
static uint64_t digest(const unsigned char *p, size_t n, int show, const char *label)
{
    uint64_t h = 0xCBF29CE484222325ULL;
    size_t i = 0;
    size_t row = 0;
    char hex[3 * 16 + 1];
    int hl = 0;

    while (i < n) {
        if ((((uintptr_t) (p + i)) & 7) == 0 && i + 8 <= n) {
            uint64_t w;
            int was_ptr;
            uint64_t c;
            memcpy(&w, p + i, 8);
            c = canon(w, &was_ptr);
            if (was_ptr) {
                h = mix(h ^ 0x5054525F5054525FULL, c);
                if (show) {
                    if (hl) {
                        out("  %s+0x%zx: %s\n", label, row, hex);
                        hl = 0;
                    }
                    out("  %s+0x%zx: ptr %s\n", label, i, bh_ptr((const void *) (uintptr_t) w));
                }
                i += 8;
                continue;
            }
        }
        h = (h ^ p[i]) * 0x100000001B3ULL;
        if (show) {
            if (hl == 0) {
                row = i;
            }
            snprintf(hex + hl * 3, 4, "%02x ", p[i]);
            if (++hl == 16) {
                out("  %s+0x%zx: %s\n", label, row, hex);
                hl = 0;
            }
        }
        i++;
    }
    if (show && hl) {
        out("  %s+0x%zx: %s\n", label, row, hex);
    }
    return mix(h, n);
}

uint64_t bh_digest(const void *p, size_t n)
{
    setup();
    return digest(p, n, 0, "");
}

uint64_t bh_mix64(uint64_t h, uint64_t x)
{
    return mix(h, x);
}

static volatile int32_t *watch_target;
static volatile sig_atomic_t watch_fired;
static volatile sig_atomic_t watch_expected;
static volatile sig_atomic_t watch_unexpected_total;

static void watch_abandon(const char *why, size_t len)
{
    (void) !write(2, why, len);
    (void) !write(2, scenario_label, strlen(scenario_label));
    (void) !write(2, "\n", 1);
    _exit(4);
}

/* CPU time (ITIMER_VIRTUAL), not wall time: a loaded machine does not fire
 * it early. The timer repeats: code that keeps setting the flag again (a
 * broken candidate looping through its own state change) is cleared again,
 * and after eight periods the run is abandoned. An unexpected firing (the
 * code was not meant to spin) is a difference already, so after four of
 * them in one run the run is abandoned too: a broken candidate costs a
 * second, not the caller's whole timeout. */
static void on_alarm(int sig)
{
    static const char stuck[] = "behaviour_rt: watchdog: still running after 8 periods in scenario: ";
    static const char many[] = "behaviour_rt: watchdog: 4 unexpected spins, the last in scenario: ";
    (void) sig;
    watch_fired++;
    if (watch_target != NULL) {
        *watch_target = 0;
    }
    if (watch_fired > 8) {
        watch_abandon(stuck, sizeof stuck - 1);
    }
    if (!watch_expected && watch_fired == 1 && ++watch_unexpected_total >= 4) {
        watch_abandon(many, sizeof many - 1);
    }
}

void bh_watchdog(long usec, volatile int32_t *clear, int expected)
{
    struct itimerval t;
    struct sigaction sa;
    memset(&sa, 0, sizeof sa);
    sa.sa_handler = on_alarm;
    sa.sa_flags = SA_RESTART;
    sigaction(SIGVTALRM, &sa, NULL);
    watch_target = clear;
    watch_fired = 0;
    watch_expected = expected;
    memset(&t, 0, sizeof t);
    t.it_value.tv_sec = usec / 1000000;
    t.it_value.tv_usec = (int) (usec % 1000000);
    t.it_interval = t.it_value;
    setitimer(ITIMER_VIRTUAL, &t, NULL);
}

int bh_watchdog_disarm(void)
{
    struct itimerval t;
    memset(&t, 0, sizeof t);
    setitimer(ITIMER_VIRTUAL, &t, NULL);
    watch_target = NULL;
    return watch_fired != 0;
}

void bh_dump_mem(const char *label, const void *p, size_t n)
{
    uint64_t h;
    setup();
    if (muted) {
        return;
    }
    h = digest(p, n, verbose, label);
    out("mem %s n=%zu h=%016llx\n", label, n, (unsigned long long) h);
}

void bh_dump_arena(const char *label)
{
    bh_dump_mem(label, arena, arena_used);
}

static int by_name(const void *a, const void *b)
{
    const Region *x = *(const Region *const *) a;
    const Region *y = *(const Region *const *) b;
    return strcmp(x->name, y->name);
}

/* Regions in name order: the link layout must not change the digest. */
void bh_dump_globals(void)
{
    static Region **order;
    static size_t ordered;
    size_t i;
    uint64_t all = 0;
    setup();
    if (muted) {
        return;
    }
    if (ordered != nregions) {
        order = realloc(order, (nregions ? nregions : 1) * sizeof *order);
        if (order == NULL) {
            exit(3);
        }
        for (i = 0; i < nregions; i++) {
            order[i] = &regions[i];
        }
        qsort(order, nregions, sizeof *order, by_name);
        ordered = nregions;
    }
    for (i = 0; i < nregions; i++) {
        Region *r = order[i];
        uint64_t h = digest(r->base, r->size, 0, r->name);
        all = mix(all, mix(r->tag, h));
        if (verbose) {
            int changed = memcmp(r->base, r->snap, r->size) != 0;
            out("global %s%s h=%016llx\n", r->name, changed ? " changed" : "", (unsigned long long) h);
            if (changed) {
                digest(r->base, r->size, 1, r->name);
            }
        }
    }
    out("globals h=%016llx\n", (unsigned long long) all);
}

/* ---- trace ---- */

static void vline(const char *fmt, va_list ap)
{
    int n = vsnprintf(line + line_len, sizeof line - line_len, fmt, ap);
    if (n > 0) {
        line_len += (size_t) n;
        if (line_len >= sizeof line) {
            line_len = sizeof line - 1;
        }
    }
}

static void addf(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
static void addf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vline(fmt, ap);
    va_end(ap);
}

void bh_log(const char *fmt, ...)
{
    va_list ap;
    setup();
    if (muted) {
        return;
    }
    va_start(ap, fmt);
    vfprintf(stdout, fmt, ap);
    va_end(ap);
    fputc('\n', stdout);
}

void bh_scenario(const char *fmt, ...)
{
    char label[512];
    va_list ap;
    setup();
    va_start(ap, fmt);
    vsnprintf(label, sizeof label, fmt, ap);
    va_end(ap);
    muted = only != NULL && strcmp(only, label) != 0;
    memcpy(scenario_label, label, sizeof scenario_label);
    scenario_label[sizeof scenario_label - 1] = '\0';
    out("== %s\n", label);
}

void bh_call_begin(const char *name)
{
    setup();
    line_len = 0;
    line_args = 0;
    line_open = 1;
    addf("call %s(", name);
}

static void close_args(void);

static void sep(void)
{
    if (line_args++) {
        addf(", ");
    }
}

void bh_arg_i(const char *type, long long value)
{
    unsigned long long v = (unsigned long long) value;
    sep();
    if (strcmp(type, "u8") == 0 || strcmp(type, "s8") == 0 || strcmp(type, "i8") == 0) {
        v &= 0xFFu;
    } else if (strcmp(type, "u16") == 0 || strcmp(type, "s16") == 0 || strcmp(type, "i16") == 0) {
        v &= 0xFFFFu;
    } else if (strcmp(type, "i64") != 0) {
        v &= 0xFFFFFFFFu;
    }
    addf("%s 0x%llx", type, v);
}

void bh_arg_f32(float value)
{
    uint32_t bits;
    memcpy(&bits, &value, 4);
    sep();
    addf("f32 %.9g[%08x]", (double) value, bits);
}

void bh_arg_f64(double value)
{
    uint64_t bits;
    memcpy(&bits, &value, 8);
    sep();
    addf("f64 %.17g[%016llx]", value, (unsigned long long) bits);
}

void bh_arg_p(const void *p)
{
    sep();
    addf("p %s", bh_ptr(p));
}

void bh_note(const char *fmt, ...)
{
    va_list ap;
    close_args();
    addf(" ");
    va_start(ap, fmt);
    vline(fmt, ap);
    va_end(ap);
}

static void close_args(void)
{
    if (line_open) {
        addf(")");
        line_open = 0;
    }
}

void bh_call_end(void)
{
    close_args();
    if (!muted) {
        fwrite(line, 1, line_len, stdout);
        fputc('\n', stdout);
    }
    line_len = 0;
}

static uint32_t next_count(const char *name)
{
    uint64_t key = name_tag(name) | 1;
    size_t i = (size_t) (key % BH_COUNTERS);
    size_t probes;
    for (probes = 0; probes < BH_COUNTERS; probes++) {
        if (counter_key[i] == key || counter_key[i] == 0) {
            counter_key[i] = key;
            return counter_val[i]++;
        }
        i = (i + 1) % BH_COUNTERS;
    }
    return 0;
}

long long bh_ret_int(const char *name)
{
    uint32_t n = next_count(name);
    uint32_t v = bh_hash32((uint32_t) name_tag(name) ^ (n * 0x9E3779B9u));
    close_args();
    addf(" -> 0x%x", v);
    return (long long) v;
}

float bh_ret_f32(const char *name)
{
    uint32_t n = next_count(name);
    uint32_t v = bh_hash32((uint32_t) name_tag(name) ^ (n * 0x9E3779B9u));
    float f = (float) ((int) (v % 4001u) - 2000) / 16.0f;
    close_args();
    addf(" -> %.9g", (double) f);
    return f;
}

double bh_ret_f64(const char *name)
{
    return (double) bh_ret_f32(name);
}

static void vargs(const char *sig, va_list ap)
{
    for (; *sig; sig++) {
        switch (*sig) {
            case 'i':
                bh_arg_i("i32", va_arg(ap, int));
                break;
            case 'l':
                bh_arg_i("i64", va_arg(ap, long long));
                break;
            case 'd':
                bh_arg_f64(va_arg(ap, double));
                break;
            case 'p':
                bh_arg_p(va_arg(ap, void *));
                break;
            default:
                fprintf(stderr, "behaviour_rt: bad vcall signature %s\n", sig);
                exit(3);
        }
    }
}

/* sig is "<ret>:<args>": ret one of v i l p; args from i l d p. */
long long bh_vcall(const char *name, const char *sig, ...)
{
    va_list ap;
    long long r = 0;
    bh_call_begin(name);
    va_start(ap, sig);
    vargs(sig + 2, ap);
    va_end(ap);
    if (sig[0] == 'i' || sig[0] == 'l') {
        r = bh_ret_int(name);
    } else if (sig[0] == 'p') {
        close_args();
        addf(" -> null");
    }
    bh_call_end();
    return r;
}

double bh_vcall_fp(const char *name, const char *sig, ...)
{
    va_list ap;
    double r;
    bh_call_begin(name);
    va_start(ap, sig);
    vargs(sig + 2, ap);
    va_end(ap);
    r = bh_ret_f64(name);
    bh_call_end();
    return r;
}
