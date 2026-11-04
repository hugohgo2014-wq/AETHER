# Architecture

AETHER is split into a platform-independent C17 core, a C++20 runtime, an Android host and a native renderer.

The C17 core owns logging, a monotonic clock facade, memory helpers and a versioned plugin ABI. The C++ runtime owns the EnTT world, JSON project/scene loading, Lua startup scripts, audio and the platform-neutral asset store.

## Android lifecycle

Java creates a GLSurfaceView and EGL context. JNI passes the Android AssetManager and the active project directory into the runtime. On first launch the starter project is copied from APK assets into the app-private AetherProjects workspace. User-created and imported projects are loaded from writable project directories, so they do not need a new APK build.

The renderer consumes the active ProjectSettings scene list. It no longer draws a fixed planet. Scene entities currently map to cube, sphere and plane geometry with translation, Euler rotation, scale and flat material color. OpenGL ES 3.0 owns shader compilation, mesh buffers and draw submission.

## Projects and extensions

project.json points to a scene and lists project-local Lua scripts and native libraries. Native Android modules are ELF .so files built for the device ABI and loaded from extensions/exe with dlopen. The public C ABI passes only versioned C structs and host callbacks across the extension boundary. Never pass C++ objects or exceptions across this boundary.

Debug builds expose runtime counters and log export. Release builds omit the debug UI and debug-only rendering path.

## Next systems

Resource handles, glTF/GLB import, texture/material pipeline, transform hierarchy, a proper camera controller, physics adapters, fixed-step simulation, job scheduling, culling/LOD, PBR, shadows, post-processing, animation, navigation and a full hierarchy/inspector editor remain separate milestones.
