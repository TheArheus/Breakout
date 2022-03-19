#include <memory>
#include <stdio.h>
#include <fstream>
#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "display.h"
#include "entity.h"
#include "entity.cpp"

bool is_running;
bool IsDebug = false;
bool StartGame = false;

i32 PreviousFrameTime = 0;
r32 DeltaTime = 0;
r32 TimeForFrame = 0;
r32 dtForFrame = 0;

internal void
CreateLevel(world* World, u32 NumOfCols, u32 NumOfRows)
{
    v2 Start = V2(80, 150);

    i32 LevelWidth  = 1000;
    i32 LevelHeight = 400;

    i32 EntityWidth  = LevelWidth  / NumOfRows;
    i32 EntityHeight = LevelHeight / NumOfCols;

    for(u32 Y = 0;
        Y < NumOfRows;
        ++Y)
    {
        for(u32 X = 0;
            X < NumOfCols;
            ++X)
        {
            v2 Position = Start + V2(X * EntityWidth, Y * EntityHeight);
            CreateEntity(World, Position, V2(0, 0), EntityWidth, EntityHeight, EntityType_Structure);
        }
    }
}

internal void 
setup(world* World)
{

    ColorBuffer->Memory = (u32*)malloc(sizeof(u32)*ColorBuffer->Width*ColorBuffer->Height);

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, 
                                SDL_TEXTUREACCESS_STREAMING, 
                                ColorBuffer->Width, ColorBuffer->Height);

    CreateLevel(World, 11, 8);

    v2 PlayerPosition = V2(ColorBuffer->Width / 2, ColorBuffer->Height - 150);
    CreateEntity(World, PlayerPosition - V2(50, 5), V2(0, 0), 100, 10, EntityType_Player);
    CreateEntity(World, PlayerPosition - V2(5, 15), V2(0, 0), 10, 10, EntityType_Ball);
}

internal void 
process_input(world* World)
{
    SDL_Event event;
    SDL_PollEvent(&event);

    entity* Player = GetEntityByType(World, EntityType_Player);
    entity* Ball = GetEntityByType(World, EntityType_Ball);

    switch(event.type)
    {
        case SDL_QUIT:
            is_running = false;
            break;
        case SDL_KEYDOWN:
            if(event.key.keysym.sym == SDLK_ESCAPE) is_running = false;
            if(event.key.keysym.sym == SDLK_r) IsDebug = !IsDebug;
            if(event.key.keysym.sym == SDLK_SPACE)
            {
                StartGame = !StartGame;
                if(StartGame)
                {
                    if((Ball->Component->dP.x == 0) && (Ball->Component->dP.y == 0))
                    Ball->Component->dP = V2(0, -160);
                }
            }

            if(Player)
            {
                if(event.key.keysym.sym == SDLK_a) 
                {
                    Player->Component->P += V2(-20, 0);
                    StartGame = !StartGame;
                    if(StartGame)
                    {
                        if((Ball->Component->dP.x == 0) && (Ball->Component->dP.y == 0))
                        Ball->Component->dP = V2(-160, -160);
                    }
                }
                if(event.key.keysym.sym == SDLK_d)
                {
                    Player->Component->P += V2(20, 0);
                    StartGame = !StartGame;
                    if(StartGame)
                    {
                        if((Ball->Component->dP.x == 0) && (Ball->Component->dP.y == 0))
                        Ball->Component->dP = V2(160, -160);
                    }
                }
            }
            break;
        case SDL_KEYUP:
            break;
    }
}

internal void 
update(world* World)
{
    dtForFrame += TimeForFrame;
    int TimeToWait = FRAME_TARGET_TIME - (SDL_GetTicks() + PreviousFrameTime);

    if(TimeToWait > 0 && (TimeToWait <= FRAME_TARGET_TIME))
    {
        SDL_Delay(TimeToWait);
    }

    DeltaTime = (SDL_GetTicks() - PreviousFrameTime) / 1000.0f;
    //if (DeltaTime > TimeForFrame) DeltaTime = TimeForFrame;
    PreviousFrameTime = SDL_GetTicks();

    UpdateEntities(World, DeltaTime);
}

internal void 
render(world* World)
{
    RenderColorBuffer();
    ClearColorBuffer(ColorBuffer, 0xFF16161d);//0xFF056263);

    entity_storage* StorageToUpdate = World->EntityStorage;
    for(u32 EntityIndex = 0;
        EntityIndex < StorageToUpdate->EntityCount;
        ++EntityIndex)
    {
        entity* Entity = StorageToUpdate->Entities + EntityIndex;

        v2 Start  = Entity->Component->P;
        v2 Width  = Entity->Component->Width  * V2(1, 0);
        v2 Height = Entity->Component->Height * V2(0, 1);

        switch(Entity->Component->Type)
        {
            case EntityType_Player:
            {
                DrawRotRect(ColorBuffer, Start, Width, Height, 0xFF0000FF, nullptr);
            } break;

            case EntityType_Ball:
            {
                DrawRotRect(ColorBuffer, Start, Width, Height, 0xFFFF0000, nullptr);
            } break;

            case EntityType_Structure:
            {
                v2 DebugStart = Start;
                //DrawRect(ColorBuffer, DebugStart - 3, DebugStart + 3, 0xFFFFFF00);
                //DrawRect(ColorBuffer, DebugStart + Width - 3, DebugStart + Width + 3, 0xFFFFFF00);
                //DrawRect(ColorBuffer, DebugStart + Height - 3, DebugStart + Height + 3, 0xFFFFFF00);
                //DrawRect(ColorBuffer, DebugStart + Width + Height - 3, DebugStart + Width + Height + 3, 0xFFFFFF00);
                DrawRotRect(ColorBuffer, Start, Width, Height, 0xFFFFFFFF, nullptr);
            } break;
        }
    }

    SDL_RenderPresent(renderer);
}

int 
main(int argc, char** argv)
{
    is_running = InitWindow();

    world* World = (world*)malloc(sizeof(world));
    World->EntityStorage = (entity_storage*)calloc(1, sizeof(entity_storage));
    World->RemovedEntityStorage = (entity_storage*)calloc(1, sizeof(entity_storage));

    setup(World);

    while(is_running)
    {
        process_input(World);
        update(World);
        render(World);
    }

    DestroyWindow();

    return 0;
}
