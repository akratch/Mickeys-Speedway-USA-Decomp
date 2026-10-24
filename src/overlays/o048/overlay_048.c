#include "overlays/overlay_048.h"

/* Tier B: the credit directory selects these distinct read-only labels.
 * Descriptor creation formats them into separate allocated storage.
 * Tier D: label names describe the text, not original source spelling. */
const char o48CreditBlankBeforeCredits[] = " ";
const char o48CreditCredits[] = "CREDITS";
const char o48CreditDevelopmentTeam[] = "DEVELOPMENT TEAM";
const char o48CreditBCullum[] = "B. CULLUM";
const char o48CreditPCunningham[] = "P. CUNNINGHAM";
const char o48CreditJDoyle[] = "J. DOYLE";
const char o48CreditNGallagher[] = "N. GALLAGHER";
const char o48CreditSHall[] = "S. HALL";
const char o48CreditNHutchinson[] = "N. HUTCHINSON";
const char o48CreditAInnes[] = "A. INNES";
const char o48CreditSMcfarlane[] = "S. MCFARLANE";
const char o48CreditPMountain[] = "P. MOUNTAIN";
const char o48CreditLMusgrave[] = "L. MUSGRAVE";
const char o48CreditJPegg[] = "J. PEGG";
const char o48CreditBSmyth[] = "B. SMYTH";
const char o48CreditMWakeley[] = "M. WAKELEY";
const char o48CreditBlankBeforeRareSupport[] = " ";
const char o48CreditRareSupport[] = "RARE SUPPORT";
const char o48CreditSFarmer[] = "S. FARMER";
const char o48CreditPCox[] = "P. COX";
const char o48CreditDCrouch[] = "D. CROUCH";
const char o48CreditMGreen[] = "M. GREEN";
const char o48CreditLLoveday[] = "L. LOVEDAY";
const char o48CreditPMikell[] = "P. MIKELL";
const char o48CreditAWilson[] = "A. WILSON";
const char o48CreditQualityAssurance[] = "QUALITY ASSURANCE";
const char o48CreditHWard[] = "H. WARD";
const char o48CreditRBullimore[] = "R. BULLIMORE";
const char o48CreditMCarter[] = "M. CARTER";
const char o48CreditJCook[] = "J. COOK";
const char o48CreditLMunton[] = "L. MUNTON";
const char o48CreditDMurchie[] = "D. MURCHIE";
const char o48CreditDParkinson[] = "D. PARKINSON";
const char o48CreditGPhelps[] = "G. PHELPS";
const char o48CreditGPrice[] = "G. PRICE";
const char o48CreditRSmith[] = "R. SMITH";
const char o48CreditJSilke[] = "J. SILKE";
const char o48CreditGStevenson[] = "G. STEVENSON";
const char o48CreditDWong[] = "D. WONG";
const char o48CreditRareUsa[] = "RARE USA";
const char o48CreditEHochberg[] = "E. HOCHBERG";
const char o48CreditSHochberg[] = "S. HOCHBERG";
const char o48CreditJRogowski[] = "J. ROGOWSKI";
const char o48CreditNoaThanksTo[] = "NOA THANKS TO";
const char o48CreditMFukuda[] = "M. FUKUDA";
const char o48CreditDJames[] = "D. JAMES";
const char o48CreditMKelbaugh[] = "M. KELBAUGH";
const char o48CreditKLobb[] = "K. LOBB";
const char o48CreditGTilden[] = "G. TILDEN";
const char o48CreditTreehouse[] = "TREEHOUSE";
const char o48CreditRShoemake[] = "R. SHOEMAKE";
const char o48CreditNoaTesting[] = "NOA TESTING";
const char o48CreditNclThanksTo[] = "NCL THANKS TO";
const char o48CreditKTerasaki[] = "K. TERASAKI";
const char o48CreditMGoto[] = "M. GOTO";
const char o48CreditSuperMarioClub[] = "SUPER MARIO CLUB";
const char o48CreditNoeThanksTo[] = "NOE THANKS TO";
const char o48CreditHUesugi[] = "H. UESUGI";
const char o48CreditYEdasawa[] = "Y. EDASAWA";
const char o48CreditDisneyInteractive[] = "DISNEY INTERACTIVE";
const char o48CreditWBeaman[] = "W. BEAMAN";
const char o48CreditJChayes[] = "J. CHAYES";
const char o48CreditRCoppinger[] = "R. COPPINGER";
const char o48CreditRDempsey[] = "R. DEMPSEY";
const char o48CreditSFuller[] = "S. FULLER";
const char o48CreditBGlenday[] = "B. GLENDAY";
const char o48CreditNHarju[] = "N. HARJU";
const char o48CreditDHintonWard[] = "D. HINTON-WARD";
const char o48CreditRJohnson[] = "R. JOHNSON";
const char o48CreditJLoter[] = "J. LOTER";
const char o48CreditNLott[] = "N. LOTT";
const char o48CreditSLum[] = "S. LUM";
const char o48CreditSOphus[] = "S. OPHUS";
const char o48CreditLPriore[] = "L. PRIORE";
const char o48CreditJSmith[] = "J. SMITH";
const char o48CreditDWinters[] = "D. WINTERS";
const char o48CreditVoiceTalent[] = "VOICE TALENT";
const char o48CreditWAllwine[] = "W. ALLWINE";
const char o48CreditTAnselmo[] = "T. ANSELMO";
const char o48CreditCBurton[] = "C. BURTON";
const char o48CreditJCummings[] = "J. CUMMINGS";
const char o48CreditBFarmer[] = "B. FARMER";
const char o48CreditTMacneille[] = "T. MACNEILLE";
const char o48CreditRTaylor[] = "R. TAYLOR";
const char o48CreditSpecialThanksTo[] = "SPECIAL THANKS TO";
const char o48CreditMArakawa[] = "M. ARAKAWA";
const char o48CreditRDisney[] = "R. DISNEY";
const char o48CreditJHochberg[] = "J. HOCHBERG";
const char o48CreditCStamper[] = "C. STAMPER";
const char o48CreditTStamper[] = "T. STAMPER";
const char o48CreditHYamauchi[] = "H. YAMAUCHI";
const char o48CreditBlankAfterCredits[] = " ";

