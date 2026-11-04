#include "aether/runtime/EngineRuntime.hpp"
#include "aether/runtime/AssetStore.hpp"
#include "aether/aether_core.h"
extern "C" const AetherPluginDescriptor *aether_sample_extension_descriptor(void);
namespace aether {
EngineRuntime &EngineRuntime::instance() { static EngineRuntime runtime; return runtime; }
void EngineRuntime::initialize(void *manager) {
    if (initialized_) return;
    aether_core_init();
    AssetStore::mountAndroid(manager);
    project_ = ProjectLoader::load("game/project.json");

    plugins_ = aether_plugin_registry_create();
    if (plugins_ != nullptr) {
        for (const auto &extension_id : project_.native_extensions) {
            if (extension_id == "aether.sample") {
                const auto result = aether_plugin_registry_register(plugins_, aether_sample_extension_descriptor());
                if (result != AETHER_OK) {
                    aether_log(AETHER_LOG_WARN, "Native extension '%s' registration failed (%d)",
                               extension_id.c_str(), (int)result);
                }
            } else {
                aether_log(AETHER_LOG_WARN, "Native extension '%s' is not linked into this build",
                           extension_id.c_str());
            }
        }
    }

    for (const auto &name : project_.entity_names) world_.createEntity(name);
    if (project_.entity_names.empty()) {
        world_.createEntity("Default Camera");
    }

    (void)audio_.initialize();
    for (const auto &script : project_.script_extensions) {
        const std::string asset_path = std::string("game/") + script;
        (void)scripting_.executeAsset(asset_path);
    }
    initialized_ = true;
    aether_log(AETHER_LOG_INFO, "Runtime ready: %zu entities", world_.entityCount());
}
void EngineRuntime::update(float dt) {
    if (initialized_ && plugins_ != nullptr) aether_plugin_registry_update(plugins_, dt);
}
}
