#include "game/overlay8_tuning.h"

/* Tier B: the indexed getter selects one of these two ten-record banks.
 * Each pointer names its actual runtime export; writable table and record
 * lifetime follow overlay 8. The module linker stores declared import
 * operands while preserving all twenty original R32 identities. */

extern Overlay8TuningRecord D_overlay_008_F0005710_1863468;
extern Overlay8TuningRecord D_overlay_008_F0005868_18635C0;
extern Overlay8TuningRecord D_overlay_008_F00059C0_1863718;
extern Overlay8TuningRecord D_overlay_008_F0005B18_1863870;
extern Overlay8TuningRecord D_overlay_008_F0005C70_18639C8;
extern Overlay8TuningRecord D_overlay_008_F0005DC8_1863B20;
extern Overlay8TuningRecord D_overlay_008_F0005F20_1863C78;
extern Overlay8TuningRecord D_overlay_008_F0006078_1863DD0;
extern Overlay8TuningRecord D_overlay_008_F00061D0_1863F28;
extern Overlay8TuningRecord D_overlay_008_F0006328_1864080;
extern Overlay8TuningRecord D_overlay_008_F0006480_18641D8;
extern Overlay8TuningRecord D_overlay_008_F00065D8_1864330;
extern Overlay8TuningRecord D_overlay_008_F0006730_1864488;
extern Overlay8TuningRecord D_overlay_008_F0006888_18645E0;
extern Overlay8TuningRecord D_overlay_008_F00069E0_1864738;
extern Overlay8TuningRecord D_overlay_008_F0006B38_1864890;
extern Overlay8TuningRecord D_overlay_008_F0006C90_18649E8;
extern Overlay8TuningRecord D_overlay_008_F0006DE8_1864B40;
extern Overlay8TuningRecord D_overlay_008_F0006F40_1864C98;
extern Overlay8TuningRecord D_overlay_008_F0007098_1864DF0;

Overlay8TuningRecord *D_overlay_008_F00071F0_1864F48[10] = {
    &D_overlay_008_F0005710_1863468,
    &D_overlay_008_F0005868_18635C0,
    &D_overlay_008_F00059C0_1863718,
    &D_overlay_008_F0005B18_1863870,
    &D_overlay_008_F0005C70_18639C8,
    &D_overlay_008_F0005DC8_1863B20,
    &D_overlay_008_F0005F20_1863C78,
    &D_overlay_008_F0006078_1863DD0,
    &D_overlay_008_F00061D0_1863F28,
    &D_overlay_008_F0006328_1864080,
};

Overlay8TuningRecord *D_overlay_008_F0007218_1864F70[10] = {
    &D_overlay_008_F0006480_18641D8,
    &D_overlay_008_F00065D8_1864330,
    &D_overlay_008_F0006730_1864488,
    &D_overlay_008_F0006888_18645E0,
    &D_overlay_008_F00069E0_1864738,
    &D_overlay_008_F0006B38_1864890,
    &D_overlay_008_F0006C90_18649E8,
    &D_overlay_008_F0006DE8_1864B40,
    &D_overlay_008_F0006F40_1864C98,
    &D_overlay_008_F0007098_1864DF0,
};