const char *gOverlay48ResourceTable[93] = {
    o48CreditBlankBeforeCredits,
    o48CreditCredits,
    o48CreditDevelopmentTeam,
    o48CreditBCullum,
    o48CreditPCunningham,
    o48CreditJDoyle,
    o48CreditNGallagher,
    o48CreditSHall,
    o48CreditNHutchinson,
    o48CreditAInnes,
    o48CreditSMcfarlane,
    o48CreditPMountain,
    o48CreditLMusgrave,
    o48CreditJPegg,
    o48CreditBSmyth,
    o48CreditMWakeley,
    o48CreditBlankBeforeRareSupport,
    o48CreditRareSupport,
    o48CreditSFarmer,
    o48CreditPCox,
    o48CreditDCrouch,
    o48CreditMGreen,
    o48CreditLLoveday,
    o48CreditPMikell,
    o48CreditAWilson,
    o48CreditQualityAssurance,
    o48CreditHWard,
    o48CreditRBullimore,
    o48CreditMCarter,
    o48CreditJCook,
    o48CreditLMunton,
    o48CreditDMurchie,
    o48CreditDParkinson,
    o48CreditGPhelps,
    o48CreditGPrice,
    o48CreditRSmith,
    o48CreditJSilke,
    o48CreditGStevenson,
    o48CreditDWong,
    o48CreditRareUsa,
    o48CreditEHochberg,
    o48CreditSHochberg,
    o48CreditJRogowski,
    o48CreditNoaThanksTo,
    o48CreditMFukuda,
    o48CreditDJames,
    o48CreditMKelbaugh,
    o48CreditKLobb,
    o48CreditGTilden,
    o48CreditTreehouse,
    o48CreditRShoemake,
    o48CreditNoaTesting,
    o48CreditNclThanksTo,
    o48CreditKTerasaki,
    o48CreditMGoto,
    o48CreditSuperMarioClub,
    o48CreditNoeThanksTo,
    o48CreditHUesugi,
    o48CreditYEdasawa,
    o48CreditDisneyInteractive,
    o48CreditWBeaman,
    o48CreditJChayes,
    o48CreditRCoppinger,
    o48CreditRDempsey,
    o48CreditSFuller,
    o48CreditBGlenday,
    o48CreditNHarju,
    o48CreditDHintonWard,
    o48CreditRJohnson,
    o48CreditJLoter,
    o48CreditNLott,
    o48CreditSLum,
    o48CreditSOphus,
    o48CreditLPriore,
    o48CreditJSmith,
    o48CreditDWinters,
    o48CreditVoiceTalent,
    o48CreditWAllwine,
    o48CreditTAnselmo,
    o48CreditCBurton,
    o48CreditJCummings,
    o48CreditBFarmer,
    o48CreditTMacneille,
    o48CreditRTaylor,
    o48CreditSpecialThanksTo,
    o48CreditMArakawa,
    o48CreditRDisney,
    o48CreditJHochberg,
    o48CreditCStamper,
    o48CreditTStamper,
    o48CreditHYamauchi,
    o48CreditBlankAfterCredits,
    NULL,
};

