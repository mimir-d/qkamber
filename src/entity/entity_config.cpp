
#include "precompiled.h"
#include "entity_config.h"

#include "entity_system.h"
#include "render/render_system.h"
#include "render/render_cache.h"

using namespace std;

EntityConfig::EntityConfig(const string& filename, RenderSystem& render) :
    m_cache(render.get_cache())
{
    flog();
    // TODO: read config from file
}

void EntityConfig::config(const string& name, EntitySystem::Entity& entity)
{
    flog();

    log_info("Config entity, name = %s", name.c_str());
    entity.add_component<SrtComponent>();
    entity.add_component<ModelComponent>().set_model(m_cache.get_model(name));
}
