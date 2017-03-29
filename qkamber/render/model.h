#pragma once

#include "render_primitive.h"
#include "mesh.h"

class Material;
class GeometryAsset;

class Model
{
public:
    class Unit
    {
    public:
        Unit(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material);
        ~Unit();

        RenderPrimitive get_primitive() const;
        const Material& get_material() const;

    private:
        std::shared_ptr<Mesh> m_mesh;
        std::shared_ptr<Material> m_material;
    };

    using Units = std::vector<Unit>;

public:
    Model(const GeometryAsset& geometry, RenderSystem& render);
    ~Model();

    const Units& get_units() const;

private:
    Units m_units;
};

///////////////////////////////////////////////////////////////////////////////
// impl
///////////////////////////////////////////////////////////////////////////////
inline Model::Unit::Unit(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material) :
    m_mesh(mesh),
    m_material(material)
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

inline const Material& Model::Unit::get_material() const
{
    return *m_material.get();
}

inline const Model::Units& Model::get_units() const
{
    return m_units;
}
