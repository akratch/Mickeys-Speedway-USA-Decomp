#include "overlays/overlay_097.h"

void overlay97InitDirection(Overlay97Object *object, Overlay97Config *config) {
    Overlay97State *state;
    f32 saved;

    if (config->scaleIndex < 5) {
        object->scale = gOverlay97MinimumScale;
    } else {
        object->scale = config->scaleIndex * gOverlay97ScaleStep;
    }

    state = object->state;
    object->angleA = (config->angleA & 0xFF) << 10;
    object->angleB = (config->angleB & 0xFF) << 10;

    saved = overlay97TrigAReloc(object->angleA);
    state->direction[0] = func_8002A8BC(object->angleB) * saved;
    state->direction[1] = -overlay97TrigAReloc(object->angleB);
    saved = overlay97TrigAReloc(object->angleA);
    state->direction[2] = func_8002A8BC(object->angleB) * saved;
    state->offset = -((state->direction[2] * object->position[2]) +
                      ((state->direction[1] * object->position[1]) +
                       (object->position[0] * state->direction[0])));
    state->scaleIndex = config->scaleIndex;
    state->value18 = config->value18;
    state->value15 = config->value15;
}

void overlay97CopyAngles(Overlay97Angles *angles, Overlay97AngleInit *init) {
    angles->x = init->x;
    angles->y = init->y;
    angles->z = init->z;
}

void overlay97InitTransform(Overlay97Transform *transform,
                            Overlay97TransformInit *init) {
    SetLinkSlot(gOverlay97TransformContext, 2, 2, transform);
    transform->mode = init->mode;
    transform->index = init->index;
    transform->angle = (s16)((((s32)init->angle) & 0xFF) << 10);
}
void overlay97InitRadius(Overlay97Object *object, Overlay97InitData *init) {
    f32 radius = (f32)init->radius * 8.0f;
    radius *= radius;
    object->radiusSquared = radius;
}

void overlay97InitResource(Overlay97ResourceObject *object,
                           Overlay97ResourceInit *init) {
    Overlay97Resource *resource;

    resource = object->resource;
    resource->first = init->first;
    resource->second = init->second;
    resource->flags = init->flags;
    resource->mode = init->mode;
    resource->colorA = init->colorA;
    resource->colorB = init->colorB;
    resource->extra = init->extra;
    resource->handle = 0;
    if (amSoundIsLooped(resource->first)) {
        amCreateAudioPoint(resource->first, (f32)init->x,
                                     (f32)init->y, (f32)init->z, 9,
                                     resource->colorB, resource->colorA,
                                     resource->second, resource->flags,
                                     resource->mode, resource->extra, 0,
                                     &resource->handle);
    } else {
        amCreateAudioPoint(resource->first, (f32)init->x,
                                     (f32)init->y, (f32)init->z, 10,
                                     resource->colorB, resource->colorA,
                                     resource->second, resource->flags,
                                     resource->mode, resource->extra, 0,
                                     &resource->handle);
    }
    func_80006EA0(object);
}

void overlay97InitSelection(Overlay97SelectionObject *object,
                            Overlay97SelectionInit *init, s32 preserve) {
    Overlay97Selection *selection;
    Overlay97SelectionTable *table;
    s16 count;

    selection = object->selection;
    table = trackGetTrack();
    selection->index = init->index;
    if (selection->index < 0) {
        selection->index = 0;
    }
    count = table->count;
    if (selection->index >= count) {
        selection->index = count - 1;
    }
    selection->first = init->first;
    selection->second = init->second;
    if (preserve == 0) {
        selection->third = 0;
        selection->fourth = 0;
    }
}

void overlay97AssignState(Overlay97Object *object, void *entry, s32 preserveState) {
    object->state = addRomdefLight(object, entry);
}

void overlay97CreateDescriptor(Overlay97DescriptorObject *object,
                               Overlay97DescriptorInit *init) {
    Overlay97Descriptor descriptor;

    descriptor.flags = 0x21;
    descriptor.target = -1;
    if (init->alternate != 0) {
        descriptor.flags = 0x23;
    }
    if (init->additive != 0) {
        descriptor.flags |= 4;
    }
    if (init->hasTarget != 0) {
        descriptor.target = 0x2B;
    }
    descriptor.kind = 0x50;
    descriptor.y = init->y;
    descriptor.x = init->x;
    descriptor.zero = 0;
    descriptor.vectorX = 0.0f;
    descriptor.vectorY = 0.0f;
    descriptor.vectorZ = 0.0f;
    descriptor.red = init->red;
    descriptor.green = init->green;
    descriptor.blue = init->blue;
    descriptor.alpha = init->alpha;
    descriptor.width = 100;
    descriptor.height = 100;
    descriptor.parent = -1;
    descriptor.enabled = 1;
    object->handle = camlightAdd(object, &descriptor, init);
}

