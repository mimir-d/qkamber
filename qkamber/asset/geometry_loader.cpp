
#include "precompiled.h"
#include "geometry_loader.h"

#include "asset_system.h"

using namespace std;

unique_ptr<GeometryAsset> GeometryLoader::load(const std::string& filename, FileFormat format)
{
    flog();

    // if the format was unspecified or default value, try to guess
    if (format == FileFormat::Unknown)
        format = get_format(filename);

    switch (format)
    {
        case FileFormat::Prefab:
            return load_prefab(filename);

        case FileFormat::Max3ds:
            return load_3ds(filename);
    }

    throw exception("unknown file format");
}

namespace
{
    ///////////////////////////////////////////////////////////////////////////
    // PrefabGeometry
    ///////////////////////////////////////////////////////////////////////////
    class PrefabGeometry : public GeometryAsset
    {
    public:
        PrefabGeometry(AssetSystem& asset, const string& name);
        ~PrefabGeometry() = default;

        const string& get_name() const final;
        const Objects& get_objects() const final;
        const Materials& get_materials() const final;

    private:
        void make_cubes(AssetSystem& asset, const string& name);
        void make_tris(AssetSystem& asset, const string& name);

    private:
        string m_name;
        Objects m_objects;
        Materials m_materials;
    };

    ///////////////////////////////////////////////////////////////////////////
    // PrefabGeometry impl
    ///////////////////////////////////////////////////////////////////////////
    PrefabGeometry::PrefabGeometry(AssetSystem& asset, const string& name) :
        m_name(name)
    {
        string model_name = basename(name);

        // NOTE: hardcoded names intentional
        if (model_name == "color_cube" || model_name == "tex_cube")
            make_cubes(asset, name);
        else if (model_name == "triangle" || model_name == "axis_triangle")
            make_tris(asset, name);
        else
            throw exception(print_fmt("unknown static geometry, name = %s", name.c_str()).c_str());
    }

    const string& PrefabGeometry::get_name() const
    {
        return m_name;
    }

    const GeometryAsset::Objects& PrefabGeometry::get_objects() const
    {
        return m_objects;
    }

    const GeometryAsset::Materials& PrefabGeometry::get_materials() const
    {
        return m_materials;
    }

