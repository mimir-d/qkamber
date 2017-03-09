#pragma once

#include "image_loader.h"

class Loader
{
public:
    Loader();
    ~Loader();

    std::unique_ptr<Image> load_image(
        const std::string& filename,
        ImageLoader::FileFormat format = ImageLoader::FileFormat::Unknown
    );

private:
    ImageLoader m_image_loader;
};

///////////////////////////////////////////////////////////////////////////////
// impl
///////////////////////////////////////////////////////////////////////////////
inline Loader::Loader()
{
    flog("id = %#x", this);
    log_info("Created resource loader");
}

inline Loader::~Loader()
{
    flog();
    log_info("Destroyed resource loader");
}

inline std::unique_ptr<Image> Loader::load_image(const std::string& filename, ImageLoader::FileFormat format)
{
    return m_image_loader.load(filename, format);
}
