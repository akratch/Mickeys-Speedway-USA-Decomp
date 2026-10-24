/* Tier B: name entry only reads these alphabet rows; ranking labels pass
 * through the font's separate conversion scratch. The format is read by
 * sprintf/vsprintf while output goes to a separate local text buffer. */
const char o58FirstAlphabetRow[] = "ABCDEFGHIJ";
const char o58SecondAlphabetRow[] = "KLMNOPQRST";
const char o58ThirdAlphabetRow[] = "UVWXYZ!*?#";
const char o58FirstRankLabel[] = "1a";
const char o58SecondRankLabel[] = "2b";
const char o58ThirdRankLabel[] = "3c";
const char o58FourthRankLabel[] = "4d";
const char o58FifthRankLabel[] = "5d";
const char o58SixthRankLabel[] = "6d";
const char o58MinuteFormat[] = "%2d";
