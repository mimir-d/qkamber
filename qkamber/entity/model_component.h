#pragma once

class Model;

class ModelComponent
{
public:
    void set_model(Model* model);
    Model* get_model();

private:
    Model* m_model;
};

///////////////////////////////////////////////////////////////////////////////
// impl
///////////////////////////////////////////////////////////////////////////////
inline void ModelComponent::set_model(Model* model)
{
    m_model = model;
}

inline Model* ModelComponent::get_model()
{
    return m_model;
}
