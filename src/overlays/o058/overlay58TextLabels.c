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

/* Tier B: only sprintf format reads and font input reads; font conversion
 * writes its separate scratch buffer. Named arrays preserve distinct stored
 * strings even where their character contents happen to agree. */
const char o58TimeSeparator[] = ":";
const char o58NegativeGapMarker[] = "-";
const char o58PositiveGapMarker[] = "+";
const char o58SecondsFormat[] = "%02d";
const char o58FractionSeparator[] = ".";
const char o58CentisecondsFormat[] = "%02d";
const char o58PositionFormat[] = "%2d";
const char o58ScoreIncreaseFormat[] = "+ %d";
const char o58ScoreSuffixA[] = "Pe";
const char o58RankScoreFormat[] = "%2d";
const char o58ScoreSuffixB[] = "Pf";
const char o58ScoreSuffixC[] = "Pe";
const char o58CounterFormat[] = "%2d";
const char o58FlagFormat[] = "%2d";
const char o58CurrentMinutesFormat[] = "%2d";
const char o58CurrentTimeSeparator[] = ":";
const char o58CurrentSecondsFormat[] = "%02d";
const char o58CurrentFractionSeparator[] = ".";
const char o58CurrentCentisecondsFormat[] = "%02d";
const char o58RecordMinutesFormat[] = "%2d";
const char o58RecordTimeSeparator[] = ":";
const char o58RecordSecondsFormat[] = "%02d";
const char o58RecordFractionSeparator[] = ".";
const char o58RecordCentisecondsFormat[] = "%02d";
const char o58AlphabetCharacterFormat[] = "%c";
