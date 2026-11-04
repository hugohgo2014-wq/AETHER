# Roadmap and scope

## Milestone 0 — native foundation (current)
C17 core, plugin ABI/registry, C++20 runtime, EnTT world, JSON project loader, direct APK asset access, Lua boot script, miniaudio, GLES3 procedural showcase, Android APK workflow and C-core smoke tests.

## Milestone 1 — production content pipeline
Texture resources/mipmaps; glTF and GLB import through fastgltf; mesh cache/vertex optimization with meshoptimizer; material serialization, shader variants and async resource handles.

## Milestone 2 — physics and gameplay systems
Box2D and Jolt adapters; fixed-step simulation; transform hierarchy; collision events; navigation through Recast/Detour; animation runtime.

## Milestone 3 — desktop editor
Dockable viewport, scene hierarchy, inspector, asset browser, console, play/pause/step toolbar, gizmos through ImGuizmo, graph editor through imgui-node-editor, undo/redo and project serialization.

## Milestone 4 — high-end renderer and large worlds
bgfx integration; PBR/IBL; shadow maps and cascades; HDR, bloom, color grading, SSAO and temporal AA; GPU instancing; culling and LOD; world partitioning, async streaming, job graph and Tracy profiling.

Unreal/Unity/Godot parity is a multi-year engineering project. The first APK is a native vertical slice, not feature parity.
