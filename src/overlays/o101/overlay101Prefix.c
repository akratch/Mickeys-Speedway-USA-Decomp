/* Tier B: label-pointer users and the active order/node builder counters.
 * Tier D: the object boundaries below are a natural reconstruction hypothesis.
 * Legacy counter aliases in overlay_101 remain separate source-view debt.
 * The labels are mutable byte arrays; their runtime RO role does not imply const.
 */
#include "PR/ultratypes.h"

extern u8 o101Label000[];
extern u8 o101Label001[];
extern u8 o101Label002[];
extern u8 o101Label003[];
extern u8 o101Label004[];
extern u8 o101Label005[];
extern u8 o101Label006[];
extern u8 o101Label007[];
extern u8 o101Label008[];
extern u8 o101Label009[];
extern u8 o101Label010[];
extern u8 o101Label011[];
extern u8 o101Label012[];
extern u8 o101Label013[];
extern u8 o101Label014[];
extern u8 o101Label015[];
extern u8 o101Label016[];
extern u8 o101Label017[];
extern u8 o101Label018[];
extern u8 o101Label019[];
extern u8 o101Label020[];
extern u8 o101Label021[];
extern u8 o101Label022[];
extern u8 o101Label023[];
extern u8 o101Label024[];
extern u8 o101Label025[];
extern u8 o101Label026[];
extern u8 o101Label027[];
extern u8 o101Label028[];
extern u8 o101Label029[];
extern u8 o101Label030[];
extern u8 o101Label031[];
extern u8 o101Label032[];
extern u8 o101Label033[];
extern u8 o101Label034[];
extern u8 o101Label035[];
extern u8 o101Label036[];
extern u8 o101Label037[];
extern u8 o101Label038[];
extern u8 o101Label039[];
extern u8 o101Label040[];
extern u8 o101Label041[];
extern u8 o101Label042[];
extern u8 o101Label043[];
extern u8 o101Label044[];
extern u8 o101Label045[];
extern u8 o101Label046[];
extern u8 o101Label047[];
extern u8 o101Label048[];
extern u8 o101Label049[];
extern u8 o101Label050[];
extern u8 o101Label051[];
extern u8 o101Label052[];
extern u8 o101Label053[];
extern u8 o101Label054[];
extern u8 o101Label055[];
extern u8 o101Label056[];
extern u8 o101Label057[];
extern u8 o101Label058[];
extern u8 o101Label059[];
extern u8 o101Label060[];
extern u8 o101Label061[];
extern u8 o101Label062[];
extern u8 o101Label063[];
extern u8 o101Label064[];
extern u8 o101Label065[];
extern u8 o101Label066[];
extern u8 o101Label067[];
extern u8 o101Label068[];
extern u8 o101Label069[];
extern u8 o101Label070[];
extern u8 o101Label071[];
extern u8 o101Label072[];
extern u8 o101Label073[];
extern u8 o101Label074[];
extern u8 o101Label075[];
extern u8 o101Label076[];
extern u8 o101Label077[];
extern u8 o101Label078[];
extern u8 o101Label079[];
extern u8 o101Label080[];
extern u8 o101Label081[];
extern u8 o101Label082[];
extern u8 o101Label083[];
extern u8 o101Label084[];
extern u8 o101Label085[];
extern u8 o101Label086[];
extern u8 o101Label087[];
extern u8 o101Label088[];
extern u8 o101Label089[];
extern u8 o101Label090[];
extern u8 o101Label091[];
extern u8 o101Label092[];
extern u8 o101Label093[];
extern u8 o101Label094[];
extern u8 o101Label095[];
extern u8 o101Label096[];
extern u8 o101Label097[];
extern u8 o101Label098[];
extern u8 o101Label099[];
extern u8 o101Label100[];
extern u8 o101Label101[];
extern u8 o101Label102[];
extern u8 o101Label103[];
extern u8 o101Label104[];
extern u8 o101Label105[];
extern u8 o101Label106[];
extern u8 o101Label107[];
extern u8 o101Label108[];
extern u8 o101Label109[];
extern u8 o101Label110[];
extern u8 o101Label111[];
extern u8 o101Label112[];

/* Tier B: the complete contiguous pointer cells and three independently
 * addressed signed counters. Tier D: this decomposition is a source object
 * hypothesis, not a recovered original declaration. No storage is mirrored. */
u8 *o101LabelPointers[113] = {
    o101Label000,
    o101Label001,
    o101Label002,
    o101Label003,
    o101Label004,
    o101Label005,
    o101Label006,
    o101Label007,
    o101Label008,
    o101Label009,
    o101Label010,
    o101Label011,
    o101Label012,
    o101Label013,
    o101Label014,
    o101Label015,
    o101Label016,
    o101Label017,
    o101Label018,
    o101Label019,
    o101Label020,
    o101Label021,
    o101Label022,
    o101Label023,
    o101Label024,
    o101Label025,
    o101Label026,
    o101Label027,
    o101Label028,
    o101Label029,
    o101Label030,
    o101Label031,
    o101Label032,
    o101Label033,
    o101Label034,
    o101Label035,
    o101Label036,
    o101Label037,
    o101Label038,
    o101Label039,
    o101Label040,
    o101Label041,
    o101Label042,
    o101Label043,
    o101Label044,
    o101Label045,
    o101Label046,
    o101Label047,
    o101Label048,
    o101Label049,
    o101Label050,
    o101Label051,
    o101Label052,
    o101Label053,
    o101Label054,
    o101Label055,
    o101Label056,
    o101Label057,
    o101Label058,
    o101Label059,
    o101Label060,
    o101Label061,
    o101Label062,
    o101Label063,
    o101Label064,
    o101Label065,
    o101Label066,
    o101Label067,
    o101Label068,
    o101Label069,
    o101Label070,
    o101Label071,
    o101Label072,
    o101Label073,
    o101Label074,
    o101Label075,
    o101Label076,
    o101Label077,
    o101Label078,
    o101Label079,
    o101Label080,
    o101Label081,
    o101Label082,
    o101Label083,
    o101Label084,
    o101Label085,
    o101Label086,
    o101Label087,
    o101Label088,
    o101Label089,
    o101Label090,
    o101Label091,
    o101Label092,
    o101Label093,
    o101Label094,
    o101Label095,
    o101Label096,
    o101Label097,
    o101Label098,
    o101Label099,
    o101Label100,
    o101Label101,
    o101Label102,
    o101Label103,
    o101Label104,
    o101Label105,
    o101Label106,
    o101Label107,
    o101Label108,
    o101Label109,
    o101Label110,
    o101Label111,
    o101Label112,
};
s32 o101OrderCounter = 0;
s32 o101Node20Counter = 0;
s32 o101Node32Counter = 0;
