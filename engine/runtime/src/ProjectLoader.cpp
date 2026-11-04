#include "aether/runtime/ProjectLoader.hpp"
#include "aether/runtime/AssetStore.hpp"
#include "aether/aether_core.h"
#include <nlohmann/json.hpp>
namespace aether {
ProjectSettings ProjectLoader::load(const std::string &path) {
    ProjectSettings settings;
    const std::string source = AssetStore::readText(path);
    if (source.empty()) { aether_log(AETHER_LOG_WARN, "Project file not found: %s", path.c_str()); return settings; }
    try {
        const auto doc = nlohmann::json::parse(source);
        if (doc.value("format", std::string()) != "aether.project") {
            aether_log(AETHER_LOG_ERROR, "Unsupported project format"); return settings;
        }
        settings.name = doc.value("name", settings.name);
        settings.start_scene = doc.value("startScene", settings.start_scene);
        if (doc.contains("renderer") && doc["renderer"].is_object())
            settings.procedural_materials = doc["renderer"].value("proceduralMaterials", true);
        aether_log(AETHER_LOG_INFO, "Loaded project '%s'", settings.name.c_str());
        const auto scene = AssetStore::readText(std::string("game/") + settings.start_scene);
        if (!scene.empty()) {
            const auto scene_doc = nlohmann::json::parse(scene);
            const auto count = scene_doc.value("entities", nlohmann::json::array()).size();
            aether_log(AETHER_LOG_INFO, "Scene loaded: %zu entities", count);
        } else aether_log(AETHER_LOG_WARN, "Scene asset missing: %s", settings.start_scene.c_str());
    } catch (const std::exception &error) {
        aether_log(AETHER_LOG_ERROR, "Project JSON error: %s", error.what());
    }
    return settings;
}
}
