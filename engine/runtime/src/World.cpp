#include "aether/runtime/World.hpp"
namespace aether {
entt::entity World::createEntity(std::string name) {
    const auto entity = registry_.create();
    registry_.emplace<NameComponent>(entity, NameComponent{std::move(name)});
    registry_.emplace<TransformComponent>(entity);
    return entity;
}
std::size_t World::entityCount() const {
    return registry_.view<const NameComponent>().size_hint();
}
}
