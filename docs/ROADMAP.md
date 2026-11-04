# Roadmap and scope

## Milestone 0.2 — native reusable project runtime (current target)
Project workspace in the APK, create/reopen/import ZIP, project-relative disk asset loading, JSON scene primitives, saved primitive additions, generic GLES3 renderer, Lua script loading, Android dlopen native extensions, Debug/Release APK separation, debug counters and log export.

## Milestone 0.3 — editor fundamentals
Touch camera orbit/pan/zoom, scene hierarchy, object selection, transform inspector, undo/redo, material editing, scene serialization UI and project asset browser.

## Milestone 0.4 — content pipeline
glTF/GLB mesh import, texture decode/upload, material resources, shader variants, async resource handles and mesh optimization.

## Milestone 0.5 — gameplay systems
Fixed-step simulation, transform hierarchy, collision/physics adapters, animation, navigation, input mapping and Lua/C++ gameplay bindings.

## Milestone 0.6 — high-end renderer and large worlds
PBR/IBL, shadow maps/cascades, HDR, bloom, color grading, SSAO, temporal anti-aliasing, GPU instancing, culling/LOD, world partitioning, streaming, job scheduling and Tracy profiling.

Unreal/Unity/Godot parity is a long-term engineering goal, not a claim about the current APK. Each milestone needs implementation, device testing and profiling before it should be described as complete.
