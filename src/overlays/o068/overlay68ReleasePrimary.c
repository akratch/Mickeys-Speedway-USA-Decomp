/* DKR v77/v80 has generic release wrappers only, with no exact donor. */
extern void *D_8007A1F4;
extern void mmFree(void *resource);

void overlay68ReleasePrimary(void) {
    if (D_8007A1F4 != 0) {
        mmFree(D_8007A1F4);
    }
    D_8007A1F4 = 0;
}
