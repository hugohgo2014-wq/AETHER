# Extension system

AETHER exposes a C ABI to avoid sharing C++ compiler settings, STL types or object layouts across plugin boundaries.

A native extension returns an AetherPluginDescriptor for AETHER_PLUGIN_ABI_VERSION. The descriptor includes a stable ID, display name, version and load/update/unload callbacks. The host API currently exposes logging and engine-owned allocation.

The sample native module is in engine/extensions/sample_native/aether_extension.c. The sample Lua extension is in app/src/main/assets/game/extensions/boot.lua.

Android native modules are linked into the APK. Lua scripts and data-only packages are loaded from project assets. Desktop builds can use the shared-library loader. Validate IDs, ABI versions and manifests; never pass C++ objects or exceptions across the C ABI.
