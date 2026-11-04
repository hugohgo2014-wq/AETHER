#include "aether/runtime/EngineRuntime.hpp"
#include "aether/runtime/AssetStore.hpp"
#include "aether/aether_core.h"
extern "C" const AetherPluginDescriptor *aether_sample_extension_descriptor(void);
namespace aether {
EngineRuntime &EngineRuntime::instance() { static EngineRuntime runtime; return runtime; }
void EngineRuntime::initialize(void *manager) {
    if (initialized_) return;
    aether_core_init(); AssetStore::mountAndroid(manager);
    plugins_ = aether_plugin_registry_create();
    if (plugins_ != nullptr) {
        const auto result = aether_plugin_registry_register(plugins_, aether_sample_extension_descriptor());
        if (result != AETHER_OK) aether_log(AETHER_LOG_WARN, "Sample extension registration failed (%d)", (int)result);
    }
    project_ = ProjectLoader::load("game/project.json");
    world_.createEntity("Procedural Planet");
    world_.createEntity("Key Light");
    world_.createEntity("Showcase Camera");
    (void)audio_.initialize();
    (void)scripting_.executeAsset("game/extensions/boot.lua");
    initialized_ = true;
    aether_log(AETHER_LOG_INFO, "Runtime ready: %zu entities", world_.entityCount());
}
void EngineRuntime::update(float dt) {
    if (initialized_ && plugins_ != nullptr) aether_plugin_registry_update(plugins_, dt);
}
}
