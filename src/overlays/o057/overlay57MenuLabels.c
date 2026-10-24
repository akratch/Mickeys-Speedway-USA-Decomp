/* Tier B: the panel's four difficulty labels and the three short result
 * labels used by its bounded font loop. Their only consumer path converts
 * input into the separate font scratch buffer before rendering; it does not
 * edit these strings. Existing callers keep their legacy relocation names. */
const char o57AmateurLabel[] = "amateur";
const char o57IntermediateLabel[] = "intermediate";
const char o57ProfessionalLabel[] = "professional";
const char o57MirroredLabel[] = "mirrored";
const char o57FirstResultLabel[] = "1a";
const char o57SecondResultLabel[] = "2b";
const char o57ThirdResultLabel[] = "3c";
