#include "overlays/overlay_059.h"

/* Tier A: the complete forty-frame family and LOCAL list pointers reproduce
 * the initialized prefix. Tier B: PrepareEntry consumes each node list and
 * BuildList copies the remaining node fields; the second node terminates it. */

typedef struct Overlay59TextureFrame {
    Overlay59Node nodes[2];
    Overlay59Node *descriptorList[1];
} Overlay59TextureFrame;

typedef char O59NodeSize[(sizeof(Overlay59Node) == 16) ? 1 : -1];
typedef char O59FrameSize[(sizeof(Overlay59TextureFrame) == 36) ? 1 : -1];
typedef char O59PointerWidth[(sizeof(void *) == 4) ? 1 : -1];

Overlay59TextureFrame o59TextureFrames[40] = {
    { {{677, 0, 0, -22, -22}, {0, 0, 0, 0, 0}}, {o59TextureFrames[0].nodes} },
    { {{678, 0, 0, -22, -22}, {0, 0, 0, 0, 0}}, {o59TextureFrames[1].nodes} },
    { {{679, 0, 0, -22, -22}, {0, 0, 0, 0, 0}}, {o59TextureFrames[2].nodes} },
    { {{680, 0, 0, -22, -22}, {0, 0, 0, 0, 0}}, {o59TextureFrames[3].nodes} },
    { {{681, 0, 0, -22, -22}, {0, 0, 0, 0, 0}}, {o59TextureFrames[4].nodes} },
    { {{682, 0, 0, -22, -22}, {0, 0, 0, 0, 0}}, {o59TextureFrames[5].nodes} },
    { {{683, 0, 0, -22, -22}, {0, 0, 0, 0, 0}}, {o59TextureFrames[6].nodes} },
    { {{684, 0, 0, -22, -22}, {0, 0, 0, 0, 0}}, {o59TextureFrames[7].nodes} },
    { {{685, 0, 0, -22, -22}, {0, 0, 0, 0, 0}}, {o59TextureFrames[8].nodes} },
    { {{686, 0, 0, -22, -22}, {0, 0, 0, 0, 0}}, {o59TextureFrames[9].nodes} },
    { {{687, 0, 0, -22, -22}, {0, 0, 0, 0, 0}}, {o59TextureFrames[10].nodes} },
    { {{688, 0, 0, -22, -22}, {0, 0, 0, 0, 0}}, {o59TextureFrames[11].nodes} },
    { {{689, 0, 0, -22, -22}, {0, 0, 0, 0, 0}}, {o59TextureFrames[12].nodes} },
    { {{690, 0, 0, -22, -22}, {0, 0, 0, 0, 0}}, {o59TextureFrames[13].nodes} },
    { {{691, 0, 0, -22, -22}, {0, 0, 0, 0, 0}}, {o59TextureFrames[14].nodes} },
    { {{692, 0, 0, -22, -22}, {0, 0, 0, 0, 0}}, {o59TextureFrames[15].nodes} },
    { {{693, 0, 0, -22, -22}, {0, 0, 0, 0, 0}}, {o59TextureFrames[16].nodes} },
    { {{694, 0, 0, -22, -22}, {0, 0, 0, 0, 0}}, {o59TextureFrames[17].nodes} },
    { {{695, 0, 0, -22, -22}, {0, 0, 0, 0, 0}}, {o59TextureFrames[18].nodes} },
    { {{696, 0, 0, -22, -22}, {0, 0, 0, 0, 0}}, {o59TextureFrames[19].nodes} },
    { {{697, 0, 0, -22, -22}, {0, 0, 0, 0, 0}}, {o59TextureFrames[20].nodes} },
    { {{698, 0, 0, -22, -22}, {0, 0, 0, 0, 0}}, {o59TextureFrames[21].nodes} },
    { {{699, 0, 0, -22, -22}, {0, 0, 0, 0, 0}}, {o59TextureFrames[22].nodes} },
    { {{700, 0, 0, -22, -22}, {0, 0, 0, 0, 0}}, {o59TextureFrames[23].nodes} },
    { {{701, 0, 0, -22, -22}, {0, 0, 0, 0, 0}}, {o59TextureFrames[24].nodes} },
    { {{702, 0, 0, -22, -22}, {0, 0, 0, 0, 0}}, {o59TextureFrames[25].nodes} },
    { {{703, 0, 0, -22, -22}, {0, 0, 0, 0, 0}}, {o59TextureFrames[26].nodes} },
    { {{704, 0, 0, -22, -22}, {0, 0, 0, 0, 0}}, {o59TextureFrames[27].nodes} },
    { {{705, 0, 0, -22, -22}, {0, 0, 0, 0, 0}}, {o59TextureFrames[28].nodes} },
    { {{706, 0, 0, -22, -22}, {0, 0, 0, 0, 0}}, {o59TextureFrames[29].nodes} },
    { {{707, 0, 0, -22, -22}, {0, 0, 0, 0, 0}}, {o59TextureFrames[30].nodes} },
    { {{708, 0, 0, -22, -22}, {0, 0, 0, 0, 0}}, {o59TextureFrames[31].nodes} },
    { {{709, 0, 0, -22, -22}, {0, 0, 0, 0, 0}}, {o59TextureFrames[32].nodes} },
    { {{710, 0, 0, -22, -22}, {0, 0, 0, 0, 0}}, {o59TextureFrames[33].nodes} },
    { {{711, 0, 0, -22, -22}, {0, 0, 0, 0, 0}}, {o59TextureFrames[34].nodes} },
    { {{712, 0, 0, -22, -22}, {0, 0, 0, 0, 0}}, {o59TextureFrames[35].nodes} },
    { {{713, 0, 0, -22, -22}, {0, 0, 0, 0, 0}}, {o59TextureFrames[36].nodes} },
    { {{714, 0, 0, -22, -22}, {0, 0, 0, 0, 0}}, {o59TextureFrames[37].nodes} },
    { {{715, 0, 0, -22, -22}, {0, 0, 0, 0, 0}}, {o59TextureFrames[38].nodes} },
    { {{716, 0, 0, -22, -22}, {0, 0, 0, 0, 0}}, {o59TextureFrames[39].nodes} },
};

