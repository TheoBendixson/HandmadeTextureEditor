/*
    texture_editor.h

    Platformer Texture Editor
    2020 Ted Bendixson
    Mooselutions, LLC

    All Rights Reserved
*/

#define Assert(Expression) if(!(Expression)) { __builtin_trap(); }

#include "memory_arena.h"

struct read_file_result 
{
    void *Contents;
    uint64 ContentsSize;
    char *Filename;
};

read_file_result PlatformOpenFileDialog();

void PlatformFreeFileMemory(void *Memory);
read_file_result PlatformReadEntireFile(char *Filename);
bool32 PlatformWriteEntireFile(uint64 FileSize, void *Memory);

void PlatformUpdatePreviewTextureSize(uint32 Width, uint32 Height);

// TODO: (Ted)  texture editor vertex types are not yet defined for other platforms.
#define Kilobytes(Value) ((Value)*1024LL)
#define Megabytes(Value) (Kilobytes(Value)*1024LL)
#define Gigabytes(Value) (Megabytes(Value)*1024LL)

struct texture_editor_memory
{
    bool32 IsInitialized;
    uint64 PermanentStorageSize;
    // NOTE: (Ted)  This must be cleared to zero at startup!!!
    void *PermanentStorage;
};

#include "texture_editor_input.h"
#include "texture_editor_file.h"
#include "texture_editor_pixel.h"
#include "texture_editor_selection.h"
#include "texture_editor_renderer.h"
#include "texture_editor_color.h"
#include "texture_editor_color_palette.h"

enum pixel_buffer_mode
{
    PixelBufferModeSixteen,
    PixelBufferModeTwentyFour,
    PixelBufferModeThirtyTwo
};

enum preview_mode
{
    PreviewModeSprite,
    PreviewModeTile
};

struct texture_editor_state
{
    memory_arena TextureArena;
    uint8* StartOfSaveFileSerializedData;

    texture_editor_pixel_buffer PixelBuffer;
    pixel_buffer_mode PixelBufferMode;

    texture_editor_selection Selection;

    int8 SelectedColorIndex;

    color_palette ColorPalette;

    uint8 ActionSlopFrames;
    uint8 NavigationSlopFrames;

    preview_mode PreviewMode;

};

