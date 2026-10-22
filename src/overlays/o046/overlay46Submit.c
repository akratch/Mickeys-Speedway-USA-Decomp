/* Generic submission wrapper only; no exact DKR donor. */
extern unsigned char gOverlay46SubmitBlock[];
extern void freeFrontEndList(void *block);

void overlay46Submit(void) {
    freeFrontEndList(gOverlay46SubmitBlock);
}
