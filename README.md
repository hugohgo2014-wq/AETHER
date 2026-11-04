# AETHER Engine

**A native Android-first 3D game-engine foundation. C17 core + C++20 runtime + OpenGL ES 3.0.**

AETHER is intended to be a reusable engine for multiple game projects, not a single hard-coded demo. The Android APK contains the editor/runtime host; each game is a separate project folder with its own scene files, scripts, assets and extensions. It does not use WebView, HTML or a browser renderer.

## What works in this build

- Create a new project in the APK, reopen saved projects, or import a project ZIP.
- Native project loading from a writable app workspace (not just assets baked into the APK).
- JSON scene loading with cube, sphere and plane primitives, per-object transforms and colors.
- Add cube/sphere objects from the APK UI; the scene is saved back to the active project.
- C17 core and C plugin ABI; C++20 runtime with EnTT ECS.
- Lua project startup scripts.
- Runtime native extension loading from `extensions/exe/*.so` through `dlopen`.
- Native OpenGL ES 3.0 renderer. The old fixed procedural-planet showcase has been removed.
- Separate Debug and Release APKs. Debug adds FPS/object/draw-call/plugin counters and log export; Release omits those tools.
- JDK 17, Android SDK 35, NDK 27.2.12479018 and CMake 3.22.1.

## Project format

Each project ZIP must contain `project.json` at its root (or one directory below it). The manifest points to a scene relative to the project directory:

```json
{
  "format": "aether.project",
  "version": 1,
  "name": "My Game",
  "startScene": "scenes/main.scene.json",
  "extensions": {
    "scripts": ["extensions/scripts/boot.lua"],
    "nativeLibraries": ["extensions/exe/libgameplay.so"]
  }
}
```

Scene entities accept `mesh` values `cube`, `sphere` or `plane`, plus `transform.position`, `transform.rotation`, `transform.scale` and `material.color`. Project files live in the application's private storage and remain separate from the engine APK.

## Modpack-style extensions

An extension pack is project-local. Lua scripts and compiled native modules are listed by relative path in `project.json`. Put Android native modules in `extensions/exe/`. A native module must be compiled with the Android NDK for the matching ABI (`arm64-v8a` or `armeabi-v7a`) and export `aether_plugin_query(uint32_t)`.

Android does not load Windows `.exe` programs. The `exe` folder name is the extension slot requested for compiled native modules; on Android the actual executable format is an ABI-matched ELF shared library with the `.so` suffix. Only load extensions you trust: native code runs with the app's permissions.

## Build

Clone with submodules:

```sh
git clone --recurse-submodules https://github.com/hugohgo2014-wq/AETHER.git
gradle :app:assembleDebug
gradle :app:assembleRelease
```

Debug APK: `app/build/outputs/apk/debug/app-debug.apk`  
Release APK: `app/build/outputs/apk/release/app-release.apk`

GitHub Actions publishes separate `AETHER-Android-Debug` and `AETHER-Android-Release` artifacts.

## Architecture

- `engine/core`: C17 logging, time, memory facade and plugin ABI/registry.
- `engine/runtime`: C++20 project/scene loader, EnTT world, Lua, audio and asset access.
- `engine/renderer`: generic GLES3 primitive scene renderer.
- `extensions/exe`: native sample module source and NDK extension contract.
- `app`: Android editor/runtime shell, project manager and JNI bridge.
- `external`: pinned upstream open-source dependencies.

## Honest scope

This is a working native engine foundation, not Unreal Engine feature parity. This build has a project workspace, primitive scene editing, scripts, native extension loading and a GLES3 runtime. glTF model importing, full inspector/transform gizmos, visual scripting, advanced PBR/shadows, physics adapters, animation, navigation, streaming and large-world tooling still need implementation. The APK should not be padded with unused libraries just to reach a 60 MB file size; engine capability matters more than artificial APK size.

Original AETHER code: MIT. Third-party modules retain their upstream licenses; see `docs/THIRD_PARTY.md`.
