struct texture_editor_pixel_buffer
{
    uint32 *Pixels;
    uint32 Width;
    uint32 Height;
};

struct pixel_location
{
    int32 X;
    int32 Y;
};

inline 
void IncrementPixel(int32 *Pixel, uint32 PixelBufferDimensionLength)
{
    *Pixel += 1; 

    if (*Pixel > ((int8)PixelBufferDimensionLength - 1))
    {
        *Pixel = 0;
    }
}
       
inline
void DecrementPixel(int32 *Pixel, uint32 PixelBufferDimensionLength)
{
    *Pixel -= 1; 

    if (*Pixel < 0)
    {
        *Pixel = ((int8)PixelBufferDimensionLength - 1);
    }
}

internal void
FillPixelAtLocationInPixelBufferWithColor(texture_editor_pixel_buffer *PixelBuffer, 
                                          pixel_location *PixelLocation, uint32 SelectedColor)
{
    uint32 *PixelRow = PixelBuffer->Pixels;
    PixelRow += PixelBuffer->Width*PixelLocation->Y;
    uint32 *Pixel = PixelRow;
    Pixel += PixelLocation->X;
    *Pixel = SelectedColor;
}

internal void
FillPixelBufferWithSingleColor(texture_editor_pixel_buffer *PixelBuffer, 
                               uint32 Color)
{
    uint32 *Dest = PixelBuffer->Pixels; 
    uint32 IterationCount = PixelBuffer->Width*PixelBuffer->Height;

    for (uint32 Index = 0; 
         Index < IterationCount; 
         Index++)
    {
        *Dest++ = Color;
    }
}
