#pragma once

class Mesh;

class ModelComponent
{
public:
    void set_mesh(Mesh* mesh);
    Mesh* get_mesh() const;

private:
    Mesh* m_mesh;
};

///////////////////////////////////////////////////////////////////////////////
// impl
///////////////////////////////////////////////////////////////////////////////
inline void ModelComponent::set_mesh(Mesh* mesh)
{
    m_mesh = mesh;
}

inline Mesh* ModelComponent::get_mesh() const
{
    return m_mesh;
}
