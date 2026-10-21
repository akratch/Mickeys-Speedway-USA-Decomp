/*
 * Host stand-in for include/PR/ultratypes.h, used only by
 * tools/behaviour_check.py. The tracked header spells s32/u32 as `long`,
 * which is 32 bits under the o32 ABI the ROM was built for but 64 bits on a
 * 64-bit host; compiled for the host as-is, every u32 shift and wrap would
 * change meaning. This one gives each name its o32 width. behaviour_check.py
 * puts this directory first on the include path and the guard below keeps
 * the tracked header from being read after it.
 */
#ifndef _ULTRATYPES_H_
#define _ULTRATYPES_H_

#include <stddef.h>

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef signed char s8;
typedef short s16;
typedef int s32;
typedef long long s64;

typedef volatile unsigned char vu8;
typedef volatile unsigned short vu16;
typedef volatile unsigned int vu32;
typedef volatile unsigned long long vu64;

typedef volatile signed char vs8;
typedef volatile short vs16;
typedef volatile int vs32;
typedef volatile long long vs64;

typedef float f32;
typedef double f64;

#ifndef NULL
#define NULL ((void *) 0)
#endif
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#endif /* _ULTRATYPES_H_ */
