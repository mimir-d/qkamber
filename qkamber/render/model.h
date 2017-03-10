#pragma once

#include "render_primitive.h"
#include "mesh.h"
#include "material.h"
#include "render_buffers.h"

class RenderDevice;
class Loader;

class Model
{
public:
    class Unit
    {
    public:
        Unit(Mesh* mesh, Material* material);
        ~Unit();

        RenderPrimitive get_primitive() const;
        Material* get_material() const;

    private:
        Mesh* m_mesh;
        Material* m_material;
    };

    using Units = std::vector<Unit>;

public:
    Model(RenderDevice& dev, Loader& loader, const std::string& tex_name);
    ~Model();

    const Units& get_units() const;

private:
    Units m_units;
    // TODO: temporary until someone owns up to these
    std::unique_ptr<Mesh> m_mesh;
    std::unique_ptr<Material> m_material;
    std::unique_ptr<Texture> m_texture;
};

///////////////////////////////////////////////////////////////////////////////
// impl
///////////////////////////////////////////////////////////////////////////////
inline Model::Unit::Unit(Mesh* mesh, Material* material) :
    m_mesh(mesh), m_material(material)
{
    flog("id = %#x");
    log_info("Created model unit %#x", this);
}

inline Model::Unit::~Unit()
{
    flog();
    log_info("Destroyed model unit %#x", this);
}

inline RenderPrimitive Model::Unit::get_primitive() const
{
    return m_mesh->get_primitive();
}

inline Material* Model::Unit::get_material() const
{
    return m_material;
}

inline const Model::Units& Model::get_units() const
{
    return m_units;
}