enum Overlay48CreditCommand {
    O48_CREDITS_LOAD = -1,
    O48_CREDITS_HOLD = -2,
    O48_CREDITS_RELEASE = -3,
    O48_CREDITS_FINISH = -4
};

s16 gOverlay48CreditCommands[128] = {
    O48_CREDITS_LOAD, 1, 120,
    O48_CREDITS_LOAD, 1, 360,
    O48_CREDITS_LOAD, 4, 180,
    O48_CREDITS_HOLD, 0,
    O48_CREDITS_LOAD, 3, 180,
    O48_CREDITS_LOAD, 3, 180,
    O48_CREDITS_LOAD, 2, 180,
    O48_CREDITS_LOAD, 3, 180,
    O48_CREDITS_RELEASE, 0,
    O48_CREDITS_LOAD, 4, 180,
    O48_CREDITS_HOLD, 0,
    O48_CREDITS_LOAD, 2, 180,
    O48_CREDITS_LOAD, 2, 180,
    O48_CREDITS_RELEASE, 0,
    O48_CREDITS_LOAD, 4, 180,
    O48_CREDITS_HOLD, 0,
    O48_CREDITS_LOAD, 3, 180,
    O48_CREDITS_LOAD, 3, 180,
    O48_CREDITS_LOAD, 2, 180,
    O48_CREDITS_LOAD, 2, 180,
    O48_CREDITS_RELEASE, 0,
    O48_CREDITS_LOAD, 4, 180,
    O48_CREDITS_LOAD, 4, 180,
    O48_CREDITS_HOLD, 0,
    O48_CREDITS_LOAD, 2, 180,
    O48_CREDITS_LOAD, 3, 180,
    O48_CREDITS_RELEASE, 0,
    O48_CREDITS_LOAD, 4, 180,
    O48_CREDITS_LOAD, 3, 180,
    O48_CREDITS_LOAD, 4, 180,
    O48_CREDITS_HOLD, 0,
    O48_CREDITS_LOAD, 3, 180,
    O48_CREDITS_LOAD, 3, 180,
    O48_CREDITS_LOAD, 3, 180,
    O48_CREDITS_LOAD, 2, 180,
    O48_CREDITS_LOAD, 2, 180,
    O48_CREDITS_RELEASE, 0,
    O48_CREDITS_LOAD, 4, 180,
    O48_CREDITS_HOLD, 0,
    O48_CREDITS_LOAD, 2, 180,
    O48_CREDITS_LOAD, 2, 180,
    O48_CREDITS_RELEASE, 0,
    O48_CREDITS_LOAD, 4, 180,
    O48_CREDITS_HOLD, 0,
    O48_CREDITS_LOAD, 3, 180,
    O48_CREDITS_RELEASE, 0,
    O48_CREDITS_LOAD, 1, 360,
    O48_CREDITS_FINISH,
};

s16 o48InitialEntrySeeds[5] = {40, 80, 120, 160, 200};


/* DKR v77/v80 and JFG have no exact matching three-call setup tuple. */
void overlay48Initialize(void) {
    mainChangeLevel(0x1C, 0, 0, 0xD, 1, 0);
    mainChangeCameras(1);
    func_80028DE4(6, 6, 0, 2, 0, 0);
    D_8007BF60 = 0;
}

