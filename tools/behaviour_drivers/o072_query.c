#include <assert.h>
#include <stdio.h>

#include BEHAVIOUR_TU

static Overlay72Candidate candidates[8];
static s32 requestedCount;
static s32 calls[8];
static s32 callCount;

s32 func_8005776C(f32 x, f32 y, f32 z, f32 radius, s32 useXZ,
                 Overlay72Candidate **results) {
    s32 i;

    assert(x == 1.0f && y == 2.0f && z == 3.0f);
    assert(radius == 2.5f && useXZ == 1);
    for (i = 0; i < requestedCount; i++) {
        results[i] = &candidates[i];
    }
    return requestedCount;
}

void func_overlay_008_F0000F1C_185EC74(Overlay72Candidate *candidate, s32 value) {
    assert(value == 0);
    assert(callCount < 8);
    calls[callCount++] = candidate - candidates;
}

int main(void) {
    Overlay72Bounds bounds = { 2.5f, 0.0f, 6.0f };
    Overlay72QueryObject object = { 0 };
    s32 i;

    object.x = 1.0f;
    object.y = 2.0f;
    object.z = 3.0f;
    object.bounds = &bounds;

    requestedCount = 0;
    overlay72Update(&object, 1);
    assert(callCount == 0);

    requestedCount = 1;
    candidates[0].height = 2.0f;
    overlay72Update(&object, 1);
    assert(callCount == 1 && calls[0] == 0);

    callCount = 0;
    requestedCount = 8;
    for (i = 0; i < 8; i++) {
        candidates[i].height = i - 1;
    }
    overlay72Update(&object, 1);
    assert(callCount == 5);
    for (i = 0; i < 5; i++) {
        assert(calls[i] == 6 - i);
    }
    puts("overlay72 query scenarios: OK");
    return 0;
}
