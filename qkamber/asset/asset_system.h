#pragma once

#include "subsystem.h"
#include "engine.h"
#include "image_loader.h"

class AssetSystem : public Subsystem
{
public:
    AssetSystem(QkEngine::Context& context);
    ~AssetSystem();

    // TODO: might have some strategies here for on-the-fly resource management
    void process() final {}

public:
    std::shared_ptr<Image> load_image(
        const std::string& filename,
        ImageLoader::FileFormat format = ImageLoader::FileFormat::Unknown
    );

private:
    using ImageCache = std::unordered_map<std::string, std::shared_ptr<Image>>;
    // using MeshCache = std::unordered_map<std::string, ...>;

    ImageLoader m_image_loader;
    ImageCache m_image_cache;
};

///////////////////////////////////////////////////////////////////////////////
// impl
///////////////////////////////////////////////////////////////////////////////
inline AssetSystem::AssetSystem(QkEngine::Context& context) :
    Subsystem(context)
{
    flog("id = %#x", this);
    log_info("Created asset system");
}

inline AssetSystem::~AssetSystem()
{
    flog();
    log_info("Destroyed asset system");
}

inline std::shared_ptr<Image> AssetSystem::load_image(const std::string& filename, ImageLoader::FileFormat format)
{
    flog();

    auto search = m_image_cache.find(filename);
    if (search == m_image_cache.end())
    {
        auto ret = std::shared_ptr<Image>{ m_image_loader.load(filename, format) };
        return m_image_cache[filename] = ret;
    }

    dlog("load_image cache hit, filename = %s", filename.c_str());
    return search->second;
}