Overlay48Entry gOverlay48Entries[5];
s16 gOverlay48Timer;
s16 *gOverlay48Script;
s16 gOverlay48ResourceIndex;
s16 gOverlay48Finished;

/* Matched 2026-10-02. One loop over the five entries with the entry pointer
 * walking beside the index: IDO runs the remainder iteration first (entry 0,
 * constant addresses) and unrolls the other four, and because the index is
 * the secondary variable it stays a run-time 1 for the seed address. */
void overlay48InitializeState(void) {
    Overlay48Entry *entry;
    s32 i;

    for (i = 0, entry = gOverlay48Entries; i < 5; i++, entry++) {
        entry->lifetime = 0;
        entry->active = 0;
        entry->seed = o48InitialEntrySeeds[i];
        entry->handle = 0;
    }
    gOverlay48Timer = 0;
    gOverlay48ResourceIndex = 0;
    gOverlay48Finished = 0;
    gOverlay48Script = gOverlay48CreditCommands;
    amTunePlay(0x16);
    func_800015F8();
}

/* A source-level switch preserves the script operation order while giving
 * IDO the retail control web and register allocation. */
void overlay48UpdateState(s32 updateRate) {
    Overlay48Entry *entry;
    s32 index;
    s32 allSettled;
    s32 maximum;
    u32 control;

    allSettled = 1;
    fontUseFont(3);
    entry = gOverlay48Entries;
    maximum = 0xFF;
    index = 0;
    do {
        if (entry->handle != 0) {
            if (entry->lifetime > 0) {
                if (entry->active == 0) {
                    entry->lifetime -= updateRate;
                    allSettled = 0;
                }
                entry->opacity += updateRate * 4;
                if (entry->opacity >= 0x100) {
                    entry->opacity = maximum;
                }
            } else if (entry->active == 0) {
                entry->opacity -= updateRate * 4;
                if (entry->opacity < 0) {
                    entry->opacity = 0;
                    overlay45ReleaseDescriptor(entry->handle);
                    entry->handle = 0;
                } else {
                    allSettled = 0;
                }
            }
            if (entry->handle != 0) {
                overlay45SetMode(entry->handle, entry->opacity);
            }
        }
        index++;
        entry++;
    } while (index < 5);

    if (gOverlay48Finished == 0 &&
        ((gOverlay48Timer -= updateRate) <= 0)) {
        control = *gOverlay48Script;
        switch ((s32) control) {
            case -1:
                if (allSettled != 0) {
                    gOverlay48Script++;
                    index = *gOverlay48Script;
                    gOverlay48Script++;
                    gOverlay48Timer = *gOverlay48Script;
                    gOverlay48Script++;

                    entry = gOverlay48Entries;
                    if (index != 0) {
                        do {
                            if (entry->handle == 0) {
                                entry->handle = overlay45CreateDescriptor(
                                    gOverlay48ResourceTable[
                                        gOverlay48ResourceIndex++],
                                    0xA0, entry->seed, 4);
                                overlay45SetMode(entry->handle, 0);
                                index--;
                                entry->lifetime = gOverlay48Timer;
                            }
                            entry++;
                        } while (index != 0);
                    }

                    while (*gOverlay48Script == -2) {
                        gOverlay48Script++;
                        gOverlay48Entries[*gOverlay48Script].active = 1;
                        gOverlay48Script++;
                    }
                }
                break;
            case -3:
                gOverlay48Script++;
                gOverlay48Entries[*gOverlay48Script].active = 0;
                gOverlay48Entries[*gOverlay48Script].lifetime = 0;
                gOverlay48Script++;
                break;
            case -4:
                gOverlay48Finished = 1;
                mainTitlePageInit(1);
                func_80001608();
                amTuneSetFade(0.45f, 0);
                break;
        }
    }
}

/* DKR v77/v80 only contain generic fixed-handle cleanup relatives. */
void overlay48ReleaseAll(void) {
    s32 index;
    Overlay48Entry *entry;

    entry = gOverlay48Entries;
    index = 0;
    do {
        if (entry->handle != 0) {
            overlay45ReleaseDescriptor(entry->handle);
            entry->handle = 0;
        }
        index++;
        entry++;
    } while (index != 5);
}
