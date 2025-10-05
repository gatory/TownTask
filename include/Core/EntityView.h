#pragma once
#include "World.h"
#include <vector>

template<typename... Components>
class EntityView {
private:
    World& world;
    std::vector<Entity> matchingEntities;
    
    template<typename T>
    bool HasComponent(Entity e);
    
    void BuildView() {
        matchingEntities.clear();
        for (Entity e : world.GetEntities()) {
            if ((HasComponent<Components>(e) && ...)) {
                matchingEntities.push_back(e);
            }
        }
    }
    
public:
    EntityView(World& w) : world(w) {
        BuildView();
    }
    
    auto begin() { return matchingEntities.begin(); }
    auto end() { return matchingEntities.end(); }
    size_t size() const { return matchingEntities.size(); }
    bool empty() const { return matchingEntities.empty(); }
};
