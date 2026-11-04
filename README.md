# AETHER Engine

**Native C/C++ game engine — Android runtime foundation and modular desktop-editor architecture.**

AETHER targets complex 2D/3D games with a C17 core, C++20 runtime systems, native Android rendering and a versioned extension ABI. This repository is the first working engineering slice, not a dishonest claim of feature parity with Unreal, Unity or Godot.

## Included now

- C17 core: logging, monotonic clock, memory facade and native plugin registry.
- C++20 runtime: EnTT ECS, JSON project/scene loader, Lua scripting and miniaudio.
- Android Java host + JNI + OpenGL ES 3.0. No WebView, HTML or browser runtime.
- Procedural 3D planet shader with animated star field, directional lighting, specular, atmosphere rim and tone mapping.
- Project content is read from the APK assets folder through Android NDK AAssetManager.
- Android APK and C-core CI workflows.
- Pinned open-source modules for rendering, editor UI, physics, navigation, asset import, profiling and optimization.

## Build

Clone with all dependencies:

    git clone --recurse-submodules https://github.com/hugohgo2014-wq/AETHER.git

Install JDK 17, AGP 8.8.2, Gradle 8.10.2, Android SDK 35, NDK 27.2.12479018 and CMake 3.22.1. Then run:

    gradle :app:assembleDebug

APK output: app/build/outputs/apk/debug/app-debug.apk

## Project content inside the APK

Demo project: app/src/main/assets/game/project.json. The native runtime opens this file and its startScene using AAssetManager. Put game resources beneath app/src/main/assets/game/ to package them into the APK.

## Structure

- engine/core — C17 ABI, logging, memory and plugin host.
- engine/runtime — C++20 world, asset and project systems.
- engine/renderer — native GLES3 showcase renderer.
- engine/audio — miniaudio integration.
- engine/scripting — embedded Lua runtime.
- engine/extensions — sample native extension.
- editor — dockable desktop-editor UI scaffold.
- external — pinned upstream projects as Git submodules.
- app — Java/JNI Android host and packaged sample project.

## Extension model

Native plugins implement the C ABI in engine/core/include/aether/aether_plugin.h. Android native modules are linked into the APK; Lua extensions are loaded from packaged project assets. Desktop builds can load native shared libraries. See docs/EXTENSIONS.md.

## Roadmap and honest scope

This foundation is the start of an engine, not a complete high-end engine yet. Scene editing, PBR render graph, advanced shadows, async streaming, 3D physics adapters, navigation, animation, visual scripting and the complete editor remain roadmap work. See docs/ROADMAP.md and docs/ARCHITECTURE.md.

AETHER original code: MIT. Third-party modules retain their upstream licenses; see docs/THIRD_PARTY.md.
