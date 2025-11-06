# Voxel Plugin Console Commands Reference

This document lists all console commands and console variables (CVars) available in the Voxel Plugin.

## Table of Contents
- [Collision & Physics](#collision--physics)
- [Renderer](#renderer)
- [Data Management](#data-management)
- [Debug & Visualization](#debug--visualization)
- [Threading](#threading)
- [Mesher](#mesher)
- [Materials](#materials)
- [Textures](#textures)
- [Tools](#tools)
- [Foliage](#foliage)
- [General](#general)

---

## Collision & Physics

### Console Variables

#### `voxel.collision.LogCookingTimes`
- **Type**: Int (0/1)
- **Default**: 0
- **Description**: If true, will log the time it took to cook the voxel meshes collisions
- **Usage**: `voxel.collision.LogCookingTimes 1`

### Console Commands

#### `voxel.collision.LogTotalCookingTime`
- **Description**: Log the accumulated total spent computing collision
- **Usage**: `voxel.collision.LogTotalCookingTime`
- **See Also**: `voxel.collision.ClearTotalCookingTime`

#### `voxel.collision.ClearTotalCookingTime`
- **Description**: Clear the accumulated total spent computing collision
- **Usage**: `voxel.collision.ClearTotalCookingTime`
- **See Also**: `voxel.collision.LogTotalCookingTime`

---

## Renderer

### Console Variables

#### `voxel.renderer.ShowUpdatedChunks`
- **Type**: Int (0/1)
- **Default**: 0
- **Description**: If true, will show the chunks recently updated
- **Usage**: `voxel.renderer.ShowUpdatedChunks 1`

#### `voxel.renderer.ShowRenderChunks`
- **Type**: Int (0/1)
- **Default**: 0
- **Description**: If true, will show the render chunks
- **Usage**: `voxel.renderer.ShowRenderChunks 1`

#### `voxel.renderer.ShowCollisionsUpdates`
- **Type**: Int (0/1)
- **Default**: 0
- **Description**: If true, will show the chunks that finished updating collisions
- **Usage**: `voxel.renderer.ShowCollisionsUpdates 1`

#### `voxel.renderer.ShowStaticMeshComponents`
- **Type**: Int (0/1)
- **Default**: 0
- **Description**: Will show the proc meshes static mesh components used for static lighting
- **Usage**: `voxel.renderer.ShowStaticMeshComponents 1`

#### `voxel.renderer.ShowChunksEmptyStates`
- **Type**: Int (0/1)
- **Default**: 0
- **Description**: If true, will show updated chunks empty state, only if non-empty. Use ShowAllChunksEmptyStates to show empty too.
- **Usage**: `voxel.renderer.ShowChunksEmptyStates 1`

#### `voxel.renderer.ShowAllChunksEmptyStates`
- **Type**: Int (0/1)
- **Default**: 0
- **Description**: If true, will show updated chunks empty state, both empty and non-empty. Use ShowChunksEmptyStates to only show non-empty ones
- **Usage**: `voxel.renderer.ShowAllChunksEmptyStates 1`

### Console Commands

#### `voxel.renderer.ClearChunksEmptyStates`
- **Description**: Clear the empty states debug
- **Usage**: `voxel.renderer.ClearChunksEmptyStates`

#### `voxel.renderer.UpdateAll`
- **Description**: Update all the chunks in all the voxel world in the scene
- **Usage**: `voxel.renderer.UpdateAll`

#### `voxel.renderer.ForceLODUpdate`
- **Description**: Update the LODs
- **Usage**: `voxel.renderer.ForceLODUpdate`

#### `voxel.renderer.UpdateStaticMeshComponents`
- **Description**: Will update all the proc meshes static mesh components used for static lighting
- **Usage**: `voxel.renderer.UpdateStaticMeshComponents`

#### `voxel.renderer.ShowCollisionAndNavmeshDebug [0|1]`
- **Description**: If true, will show chunks used for collisions/navmesh and will color all chunks according to their usage
- **Usage**:
  - `voxel.renderer.ShowCollisionAndNavmeshDebug` (toggles)
  - `voxel.renderer.ShowCollisionAndNavmeshDebug 0` (disable)
  - `voxel.renderer.ShowCollisionAndNavmeshDebug 1` (enable)
- **Color Legend**:
  - **Yellow**: Collisions and Navmesh enabled
  - **Blue**: Collisions only
  - **Green**: Navmesh only
  - **White**: Neither

---

## Data Management

### Console Commands

#### `voxel.data.CacheAllValues`
- **Description**: Cache all values
- **Usage**: `voxel.data.CacheAllValues`

#### `voxel.data.CacheAllMaterials`
- **Description**: Cache all materials
- **Usage**: `voxel.data.CacheAllMaterials`

#### `voxel.data.ClearAllCachedValues`
- **Description**: Clear all cached values
- **Usage**: `voxel.data.ClearAllCachedValues`

#### `voxel.data.ClearAllCachedMaterials`
- **Description**: Clear all cached materials
- **Usage**: `voxel.data.ClearAllCachedMaterials`

#### `voxel.data.CheckForSingleValues`
- **Description**: Check if values in a chunk are all the same, and if so only store one
- **Usage**: `voxel.data.CheckForSingleValues`

#### `voxel.data.CheckForSingleMaterials`
- **Description**: Check if materials in a chunk are all the same, and if so only store one
- **Usage**: `voxel.data.CheckForSingleMaterials`

#### `voxel.data.RoundVoxels`
- **Description**: Round all voxels that do not impact the surface nor the normals
- **Usage**: `voxel.data.RoundVoxels`

#### `voxel.data.ClearUnusedMaterials`
- **Description**: Will clear all materials that do not affect the surface to improve compression
- **Usage**: `voxel.data.ClearUnusedMaterials`

#### `voxel.data.CompressIntoHeightmap`
- **Description**: Update the heightmap to match the voxel world data
- **Usage**: `voxel.data.CompressIntoHeightmap`

#### `voxel.data.RoundToGenerator`
- **Description**: Set the voxels back to the generator value if all the voxels in a radius of 2 have the same sign as the generator
- **Usage**: `voxel.data.RoundToGenerator`

### Console Variables (Data Debug)

#### `voxel.data.ShowValuesState`
- **Type**: Int (0/1)
- **Default**: 0
- **Description**: If true, will show the values data chunks and their status (cached/created...)
- **Usage**: `voxel.data.ShowValuesState 1`
- **Color Legend**:
  - **Red**: Dirty
  - **Yellow**: Cached
  - **Green**: Single Item Stored
  - **Blue**: Single Item Stored - Dirty

#### `voxel.data.ShowMaterialsState`
- **Type**: Int (0/1)
- **Default**: 0
- **Description**: If true, will show the materials data chunks and their status (cached/created...)
- **Usage**: `voxel.data.ShowMaterialsState 1`
- **Color Legend**: Same as ShowValuesState

#### `voxel.data.ShowDirtyValues`
- **Type**: Int (0/1)
- **Default**: 0
- **Description**: If true, will show the data chunks with dirty values
- **Usage**: `voxel.data.ShowDirtyValues 1`

#### `voxel.data.ShowDirtyMaterials`
- **Type**: Int (0/1)
- **Default**: 0
- **Description**: If true, will show the data chunks with dirty materials
- **Usage**: `voxel.data.ShowDirtyMaterials 1`

#### `voxel.data.ShowDirtyVoxels`
- **Type**: Int (0/1)
- **Default**: 0
- **Description**: If true, will show every dirty voxel in the scene
- **Usage**: `voxel.data.ShowDirtyVoxels 1`
- **Color Legend**:
  - **Red**: Non-empty voxels
  - **Blue**: Empty voxels

#### `voxel.data.ShowPlaceableItemsChunks`
- **Type**: Int (0/1)
- **Default**: 0
- **Description**: If true, will show every chunk that has a placeable item
- **Usage**: `voxel.data.ShowPlaceableItemsChunks 1`

---

## Debug & Visualization

### Console Variables

#### `voxel.FreezeDebug`
- **Type**: Int (0/1)
- **Default**: 0
- **Description**: If true, won't clear previous frames boxes
- **Usage**: `voxel.FreezeDebug 1`

#### `voxel.debug.DrawTime`
- **Type**: Int
- **Default**: 1
- **Description**: Draw time will be multiplied by this
- **Usage**: `voxel.debug.DrawTime 2`

#### `voxel.ShowWorldBounds`
- **Type**: Int (0/1)
- **Default**: 0
- **Description**: If true, will show the world bounds
- **Usage**: `voxel.ShowWorldBounds 1`

#### `voxel.ShowInvokers`
- **Type**: Int (0/1)
- **Default**: 0
- **Description**: If true, will show the voxel invokers
- **Usage**: `voxel.ShowInvokers 1`
- **Color Legend**:
  - **Green**: Local Invokers / Navmesh Bounds
  - **Silver**: Remote Invokers
  - **Red**: LOD Bounds
  - **Blue**: Collisions Bounds

### Console Commands

#### `voxel.RecomputeComponentPositions`
- **Description**: Recompute the positions of all the components in all the voxel world in the scene
- **Usage**: `voxel.RecomputeComponentPositions`

#### `voxel.RebaseOntoCamera`
- **Description**: Call SetWorldOriginLocation so that the camera is at 0 0 0
- **Usage**: `voxel.RebaseOntoCamera`

#### `voxel.LogMemoryStats`
- **Description**: Log memory statistics
- **Usage**: `voxel.LogMemoryStats`

#### `voxel.debug.LogSecondsPerCycles`
- **Description**: Log the platform's seconds per cycle value
- **Usage**: `voxel.debug.LogSecondsPerCycles`

---

## Threading

### Console Variables

#### `voxel.threading.NumThreads`
- **Type**: Int
- **Default**: 2
- **Description**: The number of threads to use to process voxel tasks
- **Usage**: `voxel.threading.NumThreads 4`

#### `voxel.threading.ThreadPriority`
- **Type**: Int (0-6)
- **Default**: 2
- **Description**: Thread priority level
- **Usage**: `voxel.threading.ThreadPriority 3`
- **Values**:
  - 0: Normal
  - 1: AboveNormal
  - 2: BelowNormal
  - 3: Highest
  - 4: Lowest
  - 5: SlightlyBelowNormal
  - 6: TimeCritical

#### `voxel.threading.PriorityDuration`
- **Type**: Float
- **Default**: 0.5
- **Description**: Task priorities will be recomputed with the new invoker position every PriorityDuration seconds
- **Usage**: `voxel.threading.PriorityDuration 1.0`

### Console Commands

#### `voxel.threading.AbandonAllTasks`
- **Description**: Will abandon all active tasks
- **Usage**: `voxel.threading.AbandonAllTasks`

#### `voxel.threading.LogStats`
- **Description**: Log thread pool statistics
- **Usage**: `voxel.threading.LogStats`

#### `voxel.threading.ClearStats`
- **Description**: Clear thread pool statistics
- **Usage**: `voxel.threading.ClearStats`

---

## Mesher

### Console Variables

#### `voxel.mesher.DoNotSkipEmptyChunks`
- **Type**: Int (0/1)
- **Default**: 0
- **Description**: If true, all chunks will be computed (even empty ones)
- **Usage**: `voxel.mesher.DoNotSkipEmptyChunks 1`

### Console Commands

#### `voxel.mesher.ClearStats`
- **Description**: Clear the mesher stats
- **Usage**: `voxel.mesher.ClearStats`

#### `voxel.mesher.PrintStats`
- **Description**: Print the mesher stats
- **Usage**: `voxel.mesher.PrintStats`
- **Output**: Detailed performance breakdown by LOD, including:
  - Total time per LOD
  - Average time per chunk
  - Values/Materials access times
  - Normals/UVs computation time
  - Distance field time
  - Transitions time percentage
  - Per-voxel access times

---

## Materials

### Console Commands

#### `voxel.renderer.ClearMaterialInstancePool`
- **Description**: Clear material instance pool
- **Usage**: `voxel.renderer.ClearMaterialInstancePool`

---

## Textures

### Console Commands

#### `voxel.texture.ClearCache`
- **Description**: Clears the voxel textures memory cache
- **Usage**: `voxel.texture.ClearCache`

#### `voxel.texture.ClearIdCache`
- **Description**: Clears the voxel textures id cache, used for serialization
- **Usage**: `voxel.texture.ClearIdCache`

---

## Tools

### Console Commands

#### `voxel.tools.Freeze`
- **Description**: Toggle freezing/unfreezing of the tool manager
- **Usage**: `voxel.tools.Freeze`

---

## Foliage

### Console Commands

#### `voxel.foliage.RegenerateAll`
- **Description**: Regenerate all foliage that can be regenerated
- **Usage**: `voxel.foliage.RegenerateAll`

---

## Texture Pool

### Console Commands

#### `voxel.texturepool.compact`
- **Description**: Reallocate all the entries, reducing fragmentation & saving memory
- **Usage**: `voxel.texturepool.compact`

---

## Multiplayer

### Console Variables

#### `voxel.multiplayer.ShowSyncedChunks`
- **Type**: Int (0/1)
- **Default**: 0
- **Description**: If true, will show the synced chunks
- **Usage**: `voxel.multiplayer.ShowSyncedChunks 1`

---

## Performance Notes

### Stat Groups

The plugin exposes several stat groups for profiling:
- `STATGROUP_VoxelCounters`: General counters (tasks, collision cubes, etc.)
- Voxel tasks are categorized by type:
  - ChunksMeshing
  - VisibleChunksMeshing
  - CollisionsChunksMeshing
  - VisibleCollisionsChunksMeshing
  - MeshMerge
  - FoliageBuild
  - HISMBuild
  - AsyncEditFunctions
  - RenderOctree
  - CollisionCooking

### On-Screen Task Counter

The plugin automatically displays on-screen the number of active voxel tasks when there are pending tasks, showing:
- Total voxel tasks
- Mesher tasks
- Foliage tasks
- Edit tasks
- LOD tasks
- Collision tasks
- Number of threads

---

## Tips

### Debugging Workflow

1. **Enable visual debugging**:
   ```
   voxel.renderer.ShowUpdatedChunks 1
   voxel.renderer.ShowRenderChunks 1
   voxel.ShowInvokers 1
   ```

2. **Freeze debug visualization** to analyze specific frames:
   ```
   voxel.FreezeDebug 1
   ```

3. **Monitor performance**:
   ```
   voxel.mesher.PrintStats
   voxel.threading.LogStats
   voxel.collision.LogTotalCookingTime
   ```

4. **Optimize memory**:
   ```
   voxel.data.CheckForSingleValues
   voxel.data.CheckForSingleMaterials
   voxel.data.ClearUnusedMaterials
   voxel.texturepool.compact
   voxel.renderer.ClearMaterialInstancePool
   ```

### Data Optimization Commands

To optimize world data after heavy editing:
```
voxel.data.RoundVoxels
voxel.data.ClearUnusedMaterials
voxel.data.CheckForSingleValues
voxel.data.CheckForSingleMaterials
```

### Debug Data State

To visualize what's cached/dirty:
```
voxel.data.ShowValuesState 1
voxel.data.ShowMaterialsState 1
voxel.data.ShowDirtyValues 1
voxel.data.ShowDirtyVoxels 1
```

---

## Related Documentation

- See `CLAUDE.md` for overall project architecture
- See [Voxel Plugin Wiki](https://wiki.voxelplugin.com/) for detailed feature documentation
- Join [Discord](https://discord.voxelplugin.com) for community support

---

**Plugin Version**: Beta-415230fff-2021-06-08 (Voxel Plugin Free)
**Engine Version**: UE 5.6
**Last Updated**: 2025-06-10
