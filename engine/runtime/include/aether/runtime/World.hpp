#pragma once
#include <cstddef>
#include <string>
#include <entt/entt.hpp>
namespace aether {
struct NameComponent { std::string value; };
struct TransformComponent {
    float position[3] {0.0f, 0.0f, 0.0f};
    float rotation[3] {0.0f, 0.0f, 0.0f};
    float scale[3] {1.0f, 1.0f, 1.0f};
};
struct CameraComponent { float field_of_view = 60.0f; float near_plane = 0.1f; float far_plane = 500.0f; };
struct DirectionalLightComponent { float color[3] {1.0f, 0.96f, 0.88f}; float intensity = 4.0f; };
class World final {
public:
    entt::entity createEntity(std::string name);
    entt::registry &registry() noexcept { return registry_; }
    const entt::registry &registry() const noexcept { return registry_; }
    std::size_t entityCount() const;
private:
    entt::registry registry_;
};
}
