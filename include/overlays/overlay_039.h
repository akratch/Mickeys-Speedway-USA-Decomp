#ifndef OVERLAY_039_H
#define OVERLAY_039_H

#include "PR/ultratypes.h"
#include "PR/os.h"
#include "PRinternal/siint.h"

/* Overlay 39: DKR v77/v80 and JFG object/source scans are negative for the
 * whole module. */

extern u8 *D_8007A204;
extern u8 __osContLastCmd;

/* The resident caller, joy.c's arithmeticFunction, passes its SI message
 * queue to both; neither reads it. JFG's arithmeticFunction makes the same
 * three calls as osCic6105SendData(challenge, mq), osCic6105StartGetData(mq)
 * and osCic6105GetReadData(response) (tier B, argument roles only). */
struct OSMesgQueue_s;
void overlay39Write(u8 *source, struct OSMesgQueue_s *mq);
void overlay39Reset(struct OSMesgQueue_s *mq);
void overlay39Read(u8 *destination);

#endif
