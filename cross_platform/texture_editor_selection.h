
enum selection_mode
{
    SelectionModeSinglePixel,
    SelectionModeRectangle
};

enum selection_state
{
    SelectionStateStartPixel,
    SelectionStateEndPixel,
    SelectionStateSelected
};

struct texture_editor_selection
{
    selection_mode Mode;
    selection_state State;

    pixel_location SinglePixelLocation;

    pixel_location RectangleStart;
    pixel_location RectangleEnd;
};

internal texture_editor_selection
CreateInitializedSelection()
{
    texture_editor_selection Selection = {};
    Selection.Mode = SelectionModeSinglePixel;
    Selection.State = SelectionStateStartPixel;

    pixel_location Default = {};
    Default.X = 0;
    Default.Y = 0;
    Selection.SinglePixelLocation = Default;
    Selection.RectangleStart = Default;
    Selection.RectangleEnd = Default;

    return Selection;
}

struct pixel_buffer_draw_region
{
    uint32 StartPixel;
    uint32 EndPixel;
};

internal pixel_buffer_draw_region 
DrawRegionFromSelection(uint32 StartPixel, uint32 EndPixel)
{
    pixel_buffer_draw_region Result = {};
    Result.StartPixel = 0;
    Result.EndPixel = 0;

    if (StartPixel < EndPixel)
    {
        Result.StartPixel = StartPixel;
        Result.EndPixel = EndPixel;
    } else 
    {
        Result.StartPixel = EndPixel;
        Result.EndPixel = StartPixel;
    }

    return (Result);
}

internal void
FillRectangleWithColor(texture_editor_selection *Selection, texture_editor_pixel_buffer *PixelBuffer,
                       uint32 SelectedColor)
{
    pixel_buffer_draw_region YDrawRegion = DrawRegionFromSelection(Selection->RectangleStart.Y, 
                                                                   Selection->RectangleEnd.Y);

    pixel_buffer_draw_region XDrawRegion = DrawRegionFromSelection(Selection->RectangleStart.X, 
                                                                   Selection->RectangleEnd.X);

    uint32 XDiff = XDrawRegion.EndPixel - XDrawRegion.StartPixel;
    uint32 YDiff = YDrawRegion.EndPixel - YDrawRegion.StartPixel;

    uint32 *PixelRow = PixelBuffer->Pixels;
    PixelRow += PixelBuffer->Width*YDrawRegion.StartPixel;

    for (uint32 RowIteration = 0; 
         YDiff >= RowIteration; 
         RowIteration++)
    {
        uint32 *Pixel = PixelRow;
        Pixel += XDrawRegion.StartPixel;

        for (uint32 ColumnIteration = 0; 
             XDiff >= ColumnIteration; 
             ColumnIteration++)
        {
            *Pixel++ = SelectedColor;
        }

        PixelRow += PixelBuffer->Width;
    }
}

internal void
FillWithColorUsingSelection(texture_editor_selection *Selection, texture_editor_pixel_buffer *PixelBuffer, 
                            uint32 Color)
{
    if (Selection->Mode == SelectionModeSinglePixel)
    {
        FillPixelAtLocationInPixelBufferWithColor(PixelBuffer, &Selection->SinglePixelLocation, 
                                                  Color);
    } else if (Selection->Mode == SelectionModeRectangle &&
               Selection->State == SelectionStateSelected)
    {
        FillRectangleWithColor(Selection, PixelBuffer, Color);
    }
}
