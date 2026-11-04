#pragma once
#include "aether/runtime/ProjectLoader.hpp"
#include "aether/runtime/World.hpp"
#include "AudioSystem.hpp"
#include "LuaRuntime.hpp"
#include "aether/aether_plugin.h"
namespace aether {
class EngineRuntime final {
public:
    static EngineRuntime &instance();
    void initialize(void *android_asset_manager);
    void update(float delta_seconds);
    const ProjectSettings &project() const noexcept { return project_; }
    World &world() noexcept { return world_; }
private:
    EngineRuntime() = default;
    bool initialized_ = false;
    ProjectSettings project_;
    World world_;
    AudioSystem audio_;
    LuaRuntime scripting_;
    AetherPluginRegistry *plugins_ = nullptr;
};
}
