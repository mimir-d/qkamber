#pragma once

#include "engine.h"

class Subsystem
{
public:
    Subsystem(QkEngine::Context& context);
    ~Subsystem() = default;

    virtual void process() = 0;

protected:
    QkEngine::Context& m_context;
};

///////////////////////////////////////////////////////////////////////////////
// impl
///////////////////////////////////////////////////////////////////////////////
inline Subsystem::Subsystem(QkEngine::Context& context) :
    m_context(context)
{}
