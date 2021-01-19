
#define COPY_PASTE_BUFFER_MAX_SIZE  4096

struct texture_editor_copy_paste_buffer
{
    bool32 IsFilled;
    uint32 Width;
    uint32 Height;
    uint32 Pixels[COPY_PASTE_BUFFER_MAX_SIZE];
};
