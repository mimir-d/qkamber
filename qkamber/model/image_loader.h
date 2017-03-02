#pragma once

enum class ImageFormat
{
    Rgba8
};

class Image
{
public:
    virtual ImageFormat get_format() const = 0;
    virtual uint8_t* data() const = 0;

    virtual size_t get_width() const = 0;
    virtual size_t get_height() const = 0;
    virtual size_t get_stride() const = 0;
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
    std::unique_ptr<Image> load(const std::string& filename, FileFormat format = FileFormat::Unknown);

private:
    FileFormat get_format(const std::string& filename);
    std::unique_ptr<Image> load_bmp(const std::string& filename);
};
