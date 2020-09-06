
internal 
texture_editor_color ConvertUint32ToEditorColor(uint32 UColor)
{
    uint32 SourceAlpha = ((UColor >> 24) & 0xFF);
    uint32 SourceRed = ((UColor >> 16) & 0xFF);
    uint32 SourceGreen = ((UColor >> 8) & 0xFF);
    uint32 SourceBlue = ((UColor >> 0) & 0xFF);

    real32 DestBlue = (real32)(SourceBlue / 255.0f);
    real32 DestGreen = (real32)(SourceGreen / 255.0f);
    real32 DestRed = (real32)(SourceRed / 255.0f);
    real32 DestAlpha = (real32)(SourceAlpha / 255.0f);

    texture_editor_color Result = { DestRed, DestGreen, DestBlue, DestAlpha };

    return (Result);
}
