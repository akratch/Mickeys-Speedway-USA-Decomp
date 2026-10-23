#include "overlays/overlay_095.h"

const f32 gOverlay95FadeRate = 127.0f / 60.0f;

/*
 * Overlay 95, ADR 0006 consolidation: one translation unit for the whole
 * module (overlay95NoOp at +0x000, overlay95Update at +0x00C).
 */

void overlay95NoOp(void *object, void *entry) {
}

void overlay95Update(Overlay95Object *object, s32 updateRate) {
    Overlay95AudioState *audio;
    Overlay95QueryResult *queryResult;
    Overlay95Racer *racer;
    Overlay95RacerObject *racerOwner;
    void *found;
    void *resource;
    s32 volume;

    if (D_8007BF0C == 0) {
        audio = object->audio;
        queryResult = func_80005750(&found);
        if (found != 0) {
            racerOwner = queryResult->object;
            racer = racerOwner->racer;
            if (!(racer->flags & 1) && racer->blocked == 0) {
                if (racer->mode >= 0x30 && racer->mode < 0x40) {
                    object->flags |= 1;
                    partUpdateTriggers(object, updateRate);
                    if (object->active == 0) {
                        object->active = 1;
                        if (object->timer == 0) {
                            rumbleStart(racer->type, 0x4B, 2.0f, racer);
                            resource = camGetNo();
                            camStartShake(resource, 0.5f, 1.0f, 0.5f, 15);
                            if (audio->handle != 0) {
                                amSndStop(audio->handle);
                            }
                            amSndPlay(0x31E, audio);
                            audio->volume = 0x7F;
                            object->timer = 0xF0;
                        }
                    }
                } else {
                    object->active = 0;
                }
            }
        }
        if (object->timer != 0) {
            object->timer -= updateRate;
            if (object->timer < 0x3C) {
                volume = audio->volume - (s32) (gOverlay95FadeRate * (f32) updateRate);
                audio->volume = volume;
                if (volume < 0) {
                    audio->volume = 0;
                    volume = 0;
                }
                amSndSetVol(0x31E, audio->handle, volume & 0xFF);
            }
            if (object->timer <= 0) {
                object->timer = 0;
                if (audio->handle != 0) {
                    amSndStop(audio->handle);
                }
            }
        }
    }
}
