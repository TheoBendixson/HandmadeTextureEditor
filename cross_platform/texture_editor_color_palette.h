/*
    texture_editor_color_palette.h

    Platformer Texture Editor
    2020 Ted Bendixson
    Mooselutions, LLC

    All Rights Reserved
*/
enum color_palette_type
{
    ColorPaletteTypeGameboyGreyscale,
    ColorPaletteTypeZoneColors,
};

struct color_palette
{
    uint32 Colors [20];
    int8 ColorCount;
    color_palette_type Type;
};

internal color_palette
GetColorPalette(color_palette_type Type)
{
    color_palette Palette = {};
    Palette.Type = Type;

    if (Type == ColorPaletteTypeGameboyGreyscale)
    {
        // Black
        Palette.Colors[0] = 0xFF000000;

        // 33% Brightness
        Palette.Colors[1] = 0xFF545454;

        // 67% Brightness
        Palette.Colors[2] = 0xFFAAAAAA;

        // White
        Palette.Colors[3] = 0xFFFFFFFF;
        Palette.ColorCount = 4;

    } else if (Type == ColorPaletteTypeZoneColors)
    {
        // Light Blue
        Palette.Colors[0] = 0xFF83FBFF;

        // Light Red
        Palette.Colors[1] = 0xFFFF8883;

        // Light Green
        Palette.Colors[2] = 0xFF83FF88;

        // Light Purple
        Palette.Colors[3] = 0xFFFF83FB;
        Palette.ColorCount = 4;
    }

    return Palette;
}
