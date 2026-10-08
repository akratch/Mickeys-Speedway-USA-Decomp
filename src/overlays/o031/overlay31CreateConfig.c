#include "PR/ultratypes.h"

typedef struct Overlay31ConfigRecord {
    u8 pad00[0x2C];
    s16 level;
    u8 pad2E[0x1E];
    s32 state;
    u8 pad50[0x28];
} Overlay31ConfigRecord;

typedef struct Overlay31Config {
    u8 pad00[4];
    s32 arg2;
    s32 arg3;
    s32 arg0;
    u8 pad10[4];
    Overlay31ConfigRecord *records;
    s32 count;
    s32 reserved1C;
    u32 *masks;
    s32 lastMask;
    s32 reserved28;
    void *source;
    void *source10;
    void *source20;
} Overlay31Config;

extern void *func_overlay_031_F0000000_187F520(s32 size, s32 tag, s32 recordBytes);

Overlay31Config *overlay31CreateConfig(s32 value0, void *source, s32 divisor, s32 multiplier, s32 count)
{
  Overlay31Config *config;
  s32 maskCount;
  s32 recordBytes;
  s32 rawRecordBytes;
  s32 remainder;
  s32 i;
  s32 width;
  u32 bit;
  maskCount = count >> 5;
  remainder = count & 0x1F;
  if (remainder != 0)
  {
    maskCount++;
  }
  rawRecordBytes = (count * 0x78) + 0x38;
  recordBytes = (rawRecordBytes - (((u32) rawRecordBytes) % 4U)) + 4;
  rawRecordBytes = recordBytes;
  rawRecordBytes += maskCount * 4;
  config = func_overlay_031_F0000000_187F520(rawRecordBytes, 0x8C, recordBytes);
  config->count = count;
  config->masks = (u32 *) (((u8 *) config) + recordBytes);
  config->records = (Overlay31ConfigRecord *) (((u8 *) config) + 0x38);
  config->lastMask = maskCount - 1;
  config->reserved28 = 0;
  config->reserved1C = 0;
  config->arg3 = multiplier;
  config->arg2 = divisor;
  config->arg0 = value0;
  if (source != ((void *) 0))
  {
    width = (0x18 / divisor) * multiplier;
    if (!(width < 0x11))
    {
      width = 0x10;
    }
    config->source = source;
    config->source10 = (void *) ((width * 0x10) + ((s32) source));
    config->source20 = (void *) (((width << 2) << 3) + ((s32) source));
  }
  else
  {
    config->source = (void *) 0;
    config->source10 = (void *) 0;
    config->source20 = (void *) 0;
  }
  for (i = 0; i < count; i++)
  {
    config->records[i].state = 0;
    config->records[i].level = 0x80;
  }

  i = 0;
  if (config->lastMask >= 0)
  {
    do
    {
      config->masks[i] = ~0U;
      i++;
    }
    while (config->lastMask >= i);
  }
  bit = 1;
  if (remainder != 0)
  {
    config->masks[config->lastMask] = 0;
    for (i = remainder; i > 0; i--)
    {
      config->masks[config->lastMask] |= bit;
      bit <<= 1;
    }

  }
  return config;
}
