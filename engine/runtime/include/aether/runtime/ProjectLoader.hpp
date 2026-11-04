#pragma once
#include <array>
#include <string>
#include <vector>
namespace aether {
struct SceneObject {
    std::string name = "Object";
    std::string mesh = "cube";
    std::array<float,3> position {0.0f,0.0f,0.0f};
    std::array<float,3> rotation {0.0f,0.0f,0.0f};
    std::array<float,3> scale {1.0f,1.0f,1.0f};
    std::array<float,4> color {0.35f,0.58f,0.95f,1.0f};
    bool visible = true;
};
struct ProjectSettings {
    std::string name = "Untitled AETHER Project";
    std::string start_scene = "scenes/main.scene.json";
    std::array<float,4> clear_color {0.025f,0.035f,0.065f,1.0f};
    bool show_grid = true;
    std::vector<SceneObject> objects;
    std::vector<std::string> script_extensions;
    std::vector<std::string> native_libraries;
};
class ProjectLoader final {
public:
    static ProjectSettings load(const std::string &project_file);
};
}
