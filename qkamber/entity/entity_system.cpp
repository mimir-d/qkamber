
#include "precompiled.h"
#include "entity_system.h"

// TODO: temp until entity factory?
#include "engine.h"
#include "asset/asset_system.h"
#include "render/render_system.h"
#include "render/model.h"

using namespace std;

EntitySystem::EntitySystem(QkEngine::Context& context) :
    Subsystem(context)
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

    eid_t eid = get_next_id();
    m_store.resize(eid + 1);
    m_mask.resize(eid + 1);

    // TODO: small block allocator or value-type
    auto ret = unique_ptr<Entity>(new Entity{ *this, eid, private_tag{} });

    // add components
    // TODO: base on config file
    ret->add_component<SrtComponent>();

    auto& asset = m_context.get_asset();
    auto geometry = asset.load_geometry(name);
    // TODO: kept a memleak here
    auto m = new Model(*geometry.get(), m_context.get_render().get_device(), asset);
    ret->add_component<ModelComponent>().set_model(m);


    log_info("Created entity id = %d...", eid);
    return ret;
}
