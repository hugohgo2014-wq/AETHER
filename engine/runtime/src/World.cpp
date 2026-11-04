#include "aether/runtime/World.hpp"
#include <utility>
namespace aether {
entt::entity World::createEntity(std::string name) {
    const auto entity = registry_.create();
    registry_.emplace<NameComponent>(entity, NameComponent{std::move(name)});
    registry_.emplace<TransformComponent>(entity);
    return entity;
}
std::size_t World::entityCount() const {
    std::size_t count = 0;
    const auto view = registry_.view<const NameComponent>();
    for (const auto entity : view) {
        (void)entity;
        ++count;
    }
    return count;
}
}
