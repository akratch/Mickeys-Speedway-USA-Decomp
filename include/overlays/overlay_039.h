#ifndef OVERLAY_039_H
#define OVERLAY_039_H

#include "PR/ultratypes.h"

/* Overlay 39: DKR v77/v80 and JFG object/source scans are negative for the
 * whole module. */

extern u8 *gOverlay39Buffer;
extern u8 gOverlay39State;

void overlay39BeginReloc(void);
void overlay39SendReloc(u8 *buffer, s32 size);
void overlay39SelectReloc(s32 channel, u8 *buffer);
void overlay39FinishReloc(void);

/* The resident caller, joy.c's arithmeticFunction, passes its SI message
 * queue to both; neither reads it. JFG's arithmeticFunction makes the same
 * three calls as osCic6105SendData(challenge, mq), osCic6105StartGetData(mq)
 * and osCic6105GetReadData(response) (tier B, argument roles only). */
struct OSMesgQueue_s;
void overlay39Write(u8 *source, struct OSMesgQueue_s *mq);
void overlay39Reset(struct OSMesgQueue_s *mq);
void overlay39Read(u8 *destination);

#endif
