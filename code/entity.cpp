
void 
CreateEntity(world* World, v2 Position, v2 Velocity, u32 Width, u32 Height, entity_type Type)
{
    entity_storage* StorageToUpdate  = World->EntityStorage;
    entity_storage* StorageToGetFrom = World->RemovedEntityStorage;

    if(StorageToUpdate->EntityCount == 0)
    {
        StorageToUpdate->Entities = (entity*)malloc(sizeof(entity));
    }

    // TODO: Fetching from the "StorageToGetFrom" 
    // should be better that this I believe?
    entity NewEntity = {};
    u32 NewEntityID;
    if(StorageToGetFrom->EntityCount > 0)
    {
        NewEntity = StorageToGetFrom->Entities[--StorageToGetFrom->EntityCount];
    }
    else
    {
        NewEntityID = StorageToUpdate->EntityCount;
        entity_component* NewEntityComponent = (entity_component*)calloc(1, sizeof(entity_component));

        NewEntityComponent->P  = Position;
        NewEntityComponent->dP = Velocity;
        NewEntityComponent->Width  = Width;
        NewEntityComponent->Height = Height;
        NewEntityComponent->Type   = Type;

        NewEntity.Component = NewEntityComponent;
    }

    NewEntity.Component->StorageIndex = StorageToUpdate->EntityCount;

    NewEntity.ID = NewEntityID;

    StorageToUpdate->Entities[StorageToUpdate->EntityCount++] = NewEntity;

    if (NewEntity.Component->StorageIndex >= (StorageToUpdate->EntityCount - 1))
    {
        // NOTE: This is a bad thing here? 
        // I should think about a better solution here
        StorageToUpdate->Entities = (entity*)realloc(StorageToUpdate->Entities, sizeof(entity) * StorageToUpdate->EntityCount * 2);
    }
}

// NOTE: Do I really want to remove Entities by their ID
// Maybe create another function to remove entities directly
void
RemoveEntityByID(world* World, u32 EntityID)
{
    entity_storage* StorageInUse   = World->EntityStorage;
    entity_storage* StorageToStore = World->RemovedEntityStorage;

    if(StorageToStore->EntityCount == 0)
    {
        StorageToStore->Entities = (entity*)malloc(sizeof(entity));
    }

    entity EntityToRemove;
    entity EntityToMove = StorageInUse->Entities[--StorageInUse->EntityCount];

    for(u32 EntityIndex = 0;
        EntityIndex < StorageInUse->EntityCount;
        ++EntityIndex)
    {
        entity Entity = StorageInUse->Entities[EntityIndex];
        if(Entity.ID == EntityID)
        {
            EntityToRemove = Entity;
            break;
        }
    }

    EntityToMove.Component->StorageIndex = EntityToRemove.Component->StorageIndex;
    EntityToRemove.Component->StorageIndex = StorageToStore->EntityCount;

    StorageToStore->Entities[StorageToStore->EntityCount++] = EntityToRemove;
    StorageInUse->Entities[EntityToMove.Component->StorageIndex] = EntityToMove;

    if(EntityToRemove.Component->StorageIndex >= StorageToStore->EntityCount)
    {
        // NOTE: This is a bad thing here? 
        // I should think about a better solution here
        StorageToStore->Entities = (entity*)realloc(StorageToStore->Entities, sizeof(entity) * StorageToStore->EntityCount * 2);
    }
}

entity* 
GetEntityByType(world* World, entity_type Type)
{
    entity* Result = 0;

    entity_storage* StorageToUse = World->EntityStorage;
    for(u32 EntityIndex = 0;
        EntityIndex < StorageToUse->EntityCount;
        ++EntityIndex)
    {
        entity* EntityFound = StorageToUse->Entities + EntityIndex;
        if(EntityFound->Component->Type == Type)
        {
            Result = EntityFound;
        }
    }
    return Result;
}

struct collision_result
{
    entity* CollidedEntity;
    b32 AreCollided;
};

collision_result 
CheckForCollision(entity_storage* Storage, entity* A)
{
    collision_result Result;
    Result.AreCollided = false;
    Result.CollidedEntity = nullptr;

    for(u32 EntityIndex = 0;
        EntityIndex < Storage->EntityCount;
        ++EntityIndex)
    {
        entity* B = Storage->Entities + EntityIndex;
        if(A != B)
        {
            v2 PositionA = A->Component->P;
            i32 WidthA = A->Component->Width;
            i32 HeightA = A->Component->Height;

            v2 PositionB = B->Component->P;
            i32 WidthB = B->Component->Width;
            i32 HeightB = B->Component->Height;
            if (((PositionA.x) < (PositionB.x + WidthB)) &&
                ((PositionA.y) < (PositionB.y + HeightB)) &&
                ((PositionA.x + WidthA) > (PositionB.x)) &&
                ((PositionA.y + HeightA) > (PositionB.y)))
            {
                Result.CollidedEntity = B;
                Result.AreCollided = true;
            }
        }
    }
    return Result;
}

struct collision_resolution_result
{
    v2 Normal;
};

