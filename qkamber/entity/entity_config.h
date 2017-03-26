#pragma once

#include "entity_system.h"

class RenderSystem;
class RenderCache;

class EntityConfig
{
public:
    EntityConfig(const std::string& filename, RenderSystem& render);
    ~EntityConfig() = default;

    void config(const std::string& name, EntitySystem::Entity& entity);

private:
    RenderCache& m_cache;
};
