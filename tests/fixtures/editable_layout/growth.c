/* Appended to a diagnostic copy of the actual overlay75 source. */
u32 layout75State[8];
u32 layout75Seed = 7;

static s32 layout75Private(s32 value) {
    u32 index = (u32) value & 7;
    layout75State[index] = (layout75State[index] + ((u32) value & 255)) & 1023;
    return (s32) layout75State[index];
}

typedef struct Layout75Dispatch {
    s32 (*step)(s32 value);
    u32 *state;
} Layout75Dispatch;

Layout75Dispatch layout75Dispatch = {layout75Private, layout75State};

s32 layout75Step(s32 value) {
    s32 result = layout75Private(value);
    return result + layout75Dispatch.step(value) +
           (s32) *layout75Dispatch.state + (s32) layout75Seed;
}
