#pragma once

#include "entity_system.h"

class RenderSystem;
class AssetSystem;

class EntityConfig
{
public:
    EntityConfig(const std::string& filename);
    ~EntityConfig() = default;

    // TODO: dont like the rendersystem here
    void config(const std::string& name, EntitySystem::Entity& entity, RenderSystem& render, AssetSystem& asset);
};
