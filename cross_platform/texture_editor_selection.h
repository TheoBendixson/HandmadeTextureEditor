
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

internal void
FillRectangleWithColor(texture_editor_selection *Selection, texture_editor_pixel_buffer *PixelBuffer,
                       uint32 SelectedColor)
{
    uint32 StartPixelY = 0;
    uint32 EndPixelY = 0;

    if (Selection->RectangleStart.Y < Selection->RectangleEnd.Y)
    {
        StartPixelY = Selection->RectangleStart.Y;
        EndPixelY = Selection->RectangleEnd.Y;
    } else 
    {
        StartPixelY = Selection->RectangleEnd.Y;
        EndPixelY = Selection->RectangleStart.Y;
    }

    uint32 StartPixelX = 0;
    uint32 EndPixelX = 0;

    if (Selection->RectangleStart.X < Selection->RectangleEnd.X)
    {
        StartPixelX = Selection->RectangleStart.X;
        EndPixelX = Selection->RectangleEnd.X;
    } else 
    {
        StartPixelX = Selection->RectangleEnd.X;
        EndPixelX = Selection->RectangleStart.X;
    }

    uint32 XDiff = EndPixelX - StartPixelX;
    uint32 YDiff = EndPixelY - StartPixelY;

    uint32 *PixelRow = PixelBuffer->Pixels;
    PixelRow += PixelBuffer->Width*StartPixelY;

    for (uint32 RowIteration = 0; YDiff >= RowIteration ; RowIteration++)
    {
        uint32 *Pixel = PixelRow;
        Pixel += StartPixelX;

        for (uint32 ColumnIteration = 0; XDiff >= ColumnIteration; ColumnIteration++)
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
