#pragma once
#include <string>
#include "aether/runtime/ProjectLoader.hpp"
#include "aether/runtime/World.hpp"
#include "AudioSystem.hpp"
#include "LuaRuntime.hpp"
#include "aether/aether_plugin.h"
namespace aether {
class EngineRuntime final {
public:
    static EngineRuntime &instance();
    void initialize(void *android_asset_manager,const std::string &project_root);
    bool loadProject(const std::string &project_root);
    void addPrimitive(const std::string &mesh);
    void update(float delta_seconds);
    const ProjectSettings &project() const noexcept { return project_; }
    World &world() noexcept { return world_; }
    std::size_t extensionCount() const noexcept { return aether_plugin_registry_count(plugins_); }
private:
    EngineRuntime()=default;
    bool initialized_=false;
    bool audio_initialized_=false;
    ProjectSettings project_;
    World world_;
    AudioSystem audio_;
    LuaRuntime scripting_;
    AetherPluginRegistry *plugins_=nullptr;
};
}
