#include "PR/ultratypes.h"

typedef struct Overlay101Gfx {
    u32 w0;
    u32 w1;
} Overlay101Gfx;

typedef struct Overlay101Panel Overlay101Panel;

typedef struct Overlay101Vertex {
    s16 x;
    s16 y;
    s16 z;
    u8 red;
    u8 green;
    u8 blue;
    u8 alpha;
} Overlay101Vertex;

typedef struct Overlay101ClockPoint {
    f32 x;
    f32 y;
} Overlay101ClockPoint;

typedef union Overlay101PhaseValue {
    volatile f32 write;
    f32 read;
} Overlay101PhaseValue;

typedef struct Overlay101PhaseSpill {
    u32 pad0[2];
    Overlay101PhaseValue minor;
    u32 pad1[4];
} Overlay101PhaseSpill;

extern f32 D_E98;
extern f32 D_E9C;
extern s32 D_338;
extern void *D_33C;
extern u8 D_800002D8[];
extern Overlay101ClockPoint D_278[12];

void overlay101SetClockScaleReloc(f32 scale);
void overlay101DrawPanelReloc(Overlay101Gfx **displayList,
                              Overlay101Panel *panel);
void overlay101PrepareClockMaterialReloc(Overlay101Gfx **displayList,
                                         void *material, s32 count,
                                         s32 flags);
f32 overlay101ClockTrigAReloc(s32 angle);
f32 overlay101ClockTrigBReloc(s32 angle);

#define O101_PRIM(packet, color)                                            \
    {                                                                       \
        Overlay101Gfx *macroCommand = (Overlay101Gfx *)(packet);            \
        macroCommand->w0 = 0xFA000000;                                      \
        macroCommand->w1 = (color);                                         \
    }

#define O101_VERTICES(packet, address)                                      \
    {                                                                       \
        Overlay101Gfx *macroCommand = (Overlay101Gfx *)(packet);            \
        void *macroAddress = (void *)(address);                             \
        macroCommand->w0 =                                                  \
            ((((((u32)macroAddress & 6U) | 0x60U) & 0xFFU) << 16) |        \
             0x04000000U) | 0x80U;                                         \
        macroCommand->w1 = (u32)macroAddress;                               \
    }

#define O101_DRAW(packet, address)                                          \
    {                                                                       \
        Overlay101Gfx *macroCommand = (Overlay101Gfx *)(packet);            \
        macroCommand->w0 = 0x05510060;                                      \
        macroCommand->w1 = (u32)(address);                                  \
    }

#define O101_SYNC(packet)                                                   \
    {                                                                       \
        Overlay101Gfx *macroCommand = (Overlay101Gfx *)(packet);            \
        macroCommand->w0 = 0xE7000000;                                      \
        macroCommand->w1 = 0;                                               \
    }

/* Exact C: all 238 instruction words, the -152 frame, relocations, and linked
 * overlay range match after bounded permutation. DKR/JFG are donor-negative.
 *
 * Lane c-5 (2026-10-09): the permuter left the middle of the body as one
 * 1,553-character line of expanded packet macros, and a `new_var` copy of the
 * seconds angle. The copy was inert and is gone; the line is split with the
 * O101_* macros. One physical-line fact survives the split: `item = 2` must
 * share the line of the `do`. as1 breaks the tie between the hoisted alpha
 * 255 and the item 2 by source line, and on its own earlier line the 2 is
 * loaded first (2 words). */
void overlay101DrawClock(Overlay101Gfx **displayList, Overlay101Panel *panel, Overlay101Vertex **vertexCursor, s32 originX, s32 originY)
{
  Overlay101ClockPoint *source;
  Overlay101Vertex *vertex;
  f32 major;
  f32 middle;
  Overlay101PhaseSpill phaseSpill;
  f32 scaledA;
  f32 scaledB;
  s32 majorWhole;
  s32 middleWhole;
  s32 remainder;
  s32 timeValue;
  s32 angle;
  s32 item;
  s32 point;
  s32 baseX;
  s32 baseY;
  u32 spent;
  originX *= 10;
  originY *= 10;
  baseX = originX;
  baseY = originY;
  overlay101SetClockScaleReloc(D_E98);
  overlay101DrawPanelReloc(displayList, panel);
  overlay101SetClockScaleReloc(1.0f);
  timeValue = D_338;
  major = ((f32) timeValue) / D_E9C;
  majorWhole = (s32) major;
  remainder = timeValue - (majorWhole * 216000);
  middle = ((f32) remainder) / 3600.0f;
  middleWhole = (s32) middle;
  spent = ((u32) (majorWhole * 216000)) + ((u32) (3600 * middleWhole));
  phaseSpill.minor.write = ((f32) (timeValue - ((s32) spent))) / 60.0f;
  vertex = *vertexCursor;
  overlay101PrepareClockMaterialReloc(displayList, D_33C, 4, 0);
  O101_PRIM((*displayList)++, 0xFFFFFFA0);
  O101_VERTICES((*displayList)++, (void *) (((u32) vertex) + 0x80000000U));
  O101_DRAW((*displayList)++, D_800002D8);
  O101_SYNC((*displayList)++);
  O101_PRIM((*displayList)++, 0xFFFFFFFF);
  source = D_278;
  item = 2; do /* same line: see above */
  {
    if (item == 2)
    {
      angle = (s32) ((major * 65536.0f) / 24.0f);
      scaledA = overlay101ClockTrigAReloc(angle) * 10.0f;
      scaledB = overlay101ClockTrigBReloc(angle) * 10.0f;
      point = 4;
    }
    else if (item == 1)
    {
      angle = (s32) ((middle * 65536.0f) / 60.0f);
      scaledA = overlay101ClockTrigAReloc(angle) * 10.0f;
      scaledB = 10.0f;
      scaledB = overlay101ClockTrigBReloc(angle) * scaledB;
      point = 4;
    }
    else
    {
      angle = (((s32) phaseSpill.minor.read) << 16) / 60;
      scaledA = overlay101ClockTrigAReloc(angle) * 10.0f;
      scaledB = overlay101ClockTrigBReloc(angle) * 10.0f;
      point = 4;
    }
    if (point--)
    {
      do
      {
        vertex->x = baseX + ((s32) ((source->y * scaledB) + (scaledA * source->x)));
        vertex->y = baseY + ((s32) ((scaledA * source->y) - ((*source).x * scaledB)));
        vertex->z = 0;
        vertex->red = 0;
        vertex->green = 0;
        vertex->blue = 0;
        vertex->alpha = 255;
        source++;
        vertex++;
      }
      while (point--);
    }
  }
  while (item--);
  *vertexCursor = vertex;
}
