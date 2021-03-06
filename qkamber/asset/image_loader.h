#pragma once

class AssetSystem;

enum class ImageFormat
{
    Rgba8,
    Rgb8
};

class ImageAsset
{
public:
    virtual ImageFormat get_format() const = 0;
    virtual uint8_t* data() const = 0;

    virtual size_t get_width() const = 0;
    virtual size_t get_height() const = 0;
};

class ImageLoader
{
public:
    enum class FileFormat
    {
        Bmp,
        Unknown
    };

public:
    ImageLoader(AssetSystem& asset);
    ~ImageLoader();

    std::unique_ptr<ImageAsset> load(const std::string& filename, FileFormat format = FileFormat::Unknown);

private:
    FileFormat get_format(const std::string& filename);
    std::unique_ptr<ImageAsset> load_bmp(const std::string& filename);

    AssetSystem& m_asset;
};

///////////////////////////////////////////////////////////////////////////////
// impl
///////////////////////////////////////////////////////////////////////////////
inline ImageLoader::ImageLoader(AssetSystem& asset) :
    m_asset(asset)
{
    flog("id = %#x", this);
    log_info("Created image loader");
}

inline ImageLoader::~ImageLoader()
{
    flog();
    log_info("Destroyed image loader");
}