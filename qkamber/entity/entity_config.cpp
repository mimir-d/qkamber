
#include "precompiled.h"
#include "entity_config.h"

#include "entity_system.h"
#include "render/render_system.h"
#include "asset/asset_system.h"

using namespace std;

EntityConfig::EntityConfig(const string& filename)
{
    flog();
    // TODO: read config from file
}

void EntityConfig::config(const string& name, EntitySystem::Entity& entity, RenderSystem& render, AssetSystem& asset)
{
    flog();

    log_info("Config entity, name = %s", name.c_str());
    entity.add_component<SrtComponent>();

    auto geometry = asset.load_geometry(name);
    // TODO: kept a memleak here
    auto m = new Model(*geometry.get(), render.get_device(), asset);
    entity.add_component<ModelComponent>().set_model(m);
}