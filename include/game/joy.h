#ifndef GAME_JOY_H
#define GAME_JOY_H

#include "PR/ultratypes.h"

struct OSMesgQueue_s;

s32 joyInit(void);
extern s32 D_8007A1D4;

/* The exchange reads 16 challenge bytes and writes 16 response bytes.
 * DMA completion and the lifetime of both buffers remain caller obligations. */
void arithmeticFunction(u8 *challenge, u8 *response);
void overlay39Write(u8 *source, struct OSMesgQueue_s *mq);
void overlay39Reset(struct OSMesgQueue_s *mq);
void overlay39Read(u8 *destination);

#endif