    void PrefabGeometry::make_cubes(AssetSystem& asset, const string& name)
    {
        m_objects.emplace_back();
        m_materials.emplace_back();

        Object& obj = m_objects[m_objects.size() - 1];
        obj.name = print_fmt("%s/mesh", name.c_str());

        const size_t face_indices[] =
        {
            0, 1, 2, 3, // front
            1, 5, 3, 7, // right
            5, 4, 7, 6, // back
            4, 0, 6, 2, // left
            4, 5, 0, 1, // top
            2, 3, 6, 7  // bottom
        };

        const vec3 vertices[] =
        {
            { -1.0f,  1.0f,  1.0f },
            { 1.0f,  1.0f,  1.0f },
            { -1.0f, -1.0f,  1.0f },
            { 1.0f, -1.0f,  1.0f },

            { -1.0f,  1.0f, -1.0f },
            { 1.0f,  1.0f, -1.0f },
            { -1.0f, -1.0f, -1.0f },
            { 1.0f, -1.0f, -1.0f }
        };

        const size_t vertex_count = sizeof(face_indices) / sizeof(face_indices[0]);

        // vertices
        obj.vertices.resize(vertex_count);
        obj.normals.resize(vertex_count);

        for (size_t i = 0; i < vertex_count; i++)
        {
            const size_t fi = face_indices[i];
            obj.vertices[i] = vertices[fi];

            // same normal for both face triangles, so compute only on indices 0,1,2
            const size_t fi0 = face_indices[(i & ~3) + 0];
            const size_t fi1 = face_indices[(i & ~3) + 1];
            const size_t fi2 = face_indices[(i & ~3) + 2];
            vec3 v10 = vertices[fi1] - vertices[fi0];
            vec3 v20 = vertices[fi2] - vertices[fi0];

            obj.normals[i] = (v20 ^ v10).normalize();
        }

        // indices
        const uint16_t indices[] = { 0,  2,  1,  1,  2,  3 };
        const size_t face_index_count = sizeof(indices) / sizeof(indices[0]);
        const size_t index_count = 6 * face_index_count;

        obj.indices.resize(index_count);

        for (size_t i = 0; i < index_count; i++)
        {
            const size_t fi = i / 6;
            obj.indices[i] = static_cast<uint16_t>(4 * fi) + indices[i % face_index_count];
        }

        // material
        Material& mat = m_materials[m_materials.size() - 1];
        mat.name = print_fmt("%s/material", name.c_str());
        mat.ambient = Color(0.05f, 0.05f, 0.05f, 1.0f);
        mat.diffuse = Color(1.0f, 1.0f, 1.0f, 1.0f);

        obj.material_name = mat.name;

        if (name == ":prefab/color_cube")
        {
            obj.colors.resize(vertex_count);

            const Color colors[] =
            {
                { 1.0f, 0.0f, 0.0f, 1.0f },
                { 0.0f, 1.0f, 0.0f, 1.0f },
                { 0.0f, 0.0f, 1.0f, 1.0f },
                { 1.0f, 1.0f, 1.0f, 1.0f },

                { 0.0f, 0.0f, 0.0f, 1.0f },
                { 0.0f, 0.0f, 0.5f, 1.0f },
                { 0.0f, 0.5f, 0.0f, 1.0f },
                { 0.5f, 0.0f, 0.0f, 1.0f },
            };

            for (size_t i = 0; i < vertex_count; i++)
            {
                const size_t fi = face_indices[i];
                obj.colors[i] = colors[fi];
            }
        }
        else
        {
            obj.texcoords.resize(vertex_count);
            mat.diffuse_map = asset.load_image("check_tex256.bmp");

            const vec2 uvs[] =
            {
                // all faces have same uv
                { 0.0f, 1.0f },
                { 1.0f, 1.0f },
                { 0.0f, 0.0f },
                { 1.0f, 0.0f }
            };

            for (size_t i = 0; i < vertex_count; i++)
                obj.texcoords[i] = uvs[i % 4];
        }
    }

