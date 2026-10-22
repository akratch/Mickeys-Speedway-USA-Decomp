#include "PR/ultratypes.h"

typedef struct Overlay14Asset {
    s16 count;
    s16 pad2;
    void *pointer;
    s32 pad8;
    s16 marker;
    s16 padE;
} Overlay14Asset;

typedef struct Overlay14State {
    u8 pad00[0xF4];
    s32 *ranges;
} Overlay14State;

extern Overlay14State gOverlay14State;
extern s32 D_A4;
extern s32 D_B4;
extern void *mmAlloc(s32 size, u32 colourTag);
extern s32 piRomLoadSection(u32 assetIndex, u32 address, s32 assetOffset, s32 size);

Overlay14Asset *func_overlay_014_F00009F4_18702CC(s32 index, s32 context)
{
  s32 pad;
  s32 start;
  s32 size;
  Overlay14Asset *asset;
  s32 i;
  Overlay14Asset *entry;
  void *pointer;
  pad = index;
  start = gOverlay14State.ranges[index];
  size = gOverlay14State.ranges[index + 1] - start;
  asset = (Overlay14Asset *) mmAlloc(size, 0x85);
  if (asset != 0)
  {
    piRomLoadSection(context, (u32) asset, start, size);
    if (1)
    {
      for (i = 0, entry = asset; i < asset->count; i++, entry++)
      {
        pointer = entry->pointer;
        if (pointer == 0)
        {
          if (entry->marker == 0x4000)
          {
            entry->pointer = &D_A4;
          }
          else
          {
            entry->pointer = &D_B4;
          }
        }
        else
        {
          entry->pointer = ((s32) pointer) + ((u8 *) asset);
        }
      }

    }
  }
  return asset;
}
