# Modpack-style extension system

Every game project can carry its own extension set. This is intended to work like a modpack: the base AETHER APK supplies the engine, while project.json selects the scripts and native modules that modify or extend the runtime.

## Project manifest

\`\`\`json
{
  "extensions": {
    "scripts": ["extensions/scripts/boot.lua"],
    "nativeLibraries": [
      "extensions/exe/libgameplay.so",
      "extensions/exe/libcustom_rendering.so"
    ]
  }
}
\`\`\`

Lua scripts are loaded by the project's Lua runtime. Native libraries must export aether_plugin_query(uint32_t host_abi_version) and return an AetherPluginDescriptor for AETHER_PLUGIN_ABI_VERSION. The descriptor provides an id, display name, version, and load/update/unload callbacks. The host API exposes logging and engine-owned allocation.

## Android executable format

Android does not execute Windows .exe binaries. Keep compiled native extensions in the requested extensions/exe folder, but compile them as ELF shared libraries (.so) with the Android NDK for each target ABI (arm64-v8a and/or armeabi-v7a). Set the .so path in nativeLibraries. C and C++ source files are build inputs; the runtime loads the compiled module, not source code.

The loader uses dlopen/dlsym and rejects missing query symbols, duplicate IDs or incompatible ABI versions. Imported modules are staged inside app-private project storage and marked read-only before they are loaded. Native extensions execute with the same permissions as AETHER; only load modules you trust.

The sample module source is extensions/exe/sample_extension.cpp. A built sample is staged into the starter project's extensions/exe folder by the Android app.
