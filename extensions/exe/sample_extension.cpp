#include "aether/aether_plugin.h"
static int onLoad(const AetherHostApi *host) {
    if (host && host->log) host->log(AETHER_LOG_INFO, "AETHER sample native module loaded from extensions/exe");
    return 1;
}
static void onUpdate(const AetherHostApi *, float) {}
static void onUnload(const AetherHostApi *host) {
    if (host && host->log) host->log(AETHER_LOG_INFO, "AETHER sample native module unloaded");
}
extern "C" __attribute__((visibility("default")))
const AetherPluginDescriptor *aether_plugin_query(uint32_t host_abi_version) {
    static const AetherPluginDescriptor descriptor = {
        AETHER_PLUGIN_ABI_VERSION, "aether.sample.native", "AETHER Native Sample Module",
        0x00010000u, onLoad, onUpdate, onUnload
    };
    return host_abi_version == AETHER_PLUGIN_ABI_VERSION ? &descriptor : nullptr;
}
