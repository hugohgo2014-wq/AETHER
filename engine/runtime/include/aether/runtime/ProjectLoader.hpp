#pragma once
#include <array>
#include <string>
namespace aether {
struct ProjectSettings {
    std::string name = "AETHER Project";
    std::string start_scene = "scenes/main.scene.json";
    std::array<float, 4> clear_color {0.015f, 0.022f, 0.045f, 1.0f};
    bool procedural_materials = true;
};
class ProjectLoader final {
public:
    static ProjectSettings load(const std::string &asset_path);
};
}
