/*
    texture_editor_renderer.h
    Platformer Texture Editro

    2020 Ted Bendixson
    Mooselutions, LLC
    All Rights Reserved

 */


#if CLANG
typedef vector_float2 texture_editor_2d_vertex;
typedef vector_float2 texture_editor_2d_texture_coordinate;
typedef vector_float4 texture_editor_color;
#endif

struct texture_editor_texture
{
    uint32 *Data;
    uint32 Type;
};

struct texture_editor_texture_buffer
{
    texture_editor_texture Textures[50];
    uint32 NumberOfTextures;
    uint32 TexturesLoaded;
};

struct texture_editor_color_vertex
{
    texture_editor_2d_vertex Position;
    texture_editor_color Color;
};

struct texture_editor_texture_vertex
{
    texture_editor_2d_vertex Position;
    texture_editor_2d_texture_coordinate TextureCoordinate;
    uint32 TextureID;
};

struct color_vertex_command_buffer
{
    texture_editor_color_vertex *ColorVertices;
    uint32 NumberOfColorVertices;
};

struct texture_vertex_command_buffer
{
    texture_editor_texture_vertex *TextureVertices;
    uint32 NumberOfTextureVertices;
};

struct texture_editor_render_commands
{
    color_vertex_command_buffer ColorVertexCommandBuffers[3];
    texture_vertex_command_buffer TextureVertexCommandBuffers[3];

    uint32 FrameIndex;

    uint32 PreviewTextureWidth;
    uint32 PreviewTextureHeight;

    int ViewportWidth;
    int ViewportHeight;

    uint32 UniqueTextureCount;
};

struct y_component
{
    real32 Min;
    real32 Max;
};

inline
y_component InvertYAxis(int ViewportHeight, 
                        real32 YMin, real32 YMax)
{
    y_component Result = {};
    Result.Min = (real32)(ViewportHeight - YMin);
    Result.Max = (real32)(ViewportHeight - YMax);
    return (Result);
}

void
DrawRectangle(texture_editor_render_commands *RenderCommands, 
              v2 vMin, v2 vMax, texture_editor_color Color)
{

    texture_editor_color_vertex QuadVertices[] =
    {
        // Pixel positions, Color
        { { vMin.X, vMin.Y }, Color },
        { { vMin.X, vMax.Y }, Color },
        { { vMax.X, vMax.Y }, Color },

        { { vMin.X, vMin.Y }, Color },
        { { vMax.X, vMin.Y }, Color },
        { { vMax.X, vMax.Y }, Color }
    };

    color_vertex_command_buffer ColorCommandBuffer = RenderCommands->ColorVertexCommandBuffers[RenderCommands->FrameIndex];

    texture_editor_color_vertex *Source = QuadVertices;
    texture_editor_color_vertex *Dest = ColorCommandBuffer.ColorVertices + ColorCommandBuffer.NumberOfColorVertices;

    for (uint32 Index = 0; Index < 6; Index++)
    {
        *Dest++ = *Source++;
        ColorCommandBuffer.NumberOfColorVertices++;
    }

    RenderCommands->ColorVertexCommandBuffers[RenderCommands->FrameIndex] = ColorCommandBuffer;
}

// NOTE: (Ted)  This is texture drawing where the texture y axis starts at zero, and moves up
//              while the x axis starts at 0 and moves right.
//
//              Spatially, the texture is drawn with inverted y-coordinates to fit the inverted
//              y-coordinates in the game's physics system.
void
DrawTexturedRectangleYAxisInverted(texture_editor_render_commands *RenderCommands, 
                                   v2 vMin, v2 vMax, uint32 TextureID)
{

    texture_editor_texture_vertex QuadVertices[] =
    {
        // Pixel positions, Texture coordinates, TextureID
        { { vMin.X, vMax.Y }, { 0.0f, 1.0f }, TextureID },
        { { vMin.X, vMin.Y }, { 0.0f, 0.0f }, TextureID },
        { { vMax.X, vMax.Y }, { 1.0f, 1.0f }, TextureID },

        { { vMax.X, vMax.Y }, { 1.0f, 1.0f }, TextureID },
        { { vMin.X, vMin.Y }, { 0.0f, 0.0f }, TextureID },
        { { vMax.X, vMin.Y }, { 1.0f, 0.0f }, TextureID },
    };

    texture_vertex_command_buffer TextureCommandBuffer = RenderCommands->TextureVertexCommandBuffers[RenderCommands->FrameIndex];

    texture_editor_texture_vertex *Source = QuadVertices;
    texture_editor_texture_vertex *Dest = TextureCommandBuffer.TextureVertices + TextureCommandBuffer.NumberOfTextureVertices;

    for (uint32 Index = 0; Index < 6; Index++)
    {
        *Dest++ = *Source++;
        TextureCommandBuffer.NumberOfTextureVertices++;
    }

    RenderCommands->TextureVertexCommandBuffers[RenderCommands->FrameIndex] = TextureCommandBuffer;
}

internal void
DrawBorderedSquare(texture_editor_render_commands *RenderCommands, 
                   v2 Min, real32 BorderWidth, real32 SquareWidth, 
                   texture_editor_color BorderColor, texture_editor_color SquareColor)
{
    real32 BorderMaxX = Min.X + SquareWidth;
    real32 BorderMaxY = Min.Y + SquareWidth;

    v2 BorderMax = { BorderMaxX, BorderMaxY };
 
    DrawRectangle(RenderCommands, Min, 
                  BorderMax, BorderColor);

    real32 BorderedItemWidth = SquareWidth - 2*BorderWidth;

    real32 SquareMinX = Min.X + BorderWidth;
    real32 SquareMaxX = SquareMinX + BorderedItemWidth;
    real32 SquareMinY = Min.Y + BorderWidth;
    real32 SquareMaxY = SquareMinY + BorderedItemWidth;

    v2 SquareMin = { SquareMinX, SquareMinY };
    v2 SquareMax = { SquareMaxX, SquareMaxY }; 

    DrawRectangle(RenderCommands, SquareMin, 
                  SquareMax, SquareColor);
}

internal void
DrawSelectionBetweenPixelLocations(texture_editor_render_commands *RenderCommands,
                                   real32 PixelWidth, real32 XStartingPoint, real32 YStartingPoint,
                                   pixel_location *Start, pixel_location *End,
                                   texture_editor_color BorderColor)
{
    real32 MinX = 0.0f;
    real32 MaxX = 0.0f;

    if (Start->X < End->X)
    {
        MinX = XStartingPoint + (real32)(Start->X*PixelWidth);
        MaxX = XStartingPoint + (real32)(End->X*PixelWidth) + PixelWidth;
    } else
    {
        MinX = XStartingPoint + (real32)(End->X*PixelWidth);
        MaxX = XStartingPoint + (real32)(Start->X*PixelWidth) + PixelWidth;
    }

    real32 MinY = 0.0f;
    real32 MaxY = 0.0f;

    if (Start->Y < End->Y)
    {
        MinY = YStartingPoint + (real32)(Start->Y*PixelWidth);
        MaxY = YStartingPoint + (real32)(End->Y*PixelWidth) + PixelWidth;
    } else 
    {
        MinY = YStartingPoint + (real32)(End->Y*PixelWidth);
        MaxY = YStartingPoint + (real32)(Start->Y*PixelWidth) + PixelWidth;
    }

    v2 Min = { MinX, MinY };
    v2 Max = { MaxX, MaxY };

    DrawRectangle(RenderCommands, Min, Max, BorderColor);
}
