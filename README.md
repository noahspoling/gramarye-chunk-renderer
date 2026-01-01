# gramarye-chunk-renderer

Chunk-based rendering system for the Gramarye game engine. Provides efficient rendering of large tilemaps by dividing them into chunks that are rendered to textures and cached.

## Overview

gramarye-chunk-renderer manages chunk-based rendering where tilemaps are divided into chunks (e.g., 64x64 tiles). Each chunk is rendered to a render texture once and cached, only re-rendering when the chunk data changes. This provides efficient rendering of large worlds.

## Features

- Chunk-based rendering with configurable chunk size
- Observer-based chunk loading (entity or manual observers)
- Automatic chunk loading/unloading based on render and simulation radii
- Dirty chunk tracking for efficient updates
- Raylib-specific implementation (uses RenderTexture2D)

## Dependencies

- gramarye-libcore (for Table, Arena, IntCoord)
- gramarye-ecs (for ECS, EntityId, ComponentTypeId)
- gramarye-components (for ChunkRenderData, Observer structs)
- gramarye-component-functions (for Position_get, Atlas_getRect)
- raylib (for rendering, provided by parent project)

## Usage

```c
#include "gramarye_chunk_renderer/chunk_render_system.h"

// Initialize the chunk render system
ChunkRenderSystem chunkRenderer;
ChunkRenderSystem_init(&chunkRenderer, arena, tilemap, atlas,
                       16,  // tile size
                       64,  // chunk size (64x64 tiles)
                       5,   // render radius
                       10); // simulation radius

// Add an entity observer (entity must have Position component)
ChunkRenderSystem_add_entity_observer(&chunkRenderer, ecs, playerEntity, positionTypeId);

// Update chunk loading/unloading
ChunkRenderSystem_update(&chunkRenderer, ecs, positionTypeId);

// Render visible chunks
ChunkRenderSystem_render(&chunkRenderer, ecs, positionTypeId, &camera, aspectFit);

// Cleanup
ChunkRenderSystem_cleanup(&chunkRenderer);
```

## Integration

This library is designed to be used as a submodule in game projects. It requires raylib to be available from the parent project.

