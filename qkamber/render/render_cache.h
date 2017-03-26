#pragma once

class RenderSystem;
class AssetSystem;

class Texture;
class Mesh;
class Material;
class Model;

class RenderCache
{
public:
    RenderCache(RenderSystem& render, AssetSystem& asset);
    ~RenderCache();

    std::shared_ptr<Material> get_material(const std::string& name);
    std::shared_ptr<Mesh> get_mesh(const std::string& name);
    std::shared_ptr<Model> get_model(const std::string& name);

    void clear();

private:
    RenderSystem& m_render;
    AssetSystem& m_asset;

    template <typename T>
    using cache_t = std::unordered_map<std::string, std::shared_ptr<T>>;

    template <typename T, typename Creator>
    std::shared_ptr<T> cache_get(cache_t<T>& cache, const std::string& name, Creator create);

    cache_t<Mesh> m_meshes;
    cache_t<Material> m_materials;
    cache_t<Model> m_models;
};

///////////////////////////////////////////////////////////////////////////////
// impl
///////////////////////////////////////////////////////////////////////////////
inline RenderCache::RenderCache(RenderSystem& render, AssetSystem& asset) :
    m_render(render),
    m_asset(asset)
{
    flog("id = %#x", this);
    log_info("Created rendering cache");
}

inline RenderCache::~RenderCache()
{
    flog();

    clear();
    log_info("Destroyed rendering cache");
}

template <typename T, typename Creator>
inline std::shared_ptr<T> RenderCache::cache_get(cache_t<T>& cache, const std::string& name, Creator create)
{
    auto search = cache.find(name);
    if (search == cache.end())
    {
        dlog("Render cache miss, creating name = %s", name.c_str());
        return cache[name] = std::shared_ptr<T>{ create() };
    }

    dlog("Render cache hit, name = %s", name.c_str());
    return search->second;
}
