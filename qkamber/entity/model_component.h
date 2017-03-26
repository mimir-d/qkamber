#pragma once

class Model;

class ModelComponent
{
public:
    void set_model(std::shared_ptr<Model> model);
    Model* get_model();

private:
    std::shared_ptr<Model> m_model;
};

///////////////////////////////////////////////////////////////////////////////
// impl
///////////////////////////////////////////////////////////////////////////////
inline void ModelComponent::set_model(std::shared_ptr<Model> model)
{
    m_model = model;
}

inline Model* ModelComponent::get_model()
{
    return m_model.get();
}
