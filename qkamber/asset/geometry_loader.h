#pragma once

#include "math3.h"

class GeometryAsset
{
public:
    struct Object
    {
        std::string name;
        std::vector<vec3> vertices;
        std::vector<Color> colors;
        std::vector<vec3> normals;
        std::vector<vec2> texcoords;
        std::vector<uint16_t> indices;
        int material_index;
    };
    using Objects = std::vector<Object>;

    struct Material
    {
        std::string name;
        Color ambient, diffuse, specular, emissive;
        float specular_shininess;
        std::string tex_filename;
    };
    using Materials = std::vector<Material>;

public:
    virtual const Objects& get_objects() const = 0;
    virtual const Materials& get_materials() const = 0;
};

class GeometryLoader
{
public:
    enum class FileFormat
    {
        Max3ds,
        Unknown
    };

public:
    GeometryLoader();
    ~GeometryLoader();

    std::unique_ptr<GeometryAsset> load(const std::string& filename, FileFormat format = FileFormat::Unknown);

private:
    FileFormat get_format(const std::string& filename);
    std::unique_ptr<GeometryAsset> load_3ds(const std::string& filename);
};

///////////////////////////////////////////////////////////////////////////////
// impl
///////////////////////////////////////////////////////////////////////////////
inline GeometryLoader::GeometryLoader()
{
    flog("id = %#x", this);
    log_info("Created geometry loader");
}

inline GeometryLoader::~GeometryLoader()
{
    flog();
    log_info("Destroyed geometry loader");
}
