#pragma once

#include "math3.h"

class AssetSystem;
class ImageAsset;

class GeometryAsset
{
public:
    struct Object
    {
        std::string name;
        std::string material_name;

        std::vector<vec3> vertices;
        std::vector<Color> colors;
        std::vector<vec3> normals;
        std::vector<vec2> texcoords;
        std::vector<uint16_t> indices;
    };
    using Objects = std::vector<Object>;

    struct Material
    {
        std::string name;
        Color ambient, diffuse, specular, emissive;
        float shininess;
        std::shared_ptr<ImageAsset> diffuse_map;
    };
    using Materials = std::vector<Material>;

public:
    virtual const std::string& get_name() const = 0;
    virtual const Objects& get_objects() const = 0;
    virtual const Materials& get_materials() const = 0;
};

class GeometryLoader
{
public:
    enum class FileFormat
    {
        Prefab,
        Max3ds,
        Unknown
    };

public:
    GeometryLoader(AssetSystem& asset);
    ~GeometryLoader();

    std::unique_ptr<GeometryAsset> load(const std::string& name, FileFormat format = FileFormat::Unknown);

private:
    FileFormat get_format(const std::string& name);
    std::unique_ptr<GeometryAsset> load_prefab(const std::string& name);
    std::unique_ptr<GeometryAsset> load_3ds(const std::string& name);

    AssetSystem& m_asset;
};

///////////////////////////////////////////////////////////////////////////////
// impl
///////////////////////////////////////////////////////////////////////////////
inline GeometryLoader::GeometryLoader(AssetSystem& asset) :
    m_asset(asset)
{
    flog("id = %#x", this);
    log_info("Created geometry loader");
}

inline GeometryLoader::~GeometryLoader()
{
    flog();
    log_info("Destroyed geometry loader");
}
