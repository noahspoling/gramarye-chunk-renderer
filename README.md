# gramarye-chunk-renderer

Chunk-based rendering system for the Gramarye game engine. Provides efficient rendering of large tilemaps by dividing them into chunks that are rendered to textures and cached.

## Overview

gramarye-chunk-renderer manages chunk-based rendering where tilemaps are divided into chunks (e.g., 64x64 tiles). Each chunk is rendered to a render texture once and cached, only re-rendering when the chunk data changes. This provides efficient rendering of large worlds.

## Features

- Chunk-based rendering with configurable chunk size
- Observer-based chunk loading (entity or manual observers)
- Automatic chunk loading/unloading based on render and simulation radii
- Dirty chunk tracking for efficient updates
- Renderer interface abstraction (not raylib-specific)
- Optional integration with gramarye-chunk-controller for dirty tracking

## Architecture

### Observer System

The renderer uses observers to determine which chunks should be loaded and rendered:

- **Entity Observers**: Track entities with Position components. Chunks within render radius of these entities are loaded.
- **Manual Observers**: Track fixed tile coordinates. Useful for static cameras or debug views.

Observers can be added/removed at runtime. The system automatically manages observer capacity with dynamic resizing.

### Chunk Rendering

Each chunk is rendered to a render texture using the renderer interface:

1. **Chunk Creation**: When a chunk is first needed, a render texture is created
2. **Tile Rendering**: All tiles in the chunk are rendered to the texture using the Atlas API
3. **Caching**: The rendered texture is cached and reused until the chunk is marked dirty
4. **Re-rendering**: Dirty chunks are re-rendered when needed (either by local dirty flag or ChunkManagerSystem)

### Coordinate System

The system handles coordinate transformations:

- **Tile to Chunk**: Converts tile coordinates to chunk coordinates using floor division (handles negative coordinates correctly)
- **Chunk to Tile**: Converts chunk + local coordinates back to tile coordinates
- **World to Screen**: Uses renderer interface for camera transformations
- **Screen to World**: Handles click-to-tile conversion with proper coordinate space handling

Negative coordinates are handled correctly using floor division:
- Positive: `chunkX = tileX / chunkSize`
- Negative: `chunkX = (tileX - chunkSize + 1) / chunkSize`

### Optional Chunk Manager Integration

The renderer can optionally integrate with gramarye-chunk-controller:

- If `ChunkManagerSystem` is provided to `ChunkRenderSystem_update()`, it checks the manager for dirty chunks
- If manager is NULL, falls back to local dirty tracking
- After rendering all dirty chunks, clears dirty flags in the manager

This allows decoupled tile updates (via chunk-controller) and rendering (via chunk-renderer).

### Include Order

The header uses forward declarations to avoid hard dependencies. When including:

1. Include full API headers from gramarye-component-functions first
2. The include path order ensures component-functions headers come first
3. ChunkManagerSystem header is optional (checked via `__has_include`)

Example:
```c
#include "core/position.h"  // Position_get ECS function (includes struct)
#include "textures/atlas.h"  // Atlas_getRect (full API with raylib types)
#include "tilemap/tilemap.h"  // Tilemap_get_tile
```

## Dependencies

- **gramarye-libcore**: For Table, Arena, IntCoord hashing
- **gramarye-ecs**: For ECS, EntityId, ComponentTypeId
- **gramarye-components**: For ChunkRenderData, Observer structs, Tilemap
- **gramarye-component-functions**: For Position_get, Atlas_getRect
- **gramarye-renderer-interface**: For Renderer, CameraHandle, AspectFitHandle
- **gramarye-chunk-controller**: Optional, for dirty chunk tracking

## Usage

### Basic Setup

```c
#include "gramarye_chunk_renderer/chunk_render_system.h"

// Initialize the chunk render system
ChunkRenderSystem chunkRenderer;
ChunkRenderSystem_init(&chunkRenderer, arena, tilemap, atlas, renderer,
                       16,  // tile size (pixels)
                       64,  // chunk size (64x64 tiles)
                       5,   // render radius (chunks)
                       10); // simulation radius (chunks)

// Add an entity observer (entity must have Position component)
ChunkRenderSystem_add_entity_observer(&chunkRenderer, ecs, playerEntity, positionTypeId);

// Or add a manual observer at fixed coordinates
ChunkRenderSystem_add_manual_observer(&chunkRenderer, 100, 200);
```

### Update Loop

```c
// Update chunk loading/unloading and render dirty chunks
// Pass NULL for chunkManager to use local dirty tracking
ChunkRenderSystem_update(&chunkRenderer, ecs, positionTypeId, NULL);

// Or use chunk manager for dirty tracking
ChunkRenderSystem_update(&chunkRenderer, ecs, positionTypeId, &chunkManager);
```

### Rendering

```c
// Render visible chunks (call after camera is updated)
ChunkRenderSystem_render(&chunkRenderer, ecs, positionTypeId, &camera, &aspectFit);
```

### Click Handling

```c
// Convert screen click to tile coordinates
int tileX, tileY;
RenderVector2 mousePos = {mouseX, mouseY};
if (ChunkRenderSystem_handle_click(&chunkRenderer, mousePos, &camera, &aspectFit, &tileX, &tileY)) {
    // Click was on tilemap, tileX and tileY are set
}
```

### Coordinate Conversion

```c
// Get chunk coordinates from tile coordinates
int chunkX, chunkY;
ChunkRenderSystem_get_chunk_coord(&chunkRenderer, tileX, tileY, &chunkX, &chunkY);

// Get tile coordinates from chunk + local coordinates
int tileX, tileY;
ChunkRenderSystem_get_tile_coord(&chunkRenderer, chunkX, chunkY, localX, localY, &tileX, &tileY);
```

## Integration

This library is designed to be used as a submodule in game projects. It uses the renderer interface abstraction, so it's not tied to a specific rendering backend (though the Atlas API currently uses raylib types).

### With Chunk Controller

For full integration with tile updates:

1. Use gramarye-chunk-controller to manage tile updates
2. Pass ChunkManagerSystem to `ChunkRenderSystem_update()`
3. The renderer will check the manager for dirty chunks
4. Dirty flags are cleared after rendering

### Without Chunk Controller

The renderer can work standalone:

1. Mark chunks dirty manually: `ChunkRenderSystem_mark_chunk_dirty()`
2. Pass NULL for chunkManager in update
3. Uses local dirty tracking

## Render Radius vs Simulation Radius

- **Render Radius**: Chunks within this radius of observers are rendered to screen
- **Simulation Radius**: Chunks within this radius should be loaded for simulation (future: automatic unloading outside this radius)

Currently, only render radius is fully implemented. Simulation radius is reserved for future chunk unloading logic.

