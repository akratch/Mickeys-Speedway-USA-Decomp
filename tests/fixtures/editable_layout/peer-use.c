/* Appended only to the peer-referencing diagnostic variant. */
extern void layout108Init(void);
extern s32 layout108Step(s32 value);
extern u32 layout108State[8];
u32 *layout75PeerState = layout108State;

s32 layout75UsePeer(s32 value) {
    layout108Init();
    return layout108Step(value) + (s32) layout75PeerState[(u32) value & 7];
}
