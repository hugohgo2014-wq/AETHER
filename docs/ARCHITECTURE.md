# Runtime architecture

AETHER's Android APK is a game player, not an editor. It automatically launches the game packaged in \`app/src/main/assets/game/\`. There are no in-app project creation, import, scene-editing or object-placement controls.

## Android lifecycle

Java creates a fullscreen GLSurfaceView and EGL context. On startup it stages the packaged \`assets/game\` tree into app-private runtime storage. A version marker refreshes that copy after APK updates. JNI passes Android AssetManager and the active game root to the runtime.

The runtime loads \`project.json\`, the selected start scene, Lua startup scripts and project-listed native extensions. The renderer consumes the resulting ProjectSettings and draws supported scene objects using OpenGL ES 3.0.

## Debug and Release

Release is a clean fullscreen player with no engine overlay. Debug includes FPS/object/draw-call/native-plugin counters, a grid diagnostic toggle and logcat export. Debug controls are compiled out of Release.

## Extensions

Native Android modules are ELF shared libraries (.so) built for the target ABI and loaded from \`extensions/exe\` through dlopen. The public C ABI uses versioned C structs and host callbacks; C++ objects and exceptions must not cross this boundary.

## Current renderer scope

The current renderer supports built-in cube, sphere and plane geometry with transforms and flat material colors. glTF/GLB loading, texture/material resources, full input/gameplay bindings, physics adapters, animation, PBR, shadows, post-processing and large-world streaming remain future work.
