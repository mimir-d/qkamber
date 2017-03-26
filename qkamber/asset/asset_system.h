#pragma once

#include "subsystem.h"
#include "engine.h"
#include "image_loader.h"
#include "geometry_loader.h"

class AssetSystem : public Subsystem
{
public:
    AssetSystem(QkEngine::Context& context);
    ~AssetSystem();

    // TODO: might have some strategies here for on-the-fly resource management
    void process() final {}

public:
    std::shared_ptr<ImageAsset> load_image(
        const std::string& filename,
        ImageLoader::FileFormat format = ImageLoader::FileFormat::Unknown
    );

    std::shared_ptr<GeometryAsset> load_geometry(
        const std::string& filename,
        GeometryLoader::FileFormat format = GeometryLoader::FileFormat::Unknown
    );

private:
    template <typename T>
    using cache_t = std::unordered_map<std::string, std::shared_ptr<T>>;

    template <typename T, typename Loader, typename... Args>
    std::shared_ptr<T> cache_load(cache_t<T>& cache, Loader& loader, const std::string& filename, Args&&... args);

    ImageLoader m_image_loader;
    GeometryLoader m_geometry_loader;

    cache_t<ImageAsset> m_image_cache;
    cache_t<GeometryAsset> m_geometry_cache;
};

///////////////////////////////////////////////////////////////////////////////
// impl
///////////////////////////////////////////////////////////////////////////////
inline AssetSystem::AssetSystem(QkEngine::Context& context) :
    Subsystem(context),
    m_image_loader(*this),
    m_geometry_loader(*this)
{
    flog("id = %#x", this);
    log_info("Created asset system");
}

inline AssetSystem::~AssetSystem()
{
    flog();
    log_info("Destroyed asset system");
}

inline std::shared_ptr<ImageAsset>
AssetSystem::load_image(const std::string& filename, ImageLoader::FileFormat format)
{
    flog();
    return cache_load(m_image_cache, m_image_loader, filename, format);
}

inline std::shared_ptr<GeometryAsset>
AssetSystem::load_geometry(const std::string& filename, GeometryLoader::FileFormat format)
{
    flog();
    return cache_load(m_geometry_cache, m_geometry_loader, filename, format);
}

template <typename T, typename Loader, typename... Args>
inline std::shared_ptr<T> AssetSystem::cache_load(
    cache_t<T>& cache, Loader& loader,
    const std::string& filename, Args&&... args
) {
    auto search = cache.find(filename);
    if (search == cache.end())
    {
        dlog("Asset cache miss, creating filename = %s", filename.c_str());
        return cache[filename] = std::shared_ptr<T>{ loader.load(filename, std::forward<Args>(args)...) };
    }

    dlog("Asset cache hit, filename = %s", filename.c_str());
    return search->second;
}
