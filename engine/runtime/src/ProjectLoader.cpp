#include "aether/runtime/ProjectLoader.hpp"
#include "aether/runtime/AssetStore.hpp"
#include "aether/aether_core.h"
#include <nlohmann/json.hpp>
namespace aether {
namespace {
void readStringArray(const nlohmann::json &object, const char *key, std::vector<std::string> &out) {
    if (!object.is_object() || !object.contains(key) || !object[key].is_array()) return;
    for (const auto &entry : object[key]) if (entry.is_string()) out.push_back(entry.get<std::string>());
}
template<std::size_t N>
void readFloatArray(const nlohmann::json &object, const char *key, std::array<float,N> &out) {
    if (!object.is_object() || !object.contains(key) || !object[key].is_array()) return;
    const auto &values = object[key];
    for (std::size_t i=0; i<N && i<values.size(); ++i)
        if (values[i].is_number()) out[i] = values[i].get<float>();
}
void parseObject(const nlohmann::json &entry, ProjectSettings &settings) {
    if (!entry.is_object()) return;
    SceneObject object;
    object.name = entry.value("name", object.name);
    object.mesh = entry.value("mesh", object.mesh);
    if (entry.contains("transform")) {
        const auto &t = entry["transform"];
        readFloatArray(t,"position",object.position);
        readFloatArray(t,"rotation",object.rotation);
        readFloatArray(t,"scale",object.scale);
    }
    if (entry.contains("material")) {
        const auto &m = entry["material"];
        readFloatArray(m,"color",object.color);
    } else {
        readFloatArray(entry,"color",object.color);
    }
    object.visible = entry.value("visible",true);
    settings.objects.push_back(std::move(object));
}
}
ProjectSettings ProjectLoader::load(const std::string &path) {
    ProjectSettings settings;
    const std::string source = AssetStore::readText(path);
    if (source.empty()) {
        aether_log(AETHER_LOG_ERROR,"Project file not found: %s",path.c_str());
        return settings;
    }
    try {
        const auto doc = nlohmann::json::parse(source);
        if (!doc.is_object() || doc.value("format",std::string())!="aether.project") {
            aether_log(AETHER_LOG_ERROR,"Unsupported AETHER project format"); return settings;
        }
        settings.name = doc.value("name",settings.name);
        settings.start_scene = doc.value("startScene",settings.start_scene);
        if (doc.contains("environment") && doc["environment"].is_object()) {
            readFloatArray(doc["environment"],"clearColor",settings.clear_color);
            settings.show_grid = doc["environment"].value("grid",true);
        }
        if (doc.contains("extensions") && doc["extensions"].is_object()) {
            const auto &extensions=doc["extensions"];
            readStringArray(extensions,"scripts",settings.script_extensions);
            readStringArray(extensions,"nativeLibraries",settings.native_libraries);
        }
        const std::string scene = AssetStore::readText(settings.start_scene);
        if (scene.empty()) {
            aether_log(AETHER_LOG_ERROR,"Scene not found: %s",settings.start_scene.c_str());
            return settings;
        }
        const auto scene_doc=nlohmann::json::parse(scene);
        if (!scene_doc.is_object() || scene_doc.value("format",std::string())!="aether.scene") {
            aether_log(AETHER_LOG_ERROR,"Unsupported scene format: %s",settings.start_scene.c_str());
            return settings;
        }
        if (scene_doc.contains("environment") && scene_doc["environment"].is_object()) {
            readFloatArray(scene_doc["environment"],"clearColor",settings.clear_color);
            settings.show_grid=scene_doc["environment"].value("grid",settings.show_grid);
        }
        if (scene_doc.contains("entities") && scene_doc["entities"].is_array())
            for (const auto &entry:scene_doc["entities"]) parseObject(entry,settings);
        aether_log(AETHER_LOG_INFO,"Project '%s' loaded: %zu scene objects",settings.name.c_str(),settings.objects.size());
    } catch (const std::exception &error) {
        aether_log(AETHER_LOG_ERROR,"Project/scene JSON error: %s",error.what());
    }
    return settings;
}
}
