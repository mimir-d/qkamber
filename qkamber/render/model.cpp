
#include "precompiled.h"
#include "model.h"

#include "render/render_system.h"
#include "resource/loader.h"
#include "mesh.h"
#include "material.h"

using namespace std;

///////////////////////////////////////////////////////////////////////////////
// Model impl
///////////////////////////////////////////////////////////////////////////////
Model::Model(RenderDevice& dev, Loader& loader, const string& tex_name)
{
    flog("id = %#x", this);

    // TODO: get loader and iterate thru resource chunks
    std::unique_ptr<Image> im0 = loader.load_image(tex_name);
    m_texture = dev.create_texture(im0.get());

    // TODO: temporary
    m_mesh.reset(new Mesh{ dev });
    m_material.reset(new Material);
    m_material->set_texture(m_texture.get());

    m_units.emplace_back(m_mesh.get(), m_material.get());

    log_info("Created model %#x", this);
}

Model::~Model()
{
    flog();
    log_info("Destroyed model %#x", this);
}