    void PrefabGeometry::make_tris(AssetSystem& asset, const string& name)
    {
        m_objects.emplace_back();
        m_materials.emplace_back();

        Object& obj = m_objects[m_objects.size() - 1];
        obj.name = print_fmt("%s/mesh", name.c_str());

        const Color colors[] =
        {
            { 1.0f, 0.0f, 0.0f, 1.0f },
            { 0.0f, 1.0f, 0.0f, 1.0f },
            { 0.0f, 0.0f, 1.0f, 1.0f }
        };

        // vertices
        const size_t vertex_count = 3;
        obj.vertices.resize(vertex_count);
        obj.colors.resize(vertex_count);
        obj.normals.resize(vertex_count);

        for (size_t i = 0; i < vertex_count; i++)
        {
            obj.colors[i] = colors[i];
            obj.normals[i] = vec3{ 0.0f, 0.0f, 1.0f };
        }

        // indices
        const size_t index_count = 3;
        obj.indices.resize(index_count);
        obj.indices[0] = 0;
        obj.indices[1] = 1;
        obj.indices[2] = 2;

        // material
        Material& mat = m_materials[m_materials.size() - 1];
        mat.name = print_fmt("%s/material", name.c_str());
        mat.ambient = Color(0.05f, 0.05f, 0.05f, 1.0f);
        mat.diffuse = Color(1.0f, 1.0f, 1.0f, 1.0f);

        obj.material_name = mat.name;

        if (name == ":static/triangle")
        {
            const vec3 vertices[] =
            {
                { -1.0f, -0.5773f, 0.0f },
                { 1.0f,  -0.5773f, 0.0f },
                { 0.0f,   1.1547f, 0.0f }
            };

            for (size_t i = 0; i < vertex_count; i++)
                obj.vertices[i] = vertices[i];
        }
        else
        {
            vec3 vertices[] =
            {
                { 0.0f, 0.0f, 0.0f },
                { 2.0f, 0.0f, 0.0f },
                { 0.0f, 1.0f, 0.0f }
            };

            for (size_t i = 0; i < vertex_count; i++)
                obj.vertices[i] = vertices[i];
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    // Max3dsGeometry
    ///////////////////////////////////////////////////////////////////////////
    constexpr uint16_t M3DS_PRIMARY = 0x4d4d;
    constexpr uint16_t M3DS_VERSION = 0x0002;
    constexpr uint16_t M3DS_SCENE = 0x3d3d;

    constexpr uint16_t M3DS_COLOR_FLOAT = 0x0010;
    constexpr uint16_t M3DS_COLOR_BYTE = 0x0011;

    constexpr uint16_t M3DS_MATERIAL = 0xafff;
    constexpr uint16_t M3DS_MATERIAL_NAME = 0xa000;
    constexpr uint16_t M3DS_MATERIAL_AMBIENT = 0xa010;
    constexpr uint16_t M3DS_MATERIAL_DIFFUSE = 0xa020;
    constexpr uint16_t M3DS_MATERIAL_SPECULAR = 0xa030;
    constexpr uint16_t M3DS_MATERIAL_TEX = 0xa200;
    constexpr uint16_t M3DS_MATERIAL_TEXFILE = 0xa300;
    // addressing modes and other options
    // constexpr uint16_t M3DS_MATERIAL_FILE_FLAGS = 0xa351;

    constexpr uint16_t M3DS_OBJECT = 0x4000;
    constexpr uint16_t M3DS_OBJECT_MESH = 0x4100;
    constexpr uint16_t M3DS_OBJECT_VERTICES = 0x4110;
    constexpr uint16_t M3DS_OBJECT_FACES = 0x4120;
    constexpr uint16_t M3DS_OBJECT_MATERIAL = 0x4130;
    constexpr uint16_t M3DS_OBJECT_TEXCOORD = 0x4140;
    constexpr uint16_t M3DS_OBJECT_SMOOTHING = 0x4150;

    class Max3dsGeometry : public GeometryAsset
    {
#pragma pack(push)
#pragma pack(1)
        struct ChunkHeader
        {
            uint16_t id;
            uint32_t length;
        };

        struct Face
        {
            uint16_t vertex_index[3];
            uint16_t flags;
        };
#pragma pack(pop)

        class Parser
        {
        public:
            Parser(AssetSystem& asset, const string& filename);
            ~Parser() = default;

            void parse(Objects& objects, Materials& materials);

        private:
            template <typename T>
            void read(T* buf, size_t count = 1);
            void skip(ChunkHeader& ch);

            string read_string();
            Color read_color();

            void read_version();
            void read_vertices();
            void read_faces();
            void compute_smoothing();
            void read_texcoords();
            void read_object_material();

        private:
            AssetSystem& m_asset;
            string m_prefix;
            ifstream m_file;

            vector<Face> m_faces;
            Object* m_obj;
            Material* m_mat;
        };

    public:
        Max3dsGeometry(AssetSystem& asset, const string& name);
        ~Max3dsGeometry() = default;

        const string& get_name() const final;
        const Objects& get_objects() const final;
        const Materials& get_materials() const final;

    private:
        string m_name;
        Objects m_objects;
        Materials m_materials;
    };

    ///////////////////////////////////////////////////////////////////////////
    // Max3dsGeometry impl
    ///////////////////////////////////////////////////////////////////////////

    Max3dsGeometry::Parser::Parser(AssetSystem& asset, const string& filename) :
        m_asset(asset),
        m_prefix(print_fmt("%s/", filename.c_str())),
        m_file(filename, ios::binary)
    {}

    void Max3dsGeometry::Parser::parse(Objects& objects, Materials& materials)
    {
        ChunkHeader ch;

        read(&ch);
        if (ch.id != M3DS_PRIMARY)
            throw exception("no primary chunk in 3ds file");

        while (m_file)
        {
            read(&ch);
            switch (ch.id)
            {
                case M3DS_VERSION:
                    read_version();
                    break;

                case M3DS_SCENE:
                    // there should be only one scene in the file, so it's already selected
                    dlog("3ds scene");
                    break;

                case M3DS_OBJECT:
                    objects.emplace_back();
                    m_obj = &objects[objects.size() - 1];
                    m_obj->name = m_prefix + read_string();
                    dlog("3ds object #%d, name = %s", objects.size(), m_obj->name.c_str());
                    break;

                case M3DS_OBJECT_MESH:
                    // object mesh is already selected
                    dlog("3ds object mesh");
                    break;

                case M3DS_OBJECT_VERTICES:
                    read_vertices();
                    dlog("3ds object vertices, count = %d", m_obj->vertices.size());
                    break;

                case M3DS_OBJECT_FACES:
                    read_faces();
                    dlog("3ds object faces, count = %d", m_faces.size());
                    break;

                case M3DS_OBJECT_SMOOTHING:
                    compute_smoothing();
                    dlog("3ds object smoothing group, count = %d", m_faces.size());
                    break;

                case M3DS_OBJECT_TEXCOORD:
                    read_texcoords();
                    dlog("3ds object texcoords, count = %d", m_obj->texcoords.size());
                    break;

                case M3DS_OBJECT_MATERIAL:
                    read_object_material();
                    dlog("3ds object material name = %s", m_obj->material_name);
                    break;

                case M3DS_MATERIAL:
                    materials.emplace_back();
                    m_mat = &materials[materials.size() - 1];
                    dlog("3ds material #%d", materials.size());
                    break;

                case M3DS_MATERIAL_NAME:
                    m_mat->name = m_prefix + read_string();
                    dlog("3ds material name = %s", m_mat->name.c_str());
                    break;

                case M3DS_MATERIAL_AMBIENT:
                    m_mat->ambient = read_color();
                    dlog("3ds material ambient %.3f %.3f %.3f", m_mat->ambient.r(), m_mat->ambient.g(), m_mat->ambient.b());
                    break;

                case M3DS_MATERIAL_DIFFUSE:
                    m_mat->diffuse = read_color();
                    dlog("3ds material diffuse %.3f %.3f %.3f", m_mat->ambient.r(), m_mat->ambient.g(), m_mat->ambient.b());
                    break;

                case M3DS_MATERIAL_SPECULAR:
                    m_mat->specular = read_color();
                    m_mat->shininess = 1.0f;
                    dlog("3ds material specular %.3f %.3f %.3f", m_mat->ambient.r(), m_mat->ambient.g(), m_mat->ambient.b());
                    break;

                case M3DS_MATERIAL_TEX:
                    dlog("3ds material texture map");
                    break;

                case M3DS_MATERIAL_TEXFILE:
                    m_mat->diffuse_map = m_asset.load_image(read_string());
                    //dlog("3ds material texture file = %s", m_mat->tex_filename.c_str());
                    break;

                default:
                    skip(ch);
                    break;
            }
        };
    }

    template <typename T>
    void Max3dsGeometry::Parser::read(T* buf, size_t count)
    {
        m_file.read(reinterpret_cast<char*>(buf), sizeof(T) * count);
    }

    void Max3dsGeometry::Parser::skip(ChunkHeader& ch)
    {
        m_file.seekg(ch.length - sizeof(ChunkHeader), ios::cur);
    }

    string Max3dsGeometry::Parser::read_string()
    {
        string ret(255, '\0');
        string::iterator it = ret.begin();
        for (size_t i = 0; i < 255; ++i, ++it)
        {
            read(&*it);
            if (!*it)
                break;
        }
        return ret;
    }

    Color Max3dsGeometry::Parser::read_color()
    {
        ChunkHeader ch;
        read(&ch);

        Color ret;
        if (ch.id == M3DS_COLOR_FLOAT)
        {
            read(&ret.r());
            read(&ret.g());
            read(&ret.b());
        }
        else if (ch.id == M3DS_COLOR_BYTE)
        {
            uint8_t rgb[3];
            read(rgb, 3);
            ret.r() = rgb[0] / 255.0f;
            ret.g() = rgb[1] / 255.0f;
            ret.b() = rgb[2] / 255.0f;
        }
        else
        {
            throw exception("unknown 3ds color format");
        }

        ret.a() = 1.0f;
        return ret;
    }

    void Max3dsGeometry::Parser::read_version()
    {
        uint32_t version;
        read(&version);
        dlog("3ds file version = %d", version);
    }

    void Max3dsGeometry::Parser::read_vertices()
    {
        uint16_t count;
        read(&count);

        // some objects are invalid (lights, etc), ignore them
        if (count == 0)
            return;

        m_obj->vertices.resize(count);
        m_obj->normals.resize(count);

        read(&m_obj->vertices[0], count);

        // NOTE: 3ds keeps vertices in xyz = xz-y, so fix the coords
        for (auto& v : m_obj->vertices)
        {
            float y = v.y();
            v.y() = v.z();
            v.z() = -y;
        }
    }

    void Max3dsGeometry::Parser::read_faces()
    {
        uint16_t count;
        read(&count);

        m_faces.resize(count);
        read(&m_faces[0], count);

        for (auto& f : m_faces)
        {
            // compute normal for this face
            const vec3 v0 = m_obj->vertices[f.vertex_index[0]];
            const vec3 v1 = m_obj->vertices[f.vertex_index[1]];
            const vec3 v2 = m_obj->vertices[f.vertex_index[2]];

            const vec3 v10 = v1 - v0;
            const vec3 v20 = v2 - v0;
            const vec3 normal = (v10 ^ v20).normalize();

            // store per-vertex face normal (may overwrite if indices are repeated)
            m_obj->normals[f.vertex_index[0]] = normal;
            m_obj->normals[f.vertex_index[1]] = normal;
            m_obj->normals[f.vertex_index[2]] = normal;
        }
    }

    void Max3dsGeometry::Parser::compute_smoothing()
    {
        const size_t face_count = m_faces.size();
        vector<uint32_t> groups(face_count, 0);
        read(&groups[0], face_count);

        struct adjancency_node
        {
            size_t face_index;
            vec3 normal;
            adjancency_node* next = nullptr;
        };

        // build linked lists of adjancent vertices and sum the face normals
        vector<adjancency_node*> adj_heads{ m_obj->vertices.size() };
        vector<adjancency_node> adj_verts{ face_count * 3 };

        // compute face normals and vertex adjancency
        for (size_t i = 0; i < face_count; i++)
        {
            const auto& f = m_faces[i];
            // compute normal for this face
            const vec3 v0 = m_obj->vertices[f.vertex_index[0]];
            const vec3 v1 = m_obj->vertices[f.vertex_index[1]];
            const vec3 v2 = m_obj->vertices[f.vertex_index[2]];

            const vec3 v10 = v1 - v0;
            const vec3 v20 = v2 - v0;
            const vec3 face_normal = (v10 ^ v20).normalize();

            for (size_t j = 0; j < 3; j++)
            {
                const auto vi = f.vertex_index[j];

                // populate list node
                auto v = &adj_verts[3*i + j];
                v->face_index = i;
                v->normal = face_normal;

                // append to list
                v->next = adj_heads[vi];
                adj_heads[vi] = v;
            }
        }

        // use the smoothing groups
        for (size_t i = 0; i < face_count; i++)
        {
            const auto& f = m_faces[i];
            if (groups[i])
            {
                for (size_t j = 0; j < 3; j++)
                {
                    const auto vi = f.vertex_index[j];
                    uint32_t g = groups[i];

                    // get all tangent group mask
                    for (auto p = adj_heads[vi]; p; p = p->next)
                        if (groups[i] & groups[p->face_index])
                            g |= groups[p->face_index];

                    // sum normals per group mask
                    vec3 avg_normal;
                    for (auto p = adj_heads[vi]; p; p = p->next)
                        if (g & groups[p->face_index])
                            avg_normal += p->normal;

                    avg_normal = avg_normal.normalize();
                    m_obj->normals[f.vertex_index[0]] = avg_normal;
                    m_obj->normals[f.vertex_index[1]] = avg_normal;
                    m_obj->normals[f.vertex_index[2]] = avg_normal;
                }
            }
            else
            {
                m_obj->normals[f.vertex_index[0]] = adj_verts[3*i + 0].normal;
                m_obj->normals[f.vertex_index[1]] = adj_verts[3*i + 1].normal;
                m_obj->normals[f.vertex_index[2]] = adj_verts[3*i + 2].normal;
            }
        }
    }

    void Max3dsGeometry::Parser::read_texcoords()
    {
        uint16_t count;
        read(&count);

        m_obj->texcoords.resize(count);
        read(&m_obj->texcoords[0], count);

        // TODO: seems uvs are mirrored in 3ds, might be wrong
        for (auto& uv : m_obj->texcoords)
            uv.y() = 1.0f - uv.y();
    }

    void Max3dsGeometry::Parser::read_object_material()
    {
        m_obj->material_name = m_prefix + read_string();

        // NOTE: apparently 3ds might say to just render a subset of the vertices
        uint16_t face_count;
        read(&face_count);

        m_obj->indices.resize(face_count * 3);
        for (size_t i = 0; i < face_count; i++)
        {
            uint16_t face_index;
            read(&face_index);

            m_obj->indices[i * 3 + 0] = m_faces[face_index].vertex_index[0];
            m_obj->indices[i * 3 + 1] = m_faces[face_index].vertex_index[1];
            m_obj->indices[i * 3 + 2] = m_faces[face_index].vertex_index[2];
        }
    }

    Max3dsGeometry::Max3dsGeometry(AssetSystem& asset, const string& filename) :
        m_name(filename)
    {
        flog("id = %#x", this);
        Parser{ asset, filename }.parse(m_objects, m_materials);

        // TODO: this should be covered by config files
        if (filename.find("ship.3ds") != string::npos)
        {
            m_materials[0].specular = Color{ 1.0f, 1.0f, 1.0f, 1.0f };
            m_materials[0].shininess = 100.0f;
        }
    }

    const string& Max3dsGeometry::get_name() const
    {
        return m_name;
    }

    const GeometryAsset::Objects& Max3dsGeometry::get_objects() const
    {
        return m_objects;
    }

    const GeometryAsset::Materials& Max3dsGeometry::get_materials() const
    {
        return m_materials;
    }
}

GeometryLoader::FileFormat GeometryLoader::get_format(const std::string& name)
{
    if (name.substr(name.size() - 4, 4) == ".3ds")
        return GeometryLoader::FileFormat::Max3ds;

    if (dirname(name) == ":prefab")
        return FileFormat::Prefab;

    return FileFormat::Unknown;
}

unique_ptr<GeometryAsset> GeometryLoader::load_prefab(const std::string& name)
{
    flog();
    log_info("Loading prefab geometry, name = %s...", name.c_str());
    return unique_ptr<GeometryAsset>{ new PrefabGeometry{ m_asset, name } };
}

unique_ptr<GeometryAsset> GeometryLoader::load_3ds(const std::string& name)
{
    flog();
    log_info("Loading geometry filename = %s...", name.c_str());
    return unique_ptr<GeometryAsset>{ new Max3dsGeometry{ m_asset, name } };
}
