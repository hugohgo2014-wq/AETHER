#include "aether/runtime/ProjectLoader.hpp"
#include "aether/runtime/AssetStore.hpp"
#include "aether/aether_core.h"
#include <nlohmann/json.hpp>
namespace aether {
namespace {
void readStringArray(const nlohmann::json &object, const char *key, std::vector<std::string> &out) {
    if (!object.is_object() || !object.contains(key) || !object[key].is_array()) return;
    for (const auto &entry : object[key]) {
        if (entry.is_string()) out.push_back(entry.get<std::string>());
    }
}
}
ProjectSettings ProjectLoader::load(const std::string &path) {
    ProjectSettings settings;
    const std::string source = AssetStore::readText(path);
    if (source.empty()) {
        aether_log(AETHER_LOG_WARN, "Project file not found: %s", path.c_str());
        return settings;
    }
    try {
        const auto doc = nlohmann::json::parse(source);
        if (!doc.is_object() || doc.value("format", std::string()) != "aether.project") {
            aether_log(AETHER_LOG_ERROR, "Unsupported project format");
            return settings;
        }
        settings.name = doc.value("name", settings.name);
        settings.start_scene = doc.value("startScene", settings.start_scene);
        if (doc.contains("renderer") && doc["renderer"].is_object()) {
            settings.procedural_materials = doc["renderer"].value("proceduralMaterials", true);
        }
        if (doc.contains("extensions") && doc["extensions"].is_object()) {
            const auto &extensions = doc["extensions"];
            readStringArray(extensions, "scripts", settings.script_extensions);
            readStringArray(extensions, "native", settings.native_extensions);
        }
        aether_log(AETHER_LOG_INFO, "Loaded project '%s'", settings.name.c_str());

        const std::string scene_path = std::string("game/") + settings.start_scene;
        const std::string scene = AssetStore::readText(scene_path);
        if (!scene.empty()) {
            const auto scene_doc = nlohmann::json::parse(scene);
            if (scene_doc.is_object() && scene_doc.contains("entities") && scene_doc["entities"].is_array()) {
                for (const auto &entity : scene_doc["entities"]) {
                    if (entity.is_object() && entity.contains("name") && entity["name"].is_string()) {
                        settings.entity_names.push_back(entity["name"].get<std::string>());
                    }
                }
            }
            aether_log(AETHER_LOG_INFO, "Scene loaded: %zu entities", settings.entity_names.size());
        } else {
            aether_log(AETHER_LOG_WARN, "Scene asset missing: %s", scene_path.c_str());
        }
    } catch (const std::exception &error) {
        aether_log(AETHER_LOG_ERROR, "Project JSON error: %s", error.what());
    }
    return settings;
}
}
