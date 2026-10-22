#include "PR/ultratypes.h"

extern u8 gOverlay55SetupBase[];
extern u8 gOverlay55SourceBase[];
extern u8 gOverlay55ExternalSetupTarget[];
extern u8 gOverlay55SetupSecondary[];
extern u8 gOverlay55RecordsA[];
extern u8 gOverlay55RecordsB[];
extern u8 gOverlay55SourceB[];
extern s8 gOverlay55Status[];
extern s32 gOverlay55StateWord;
extern f32 gOverlay55StateValue;
extern s16 gOverlay55Result;

extern void loadFrontEndList(void *target);
extern void setupFrontEndList(void *target);
extern void amTunePlay(s32 mode);
extern void runlinkDownloadCode(s32 resource);
extern void overlay55PatchIndices(void *records);
extern void overlay55CopyOffsetRecords(
    void *records, void *source, s32 index, s32 offset);
extern void overlay56LoadResource(void);
extern s32 levelGetNumber(void);

void overlay55Initialize(void) {
    s32 index;
    u8 *sourceA;
    u8 *sourceB;
    s8 *status;

    loadFrontEndList(gOverlay55SetupBase);
    loadFrontEndList(gOverlay55ExternalSetupTarget);
    setupFrontEndList(gOverlay55SetupSecondary);
    amTunePlay(4);
    gOverlay55StateWord = 0x104;
    runlinkDownloadCode(11);
    overlay55PatchIndices(gOverlay55RecordsA);
    overlay55PatchIndices(gOverlay55RecordsB);

    sourceA = gOverlay55SourceBase,
    sourceB = gOverlay55SourceB,
    status = gOverlay55Status;
    index = 0;
    do {
        overlay55CopyOffsetRecords(gOverlay55RecordsA, sourceA, index, 0);
        overlay55CopyOffsetRecords(gOverlay55RecordsB, sourceB, index, 0);
        index++;
        sourceA += 0xA0;
        sourceB += 0x20;
        status++;
        status[-1] = -1;
    } while (index != 4);

    gOverlay55StateValue = -80.0f;
    overlay56LoadResource();
    gOverlay55Result = (s16)levelGetNumber();
}
