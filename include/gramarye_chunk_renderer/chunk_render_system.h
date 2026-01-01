#ifndef CHUNK_RENDER_SYSTEM_H
#define CHUNK_RENDER_SYSTEM_H

#include "gramarye_renderer/renderer.h"  // Renderer interface
#include "gramarye_renderer/camera.h"  // CameraHandle, AspectFitHandle
#include "tilemap/tilemap.h"  // From gramarye-components
#include "textures/atlas.h"  // Full Atlas API from gramarye-component-functions
#include "tilemap/chunk_render_data.h"  // ChunkRenderData struct
#include "tilemap/chunk_observer.h"  // Observer struct
#include "gramarye_ecs/ecs.h"
#include "gramarye_ecs/entity.h"
#include "gramarye_ecs/component.h"
#include "arena.h"
#include "table.h"
#include "hash/int_coord_hash.h"

// Chunk render system structure
typedef struct ChunkRenderSystem {
    Arena_T arena;
    Table_T chunks;              // IntCoord (chunkX, chunkY) -> ChunkRenderData*
    
    // Observer management
    Observer* observers;          // Array of observers
    size_t observerCount;
    size_t observerCapacity;
    
    // Configuration
    int chunkSize;               // Tiles per chunk (64x64)
    int tileSize;                // Pixels per tile
    int renderRadius;            // Chunks to render (5)
    int simulationRadius;         // Chunks to simulate (10)
    
    // References to game systems
    Tilemap* tilemap;
    Atlas* atlas;
    Renderer* renderer;          // Renderer interface for camera transformations
    
    uint64_t currentFrame;       // Current frame counter
} ChunkRenderSystem;

// Initialize the chunk render system
void ChunkRenderSystem_init(ChunkRenderSystem* system, 
                            Arena_T arena,
                            Tilemap* tilemap, 
                            Atlas* atlas,
                            Renderer* renderer,
                            int tileSize,
                            int chunkSize,
                            int renderRadius,
                            int simulationRadius);

// Add an entity observer (entity must have Position component)
void ChunkRenderSystem_add_entity_observer(ChunkRenderSystem* system, 
                                           ECS* ecs,
                                           EntityId entity,
                                           ComponentTypeId positionTypeId);

// Add a manual observer at specific tile coordinates
void ChunkRenderSystem_add_manual_observer(ChunkRenderSystem* system, int tileX, int tileY);

// Remove an entity observer
void ChunkRenderSystem_remove_entity_observer(ChunkRenderSystem* system, EntityId entity);

// Remove a manual observer at specific tile coordinates
void ChunkRenderSystem_remove_manual_observer(ChunkRenderSystem* system, int tileX, int tileY);

// Forward declaration
typedef struct ChunkManagerSystem ChunkManagerSystem;

// Update chunk loading/unloading based on observers
// If chunkManager is provided, it will be used to check for dirty chunks
// and dirty flags will be cleared after rendering
void ChunkRenderSystem_update(ChunkRenderSystem* system, 
                              ECS* ecs,
                              ComponentTypeId positionTypeId,
                              ChunkManagerSystem* chunkManager);

// Render visible chunks
void ChunkRenderSystem_render(ChunkRenderSystem* system,
                              ECS* ecs,
                              ComponentTypeId positionTypeId,
                              CameraHandle camera, 
                              AspectFitHandle aspectFit);

// Convert tile coordinates to chunk coordinates
void ChunkRenderSystem_get_chunk_coord(ChunkRenderSystem* system, 
                                       int tileX, int tileY, 
                                       int* outChunkX, int* outChunkY);

// Convert chunk coordinates and local coordinates to tile coordinates
void ChunkRenderSystem_get_tile_coord(ChunkRenderSystem* system,
                                      int chunkX, int chunkY,
                                      int localX, int localY,
                                      int* outTileX, int* outTileY);

// Handle click to tile conversion (returns true if click was on tilemap)
bool ChunkRenderSystem_handle_click(ChunkRenderSystem* system,
                                    RenderVector2 mousePos,
                                    CameraHandle camera,
                                    AspectFitHandle aspectFit,
                                    int* outTileX,
                                    int* outTileY);

// Mark a chunk as dirty (needs re-rendering)
// NOTE: This is deprecated - use ChunkManagerSystem_mark_chunk_dirty instead
// Kept for backwards compatibility but should not be used in new code
void ChunkRenderSystem_mark_chunk_dirty(ChunkRenderSystem* system, int tileX, int tileY);

// Cleanup resources
void ChunkRenderSystem_cleanup(ChunkRenderSystem* system);

#endif // CHUNK_RENDER_SYSTEM_H

