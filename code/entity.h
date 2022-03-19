#if !defined(entity_h)
#define entity_h

enum entity_type
{
    EntityType_Player,
    EntityType_Ball,
    EntityType_Structure,

    EntityType_Count,
};

// NOTE: I could try here to create
// a second entity storage for removed entity ids
// instead of this thing.
// But Could it be cost more on modern machines?
enum entity_flags
{
    EntityFlag_Placed = (1 << 0),
};

struct entity_component
{
    v2 P;
    v2 dP;

    u32 Width;
    u32 Height;

    b32 Flags;

    i32 StorageIndex;
    
    entity_type Type;
};

struct entity
{
    u32 ID;
    entity_component* Component;
};

struct entity_storage
{
    entity* Entities;
    u32 EntityCount;
};

struct world
{
    entity_storage* EntityStorage;
    entity_storage* RemovedEntityStorage;

    // Should I count entities that is in use?
    // Entities that I got from this storage?
    // But isn't too space consuming?
    entity GlobalEntityStorage[512];
};

#endif
