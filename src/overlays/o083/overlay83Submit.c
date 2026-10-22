#include "PR/ultratypes.h"

extern void overlay83Update(void *object);
extern void overlay83DrawLines(void *request);

/* DKR v77/v80 and JFG checks found no exact donor for this two-stage wrapper. */
void overlay83Submit(void *object, void *request) {
    overlay83Update(object);
    overlay83DrawLines(request);
}
