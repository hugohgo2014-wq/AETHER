# AETHER Runtime

**Native Android game player — C17 core, C++20 runtime, JNI and OpenGL ES 3.0.**

AETHER's APK is a runtime/player. It automatically launches the game packaged in \`app/src/main/assets/game/\`. It is not an editor: there are no in-app project creation, scene editing, object placement, ZIP import or asset-browser screens. It does not use WebView or HTML.

## What runs

- Packaged game project loaded from \`assets/game/project.json\`.
- JSON scene loading with cube, sphere and plane primitives, transforms and colors.
- C17 core, C++20 runtime and EnTT ECS.
- Lua startup scripts.
- Project-listed Android native extensions loaded from \`extensions/exe/*.so\` through \`dlopen\`.
- Native OpenGL ES 3.0 renderer.
- Separate Debug and Release APKs. Debug has FPS/object/draw-call/plugin counters, grid diagnostic toggle and log export. Release launches fullscreen without engine UI.

## Put a game in the APK

Replace the contents of \`app/src/main/assets/game/\` with the game project before building:

\`\`\`text
app/src/main/assets/game/
  project.json
  scenes/main.scene.json
  models/
  textures/
  materials/
  audio/
  fonts/
  extensions/
    scripts/
    exe/
\`\`\`

The \`startScene\` path in \`project.json\` is relative to \`assets/game/\`. The app stages this directory into its private runtime folder and refreshes it when the APK version changes. To ship a different game, replace the assets and build a new APK.

The currently implemented scene renderer supports cube, sphere and plane primitives with position, rotation, scale and flat color. glTF/GLB import, texture/material loading and advanced rendering are not implemented yet; placing files in those folders alone does not make them render.

## Modpack-style extensions

Lua scripts are listed in \`extensions.scripts\`. Compiled native modules are listed in \`extensions.nativeLibraries\` and placed in \`extensions/exe/\`.

Android does not execute Windows \`.exe\` files. Native extensions must be ELF shared libraries (\`.so\`) built with the Android NDK for the matching ABI (\`arm64-v8a\` or \`armeabi-v7a\`) and export \`aether_plugin_query(uint32_t)\`. Only load trusted native modules because they run with the app's permissions.

## Build

\`\`\`sh
git clone --recurse-submodules https://github.com/hugohgo2014-wq/AETHER.git
gradle :app:assembleDebug
gradle :app:assembleRelease
\`\`\`

Debug: \`app/build/outputs/apk/debug/app-debug.apk\`  
Release: \`app/build/outputs/apk/release/app-release.apk\`

GitHub Actions publishes \`AETHER-Android-Debug\` and \`AETHER-Android-Release\` artifacts.

## Honest scope

This is a native runtime foundation, not Unreal Engine feature parity. It currently runs packaged scenes using supported primitives, Lua startup scripts and compatible native extensions. glTF/GLB, textures/materials, full gameplay input, physics integration, animation, PBR/shadows, streaming and large-world systems remain future work.
