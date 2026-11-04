# Native extensions
Android native modules are declared in project.json:
extensions.nativeLibraries = ["extensions/exe/libgameplay.so"]

Compile C/C++ modules with the Android NDK for the target ABI (arm64-v8a or
armeabi-v7a). Export aether_plugin_query(uint32_t host_abi_version) and return
an AetherPluginDescriptor with ABI version 1. Do not place Windows .exe files here.
Native modules execute with the permissions of the app; load only code you trust.
