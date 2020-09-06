enum texture_file_type
{
    TextureFileTypeNoHeader,
    TextureFileTypeWidthHeight,
    TextureFileTypeUnknown
};

struct texture_file_header
{
    uint32 FileSize;
    uint32 TextureWidth;
    uint32 TextureHeight;
    uint32 PixelBufferOffset;
};
