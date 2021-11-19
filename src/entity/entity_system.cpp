
#include "precompiled.h"
#include "entity_system.h"

#include "entity_config.h"
#include "asset/asset_system.h"
#include "render/render_system.h"
#include "render/model.h"
#include "engine.h"

using namespace std;

EntitySystem::EntitySystem(QkEngine::Context& context) :
    Subsystem(context),
    m_config{ new EntityConfig{ "entities.ini", context.get_render() } }
{
    flog("id = %#x", this);
    log_info("Created entity system");
}

EntitySystem::~EntitySystem()
{
    flog();
    log_info("Destroyed entity system");
}

unique_ptr<EntitySystem::Entity> EntitySystem::create_entity(const string& name)
{
    flog();

    // allocate id and reserve components
    eid_t eid = get_next_id();
    m_store.resize(eid + 1);
    m_mask.resize(eid + 1);

    // TODO: small block allocator or value-type
    auto ret = unique_ptr<Entity>(new Entity{ *this, eid, private_tag{} });
    m_config->config(name, *ret.get());

    log_info("Created entity id = %d...", eid);
    return ret;
}