/* Tier B: Advance reads count at stride eight; PrepareEntry selects the
 * corresponding real frame list through the pointer at member four. The
 * caller's descriptor-index and entry-state domains remain preconditions. */
typedef struct Overlay59FrameGroup {
    s32 count;
    Overlay59Node **frames;
} Overlay59FrameGroup;

#ifdef TARGET_N64
typedef char O59FrameGroupSize[(sizeof(Overlay59FrameGroup) == 8) ? 1 : -1];
#endif

Overlay59FrameGroup o59FrameGroups[40] = {
    {1, o59TextureFrames[0].descriptorList},
    {1, o59TextureFrames[1].descriptorList},
    {1, o59TextureFrames[2].descriptorList},
    {1, o59TextureFrames[3].descriptorList},
    {1, o59TextureFrames[4].descriptorList},
    {1, o59TextureFrames[5].descriptorList},
    {1, o59TextureFrames[6].descriptorList},
    {1, o59TextureFrames[7].descriptorList},
    {1, o59TextureFrames[8].descriptorList},
    {1, o59TextureFrames[9].descriptorList},
    {1, o59TextureFrames[10].descriptorList},
    {1, o59TextureFrames[11].descriptorList},
    {1, o59TextureFrames[12].descriptorList},
    {1, o59TextureFrames[13].descriptorList},
    {1, o59TextureFrames[14].descriptorList},
    {1, o59TextureFrames[15].descriptorList},
    {1, o59TextureFrames[16].descriptorList},
    {1, o59TextureFrames[17].descriptorList},
    {1, o59TextureFrames[18].descriptorList},
    {1, o59TextureFrames[19].descriptorList},
    {1, o59TextureFrames[20].descriptorList},
    {1, o59TextureFrames[21].descriptorList},
    {1, o59TextureFrames[22].descriptorList},
    {1, o59TextureFrames[23].descriptorList},
    {1, o59TextureFrames[24].descriptorList},
    {1, o59TextureFrames[25].descriptorList},
    {1, o59TextureFrames[26].descriptorList},
    {1, o59TextureFrames[27].descriptorList},
    {1, o59TextureFrames[28].descriptorList},
    {1, o59TextureFrames[29].descriptorList},
    {1, o59TextureFrames[30].descriptorList},
    {1, o59TextureFrames[31].descriptorList},
    {1, o59TextureFrames[32].descriptorList},
    {1, o59TextureFrames[33].descriptorList},
    {1, o59TextureFrames[34].descriptorList},
    {1, o59TextureFrames[35].descriptorList},
    {1, o59TextureFrames[36].descriptorList},
    {1, o59TextureFrames[37].descriptorList},
    {1, o59TextureFrames[38].descriptorList},
    {1, o59TextureFrames[39].descriptorList},
};
