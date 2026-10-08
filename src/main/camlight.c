/*
 * Disabled camera-light interface -- ROM 0x1BE50-0x1BEA0.
 *
 * PROVENANCE -- the TU and function names are borrowed from Jet Force
 * Gemini's public retail-derived src/camlight.c and its nonmatching assembly
 * names.  Mickey's same-order entry points are deliberately stubbed; no JFG
 * body is adapted here.  Evidence is tier B/D, not tier-A byte identity.
 */

void camlightInit(void) {
}
void camlightFlush(void) {
}
void *camlightAdd(void *object, void *source) {
    return 0;
}
void camlightDelete(void *light) {
}
void camlightUpdateAll(void) {
}
void camlightUpdate(void *light) {
}
void camlightVisibilityCheck(void) {
}
void camlightDraw(void *displayList, void *matrix, void *light) {
}