collision_resolution_result
ResolveCollisionBoxBox(entity* A, entity* B)
{
    collision_resolution_result Result = {};

    std::vector<v2> VerticesOfA;
    std::vector<v2> VerticesOfB;

    VerticesOfA.push_back(A->Component->P);
    VerticesOfA.push_back(A->Component->P + V2(A->Component->Width, 0));
    VerticesOfA.push_back(A->Component->P + V2(A->Component->Width, A->Component->Height));
    VerticesOfA.push_back(A->Component->P + V2(0, A->Component->Height));

    VerticesOfB.push_back(B->Component->P);
    VerticesOfB.push_back(B->Component->P + V2(B->Component->Width, 0));
    VerticesOfB.push_back(B->Component->P + V2(B->Component->Width, B->Component->Height));
    VerticesOfB.push_back(B->Component->P + V2(0, B->Component->Height));

    r32 Separation = -FLT_MAX;
    for(u32 IndexA = 0;
        IndexA < VerticesOfA.size();
        ++IndexA)
    {
        v2 SideOfA_A = VerticesOfA.data()[IndexA];
        v2 SideOfA_B = VerticesOfA.data()[(IndexA + 1) % VerticesOfA.size()];

        v2 NormalOfA = Normal(SideOfA_B - SideOfA_A);

        r32 MinimumSeparation = FLT_MAX;
        v2 NormalOfB;
        for(u32 IndexB = 0;
            IndexB < VerticesOfB.size();
            ++IndexB)
        {
            v2 SideOfB_A = VerticesOfB.data()[IndexB];

            r32 Projection = Inner(NormalOfA, SideOfB_A - SideOfA_A);
            if(Projection < MinimumSeparation)
            {
                MinimumSeparation = Projection;
            }
        }
        if(MinimumSeparation > Separation) 
        {
            Separation = MinimumSeparation;
            Result.Normal = NormalOfA;
        }
    }

    return Result;
}

void
UpdateEntities(world* World, r32 DeltaTime)
{
    entity_storage* StorageToUpdate = World->EntityStorage;
    for(u32 EntityIndex = 0;
        EntityIndex < StorageToUpdate->EntityCount;
        ++EntityIndex)
    {
        entity* Entity = StorageToUpdate->Entities + EntityIndex;

        Entity->Component->P += Entity->Component->dP * DeltaTime;

        if(Entity->Component->Type == EntityType_Player)
        {
            if(Entity->Component->P.x < 0)
            {
                Entity->Component->P.x = 10;
            }
            else if((Entity->Component->P.x + Entity->Component->Width) > ColorBuffer->Width)
            {
                Entity->Component->P.x = ColorBuffer->Width - Entity->Component->Width - 10;
            }
        }

        if(Entity->Component->Type == EntityType_Ball)
        {
            if(Entity->Component->P.x < 0)
            {
                v2 WallNormal = V2(-1, 0);
                Entity->Component->dP = Entity->Component->dP - 2.0f * Inner(Entity->Component->dP, WallNormal) * WallNormal;
            }
            else if((Entity->Component->P.x + Entity->Component->Width) > ColorBuffer->Width)
            {
                v2 WallNormal = V2( 1, 0);
                Entity->Component->dP = Entity->Component->dP - 2.0f * Inner(Entity->Component->dP, WallNormal) * WallNormal;
            }
            else if(Entity->Component->P.y < 0)
            {
                v2 WallNormal = V2( 0, 1);
                Entity->Component->dP = Entity->Component->dP - 2.0f * Inner(Entity->Component->dP, WallNormal) * WallNormal;
            }
        }

        // TODO: Better collision detection here
        collision_result CollisionResult = CheckForCollision(StorageToUpdate, Entity);
        if(CollisionResult.AreCollided)
        {
            if((Entity->Component->Type == EntityType_Ball) && (CollisionResult.CollidedEntity->Component->Type == EntityType_Structure))
            {
                collision_resolution_result ResolutionResult = ResolveCollisionBoxBox(CollisionResult.CollidedEntity, Entity);
                v2 WallNormal = ResolutionResult.Normal;
                Entity->Component->dP = Entity->Component->dP - 2.0f * Inner(Entity->Component->dP, WallNormal) * WallNormal;
            }

            if((Entity->Component->Type == EntityType_Ball) && (CollisionResult.CollidedEntity->Component->Type == EntityType_Player))
            {
#if 0
                // NOTE: Those checks works that they can't check for 
                // left and right collision.
                // Do I need these kind of collisions
                if((Entity->Component->P.y + Entity->Component->Height) >= CollisionResult.CollidedEntity->Component->P.y)
                {
                    // From top
                    v2 WallNormal = V2(0, 1);
                    Entity->Component->dP = Entity->Component->dP - 2.0f * Inner(Entity->Component->dP, WallNormal) * WallNormal;
                }
                else if(Entity->Component->P.x <= (CollisionResult.CollidedEntity->Component->P.x + CollisionResult.CollidedEntity->Component->Width))
                {
                    // From right
                    v2 WallNormal = V2(1, 0);
                    Entity->Component->dP = Entity->Component->dP - 2.0f * Inner(Entity->Component->dP, WallNormal) * WallNormal;
                }
                else if(Entity->Component->P.x >= CollisionResult.CollidedEntity->Component->P.x)
                {
                    // From left
                    v2 WallNormal = V2(-1, 0);
                    Entity->Component->dP = Entity->Component->dP - 2.0f * Inner(Entity->Component->dP, WallNormal) * WallNormal;
                }
#else
                collision_resolution_result ResolutionResult = ResolveCollisionBoxBox(CollisionResult.CollidedEntity, Entity);
                v2 WallNormal = ResolutionResult.Normal;
                Entity->Component->dP = Entity->Component->dP - 2.0f * Inner(Entity->Component->dP, WallNormal) * WallNormal;
#endif
            }

            if (CollisionResult.CollidedEntity->Component->Type == EntityType_Structure)
            {
                RemoveEntityByID(World, CollisionResult.CollidedEntity->ID);
            }
        }
    }
}
