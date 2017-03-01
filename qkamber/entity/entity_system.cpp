
#include "precompiled.h"
#include "entity_system.h"

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

std::unique_ptr<EntitySystem::Entity> EntitySystem::create_entity()
{
    flog();

    eid_t eid = get_next_id();
    m_store.resize(eid + 1);
    m_mask.resize(eid + 1);

    // TODO: small block allocator or value-type
    log_info("Creating entity id = %d...", eid);
    return std::unique_ptr<Entity>(new Entity{ *this, eid });
}
