typedef signed char s8;
typedef unsigned char u8;
typedef signed short s16;
typedef signed long s32;
typedef float f32;

typedef struct Overlay83Record {
    s16 x;
    s16 y;
    s16 z;
    s8 life;
    s8 alpha;
    f32 halfLength;
    f32 worldX;
    f32 worldY;
    f32 worldZ;
} Overlay83Record;

typedef struct Overlay83Object {
    s16 x;
    s16 y;
    s16 z;
    s16 height;
    u8 pad08[4];
    f32 worldX;
    f32 worldY;
    f32 worldZ;
    s16 halfLength;
    s16 velocityX;
    s16 velocityY;
    s16 velocityZ;
    u8 pad20[3];
    u8 opacity;
    u8 pad24[4];
    void *linkedObject;
    u8 firstRecord;
    u8 lastRecord;
    u8 recordCount;
    u8 pad2F;
    Overlay83Record records[8];
} Overlay83Object;

typedef struct Overlay83SourceState {
    u8 pad00[2];
    s16 opacity;
} Overlay83SourceState;

typedef struct Overlay83Parent {
    u8 pad00[0x0C];
    f32 worldX;
    f32 worldY;
    f32 worldZ;
    u8 pad18[0x4C];
    Overlay83SourceState *sourceState;
} Overlay83Parent;

typedef struct Overlay83LinkedObject {
    u8 pad00[0x18];
    f32 worldX;
    f32 worldY;
    f32 worldZ;
} Overlay83LinkedObject;

extern void pointListRPY(s32 mode, Overlay83Object *object,
                                          f32 *source, f32 *destination);
extern void changeLightIntensity(Overlay83LinkedObject *object,
                                       s32 opacity);

void overlay83Update(Overlay83Parent *parent,
                     Overlay83Object *object,
                     s32 updateRate) {
    s32 activeCount;
    s32 recordIndex;
    Overlay83Record *record;
    Overlay83SourceState *sourceState;
    Overlay83LinkedObject *linkedObject;

    activeCount = object->recordCount;
    sourceState = parent->sourceState;
    if (activeCount != 0) {
        recordIndex = object->firstRecord;
        do {
            record = &object->records[recordIndex];
            recordIndex++;
            if (recordIndex >= 8) {
                recordIndex = 0;
            }
            record->life--;
            if (record->life < 0) {
                object->firstRecord = recordIndex;
                object->recordCount--;
            } else {
                record->alpha = (record->life * 0x24 * object->opacity) >> 8;
            }
        } while (recordIndex != object->lastRecord);
        activeCount = object->recordCount;
    }

    object->opacity = sourceState->opacity;
    if (activeCount < 8 && object->opacity != 0) {
        record = &object->records[object->lastRecord];
        object->recordCount = activeCount + 1;
        object->lastRecord++;
        if (object->lastRecord >= 8) {
            object->lastRecord = 0;
        }
        record->x = object->x;
        record->y = object->y;
        record->z = object->z;
        record->life = 7;
        record->alpha = (record->life * 0x24 * object->opacity) >> 8;
        record->halfLength = object->halfLength;
        record->worldX = object->worldX;
        record->worldY = object->worldY;
        record->worldZ = object->worldZ;
    }

    object->x += object->velocityX * updateRate;
    object->y += object->velocityY * updateRate;
    object->z += object->velocityZ * updateRate;
    object->worldY = object->height;
    object->worldX = 0.0f;
    object->worldZ = 0.0f;
    pointListRPY(1, object, &object->worldX, &object->worldX);
    linkedObject = object->linkedObject;
    object->worldX += parent->worldX;
    object->worldY += parent->worldY;
    object->worldZ += parent->worldZ;

    if (linkedObject != 0) {
        linkedObject->worldX = object->worldX;
        linkedObject->worldY = object->worldY;
        linkedObject->worldZ = object->worldZ;
        changeLightIntensity(linkedObject, object->opacity);
    }
}
