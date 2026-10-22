/* DKR v77/v80 has generic release wrappers only, with no exact donor. */
extern void *D_8007A1F8;
extern void mmFree(void *resource);

void overlay68ReleaseTertiary(void) {
    if (D_8007A1F8 != 0) {
        mmFree(D_8007A1F8);
    }
    D_8007A1F8 = 0;
}
