#include "PR/ultratypes.h"

/* Tier B: the credits descriptor table selects these null-terminated labels
 * for overlay 45 text descriptors. Keep distinct writable arrays in source
 * order; inter-label zeros are the compiler's ordinary alignment, not fields.
 * Tier D: names describe the visible labels, not original source spelling.
 * Existing proxy pointer bindings remain separate from these physical owners. */

/* Five initial entry seeds, consumed by the bounded initialization loop. */
s16 o48InitialEntrySeeds[5] = {40, 80, 120, 160, 200};

char o48CreditBlankBeforeCredits[] = " ";
char o48CreditCredits[] = "CREDITS";
char o48CreditDevelopmentTeam[] = "DEVELOPMENT TEAM";
char o48CreditBCullum[] = "B. CULLUM";
char o48CreditPCunningham[] = "P. CUNNINGHAM";
char o48CreditJDoyle[] = "J. DOYLE";
char o48CreditNGallagher[] = "N. GALLAGHER";
char o48CreditSHall[] = "S. HALL";
char o48CreditNHutchinson[] = "N. HUTCHINSON";
char o48CreditAInnes[] = "A. INNES";
char o48CreditSMcfarlane[] = "S. MCFARLANE";
char o48CreditPMountain[] = "P. MOUNTAIN";
char o48CreditLMusgrave[] = "L. MUSGRAVE";
char o48CreditJPegg[] = "J. PEGG";
char o48CreditBSmyth[] = "B. SMYTH";
char o48CreditMWakeley[] = "M. WAKELEY";
char o48CreditBlankBeforeRareSupport[] = " ";
char o48CreditRareSupport[] = "RARE SUPPORT";
char o48CreditSFarmer[] = "S. FARMER";
char o48CreditPCox[] = "P. COX";
char o48CreditDCrouch[] = "D. CROUCH";
char o48CreditMGreen[] = "M. GREEN";
char o48CreditLLoveday[] = "L. LOVEDAY";
char o48CreditPMikell[] = "P. MIKELL";
char o48CreditAWilson[] = "A. WILSON";
char o48CreditQualityAssurance[] = "QUALITY ASSURANCE";
char o48CreditHWard[] = "H. WARD";
char o48CreditRBullimore[] = "R. BULLIMORE";
char o48CreditMCarter[] = "M. CARTER";
char o48CreditJCook[] = "J. COOK";
char o48CreditLMunton[] = "L. MUNTON";
char o48CreditDMurchie[] = "D. MURCHIE";
char o48CreditDParkinson[] = "D. PARKINSON";
char o48CreditGPhelps[] = "G. PHELPS";
char o48CreditGPrice[] = "G. PRICE";
char o48CreditRSmith[] = "R. SMITH";
char o48CreditJSilke[] = "J. SILKE";
char o48CreditGStevenson[] = "G. STEVENSON";
char o48CreditDWong[] = "D. WONG";
char o48CreditRareUsa[] = "RARE USA";
char o48CreditEHochberg[] = "E. HOCHBERG";
char o48CreditSHochberg[] = "S. HOCHBERG";
char o48CreditJRogowski[] = "J. ROGOWSKI";
char o48CreditNoaThanksTo[] = "NOA THANKS TO";
char o48CreditMFukuda[] = "M. FUKUDA";
char o48CreditDJames[] = "D. JAMES";
char o48CreditMKelbaugh[] = "M. KELBAUGH";
char o48CreditKLobb[] = "K. LOBB";
char o48CreditGTilden[] = "G. TILDEN";
char o48CreditTreehouse[] = "TREEHOUSE";
char o48CreditRShoemake[] = "R. SHOEMAKE";
char o48CreditNoaTesting[] = "NOA TESTING";
char o48CreditNclThanksTo[] = "NCL THANKS TO";
char o48CreditKTerasaki[] = "K. TERASAKI";
char o48CreditMGoto[] = "M. GOTO";
char o48CreditSuperMarioClub[] = "SUPER MARIO CLUB";
char o48CreditNoeThanksTo[] = "NOE THANKS TO";
char o48CreditHUesugi[] = "H. UESUGI";
char o48CreditYEdasawa[] = "Y. EDASAWA";
char o48CreditDisneyInteractive[] = "DISNEY INTERACTIVE";
char o48CreditWBeaman[] = "W. BEAMAN";
char o48CreditJChayes[] = "J. CHAYES";
char o48CreditRCoppinger[] = "R. COPPINGER";
char o48CreditRDempsey[] = "R. DEMPSEY";
char o48CreditSFuller[] = "S. FULLER";
char o48CreditBGlenday[] = "B. GLENDAY";
char o48CreditNHarju[] = "N. HARJU";
char o48CreditDHintonWard[] = "D. HINTON-WARD";
char o48CreditRJohnson[] = "R. JOHNSON";
char o48CreditJLoter[] = "J. LOTER";
char o48CreditNLott[] = "N. LOTT";
char o48CreditSLum[] = "S. LUM";
char o48CreditSOphus[] = "S. OPHUS";
char o48CreditLPriore[] = "L. PRIORE";
char o48CreditJSmith[] = "J. SMITH";
char o48CreditDWinters[] = "D. WINTERS";
char o48CreditVoiceTalent[] = "VOICE TALENT";
char o48CreditWAllwine[] = "W. ALLWINE";
char o48CreditTAnselmo[] = "T. ANSELMO";
char o48CreditCBurton[] = "C. BURTON";
char o48CreditJCummings[] = "J. CUMMINGS";
char o48CreditBFarmer[] = "B. FARMER";
char o48CreditTMacneille[] = "T. MACNEILLE";
char o48CreditRTaylor[] = "R. TAYLOR";
char o48CreditSpecialThanksTo[] = "SPECIAL THANKS TO";
char o48CreditMArakawa[] = "M. ARAKAWA";
char o48CreditRDisney[] = "R. DISNEY";
char o48CreditJHochberg[] = "J. HOCHBERG";
char o48CreditCStamper[] = "C. STAMPER";
char o48CreditTStamper[] = "T. STAMPER";
char o48CreditHYamauchi[] = "H. YAMAUCHI";
char o48CreditBlankAfterCredits[] = " ";

/* Tier B: the final script operation passes this float to amTuneSetFade. */
f32 o48MusicTransitionValue = 0.45f;
