#pragma once
#include <array>
#include <string>
#include <vector>
namespace aether {
struct ProjectSettings {
    std::string name = "AETHER Project";
    std::string start_scene = "scenes/main.scene.json";
    std::array<float, 4> clear_color {0.015f, 0.022f, 0.045f, 1.0f};
    bool procedural_materials = true;
    std::vector<std::string> entity_names;
    std::vector<std::string> script_extensions;
    std::vector<std::string> native_extensions;
};
class ProjectLoader final {
public:
    static ProjectSettings load(const std::string &asset_path);
};
}