/*
 * PROVENANCE: Diddy Kong Racing, src/object_functions.c
 * (obj_init_scenery), supplies the radius clamp, scale, model-index, and
 * angle semantics. Neither DKR v77/v80 nor JFG contains an exact donor for
 * Mickey's model-bound scan tail. The local header-sized pointer step is
 * intentional: it preserves the original traversal base through the scan.
 *
 * Promotion reproof (2026-08-29): historical configured NON_MATCHING C was
 * measured at 143/144, one word away at +0xD0, frameless and 0x240 bytes with
 * zero relocations. No candidate object, target object, comparison digest, or
 * linked-C artifact survives in canonical Git. The source body is unchanged,
 * but current-HEAD C remains uncompiled; ordinary linked equality proves the
 * fallback only. The owned +0x508..+0x748 / ROM 0x18D83A0..0x18D85E0 range
 * has no padding; Overlay 97's separate +0xA54..+0xA60 padding follows another
 * function. ORT 1194 and resident relocation 36 authenticate
 * func_8000AA38+0x7C as the sole inbound. Historical C derived the values
 * cursor as a1+0x3E while retail used equivalent a3+2. This is an address-
 * carrier allocation difference, not CFG, size, frame, padding, or relocation
 * drift.
 * The model-bound scan is a six-trip loop over a volatile s16 cursor, not an
 * unrolled chain. IDO unrolls it, and the unroller materialises the initial
 * induction pointer (a3 = model + 0x3C) so the first read goes through it,
 * where a hand-written chain lets uopt fold that base into the load. The
 * volatile qualifier is what keeps the load unfolded; without it the same
 * loop costs words in the scan tail.
 */
void overlay97InitScale(Overlay97ScaleObject *object, void *entryArg) {
    f32 radius;
    f32 maximum;
    f32 absolute;
    f32 value;
    register volatile Overlay97ModelValue *values;
    s32 i;
    s32 modelIndex;
    Overlay97ModelInstance *instance;
    Overlay97Interaction *interaction;
    Overlay97ScaleEntry *entry;
    Overlay97ModelHeader *model;

    entry = entryArg;

    radius = entry->radius & 0xFF;
    if (radius < 10.0f) {
        radius = 10.0f;
    }
    radius /= 64.0f;
    object->scale = object->header->scale * radius;
    if (object->shadow != 0) {
        object->shadow->scale = object->header->shadowScale * radius;
        object->shadow->offset = object->header->shadowOffset * radius;
    }
    absolute = radius;
    value = absolute;

    object->modelIndex = entry->modelIndex;
    object->angle = entry->angle << 8;
    interaction = object->interaction;
    interaction->enabled = 1;
    modelIndex = object->modelIndex;
    if (modelIndex >= object->header->modelCount) {
        object->modelIndex = 0;
        modelIndex = object->modelIndex;
    }

    instance = object->models[modelIndex];
    model = instance->model;
    values = (Overlay97ModelValue *)&model->bounds;
    absolute = 0.0f;
    for (i = 0; i < 6; i++) {
        maximum = values->value;
        value = maximum;
        if (maximum < 0.0f) {
            value = -maximum;
        }
        if (absolute < value) {
            absolute = value;
        }
        values++;
    }
    object->radius = object->scale * absolute;
}

void overlay97InitBounds(Overlay97BoundsObject *object,
                         Overlay97BoundsInit *init, s32 preserve) {
    Overlay97Bounds *bounds;

    bounds = object->bounds;
    bounds->first = (f32)init->first;
    bounds->topA = (f32)(init->center + init->halfA);
    bounds->bottomA = (f32)(init->center - init->halfA);
    bounds->second = (f32)init->second;
    bounds->topB = (f32)(init->center + init->halfB);
    bounds->bottomB = (f32)(init->center - init->halfB);
    bounds->third = (u32)(f32)init->third;
    bounds->width = (f32)init->width;
    bounds->height = (f32)init->height;
    bounds->x = object->x + (f32)init->x;
    bounds->y = object->y + (f32)init->y;
    bounds->z = object->z + (f32)init->z;
    if (preserve == 0) {
        func_8001C054();
    }
}

void overlay97InitPlane(Overlay97PlaneObject *object, Overlay97PlaneInit *init) {
    Overlay97Plane *plane;
    s16 angles[3];
    f32 radius;

    plane = object->plane;
    angles[0] = init->angle - 0x4000;
    angles[1] = 0;
    angles[2] = 0;
    plane->x = 0.0f;
    plane->y = 0.0f;
    plane->z = -1.0f;
    mathOneFloatRPY(angles, plane, object, init);
    plane->distance = -((plane->z * object->z) +
                        ((object->x * plane->x) + (object->y * plane->y)));
    plane->radiusSquared = (f32)init->radius;
    radius = plane->radiusSquared;
    plane->radiusSquared = radius * radius;
    plane->height = object->y + (f32)init->height;
    plane->first = init->first;
    plane->second = init->second;
}
