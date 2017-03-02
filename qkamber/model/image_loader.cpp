
#include "precompiled.h"
#include "image_loader.h"

using namespace std;

unique_ptr<Image> ImageLoader::load(const std::string& filename, FileFormat format)
{
    flog();

    // if the format was unspecified or default value, try to guess
    if (format == FileFormat::Unknown)
        format = get_format(filename);

    switch (format)
    {
        case FileFormat::Bmp:
            return load_bmp(filename);
    }

    throw exception("unknown file format");
}

namespace
{
    constexpr uint16_t BitmapSignature = 0x4d42;

    class BmpImage : public Image
    {
    #pragma pack(push)
    #pragma pack(1)
        struct FileHeader
        {
            uint16_t signature;
            uint32_t size;
            uint32_t reserved;
            uint32_t bits_offset;
        };

        struct BitmapHeader
        {
            uint32_t header_size;
            int32_t width;
            int32_t height;
            uint16_t plane_count;
            uint16_t bit_count;
            uint32_t compression;
            uint32_t image_size;
            int32_t horz_resolution;
            int32_t vert_resolution;
            uint32_t palette_color_count;
            uint32_t palette_important_count;
        };
    #pragma pack(pop)

    public:
        BmpImage(const string& filename);
        ~BmpImage() = default;

        ImageFormat get_format() const;
        uint8_t* data() const;

        size_t get_width() const;
        size_t get_height() const;
        size_t get_stride() const;

        static bool is_valid(const string& filename);

    private:
        void read_file_header(ifstream& fin);
        void read_bitmap_header(ifstream& fin);
        void read_data(ifstream& fin);

        FileHeader m_fh;
        BitmapHeader m_bh;
        std::unique_ptr<uint8_t[]> m_data;
        size_t m_stride;
    };

    inline BmpImage::BmpImage(const string& filename)
    {
        flog("id = %#x", this);

        ifstream fin(filename, ios::binary);
        if (!fin)
            throw exception("cannot open file");

        read_file_header(fin);
        read_bitmap_header(fin);
        read_data(fin);
    }

    inline ImageFormat BmpImage::get_format() const
    {
        // TODO: only supports rgba8 images atm
        return ImageFormat::Rgba8;
    }

    inline uint8_t* BmpImage::data() const
    {
        return m_data.get();
    }

    inline size_t BmpImage::get_width() const
    {
        return static_cast<size_t>(m_bh.width);
    }

    inline size_t BmpImage::get_height() const
    {
        return static_cast<size_t>(m_bh.height);
    }

    inline size_t BmpImage::get_stride() const
    {
        return m_stride;
    }

    inline bool BmpImage::is_valid(const string& filename)
    {
        flog();

        ifstream fin(filename, ios::binary);
        FileHeader fh;
        fin.read(reinterpret_cast<char*>(&fh), sizeof(FileHeader));

        if (fin.eof() || fin.fail())
        {
            // filesize is too small, could not read header
            return false;
        }
        return fh.signature == BitmapSignature;
    }

    inline void BmpImage::read_file_header(ifstream& fin)
    {
        fin.seekg(0, ios::beg);
        fin.read(reinterpret_cast<char*>(&m_fh), sizeof(FileHeader));
        if (fin.eof() || fin.fail())
            throw exception("invalid file size");

        if (m_fh.signature != BitmapSignature)
            throw exception("invalid bitmap signature");

        // check full bitmap size in order to skip further errors
        fin.seekg(0, ios::end);
        if (static_cast<uint32_t>(fin.tellg()) != m_fh.size)
            throw exception("invalid file size");
        fin.seekg(sizeof(FileHeader), ios::beg);
    }

    inline void BmpImage::read_bitmap_header(ifstream& fin)
    {
        fin.seekg(sizeof(FileHeader), ios::beg);
        fin.read(reinterpret_cast<char*>(&m_bh), sizeof(BitmapHeader));

        if (m_bh.bit_count != 24 && m_bh.bit_count != 32)
            throw exception(print_fmt("unsupported bitcount %d, must be 24/32", m_bh.bit_count).c_str());

        if (m_bh.compression != 0)
            throw exception("unsupported compression type, must be uncompressed/0");

        m_stride = ((m_bh.width * 4) + 3) & ~3;
    }

    inline void BmpImage::read_data(ifstream& fin)
    {
        fin.seekg(m_fh.bits_offset, ios::beg);
        m_data.reset(new uint8_t[m_bh.height * m_stride]);

        size_t file_stride = ((m_bh.width * m_bh.bit_count / 8) + 3) & ~3;
        std::unique_ptr<uint8_t[]> read_buf{ new uint8_t[file_stride] };

        uint8_t* pdata = m_data.get() + (m_bh.height - 1) * m_stride;
        for (int y = 0; y < m_bh.height; y++)
        {
            uint8_t* pbuf = read_buf.get();
            fin.read(reinterpret_cast<char*>(pbuf), file_stride);

            // TODO: extract this somewhat dup code
            uint8_t* prow = pdata;
            if (m_bh.bit_count == 24)
            {
                for (int x = 0; x < m_bh.width; x++)
                {
                    // convert BGR to RGB, fully opaque
                    prow[0] = pbuf[2];
                    prow[1] = pbuf[1];
                    prow[2] = pbuf[0];
                    prow[3] = 0xff;

                    prow += 4;
                    pbuf += 3;
                }
            }
            else
            {
                for (int x = 0; x < m_bh.width; x++)
                {
                    // convert BGRA to RGBA
                    prow[0] = pbuf[2];
                    prow[1] = pbuf[1];
                    prow[2] = pbuf[0];
                    prow[3] = pbuf[3];

                    prow += 4;
                    pbuf += 4;
                }
            }

            // bitmap is stored with mirrored y
            pdata -= m_stride;
        }
    }
}

ImageLoader::FileFormat ImageLoader::get_format(const std::string& filename)
{
    if (BmpImage::is_valid(filename))
        return FileFormat::Bmp;

    return FileFormat::Unknown;
}

unique_ptr<Image> ImageLoader::load_bmp(const std::string& filename)
{
    flog();
    log_info("Loading bitmap filename = %s...", filename.c_str());
    return unique_ptr<Image>{ new BmpImage{ filename } };
}
