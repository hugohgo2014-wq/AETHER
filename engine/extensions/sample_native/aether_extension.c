#include "aether/aether_plugin.h"
static int sample_on_load(const AetherHostApi *host) {
    if (host != NULL && host->log != NULL) host->log(AETHER_LOG_INFO, "Sample native extension attached");
    return 1;
}
static void sample_on_update(const AetherHostApi *host, float dt) { (void)host; (void)dt; }
static void sample_on_unload(const AetherHostApi *host) {
    if (host != NULL && host->log != NULL) host->log(AETHER_LOG_INFO, "Sample native extension detached");
}
const AetherPluginDescriptor *aether_sample_extension_descriptor(void) {
    static const AetherPluginDescriptor descriptor = {
        AETHER_PLUGIN_ABI_VERSION, "aether.sample", "AETHER Sample Extension", 0x00010000u,
        sample_on_load, sample_on_update, sample_on_unload
    };
    return &descriptor;
}
