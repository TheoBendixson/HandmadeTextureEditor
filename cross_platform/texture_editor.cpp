/*
    texture_editor.cpp 
    Platformer Texture Editor

    2020 Ted Bendixson
    Mooselutions, LLC
    All Rights Reserved

 */

#include "texture_editor_math.h"
#include "texture_editor.h"

// TODO: (Ted)  
//              1.  Grid Mode.
//              2.  Move Entire Sprite Left / Right.


// TODO: (Ted)  Have this take the width and height of the texture into account.
//      
//              In all likelihood, this will load entire texture packs and not just a single texture
//              since that is more efficient.
internal void
LoadASingleTexture(texture_editor_memory *Memory, texture_editor_texture_buffer *TextureBuffer, 
                   char *TextureName)
{
    read_file_result ReadResult = PlatformReadEntireFile(TextureName);
    texture_editor_texture Texture = {};
    Texture.Data = (uint32 *)ReadResult.Contents;
    uint32 TexturesLoaded = TextureBuffer->TexturesLoaded;
    TextureBuffer->Textures[TexturesLoaded] = Texture; 
    TextureBuffer->TexturesLoaded++;
}


// TODO: (Ted)  Actually do something here. Keeping it as a placeholder for now.
internal void
LoadTextures(texture_editor_memory *Memory, texture_editor_texture_buffer *TextureBuffer)
{

}

internal void
InitializePixelBufferAndPreviewTexture(texture_editor_memory *Memory, texture_editor_state *EditorState,
                                       texture_editor_render_commands *RenderCommands,
                                       uint32 Width, uint32 Height)
{
    InitializeArena(&EditorState->TextureArena, Memory->PermanentStorageSize - sizeof(texture_editor_state),
                    (uint8*)Memory->PermanentStorage + sizeof(texture_editor_state));

    texture_editor_pixel_buffer PixelBuffer = {};
    PixelBuffer.Width = Width; 
    PixelBuffer.Height = Height; 
    uint32 PixelArraySize = PixelBuffer.Width*PixelBuffer.Height;
    PixelBuffer.Pixels = (uint32 *)PushArray(&EditorState->TextureArena, PixelArraySize, uint32);

    uint32 PixelBufferSize = PixelBuffer.Width*PixelBuffer.Height*sizeof(uint32); 
    uint32 SaveFileSize = sizeof(texture_file_header) + PixelBufferSize;
    EditorState->StartOfSaveFileSerializedData = (uint8 *)PushSize_(&EditorState->TextureArena, SaveFileSize);

    FillPixelBufferWithSingleColor(&PixelBuffer, 0xFFFFFFFF);
    EditorState->PixelBuffer = PixelBuffer;
    EditorState->Selection = CreateInitializedSelection();

    RenderCommands->PreviewTextureWidth = Width;
    RenderCommands->PreviewTextureHeight = Height;
    PlatformUpdatePreviewTextureSize(Width, Height);
}

internal
void UpdateAndRender(texture_editor_memory *Memory, texture_editor_input *Input, 
                     texture_editor_keyboard_input *Keyboard,
                     texture_editor_render_commands *RenderCommands)
{
    Assert(sizeof(texture_editor_state) <= Memory->PermanentStorageSize);

    texture_editor_state *EditorState = (texture_editor_state *)Memory->PermanentStorage;

    if(!Memory->IsInitialized)
    {
        EditorState->Selection = CreateInitializedSelection();

        EditorState->ActionSlopFrames = 0;
        EditorState->NavigationSlopFrames = 0;
        EditorState->PixelBufferMode = PixelBufferModeTwentyFour;

        uint32 TextureWidth = 24;
        uint32 TextureHeight = 24;
        InitializePixelBufferAndPreviewTexture(Memory, EditorState, 
                                               RenderCommands,
                                               TextureWidth, TextureHeight);

        EditorState->ColorPalette = GetColorPalette(ColorPaletteTypeGameboyGreyscale);
        EditorState->SelectedColorIndex = 0;
        EditorState->PreviewMode = PreviewModeSprite;

        Memory->IsInitialized = true;
    }

    texture_editor_pixel_buffer *PixelBuffer = &EditorState->PixelBuffer;
    texture_editor_controller_input *GameController = &Input->GameController;

    uint8 NavigationPauseFrames = 8;

    if((Keyboard->L.EndedDown || 
        Keyboard->RightArrow.EndedDown ||
        GameController->Right.EndedDown) && 
        EditorState->NavigationSlopFrames == 0)
    {
        // TODO: (Ted)  Add the ability to move a rectangular selection here.
        IncrementPixel(&EditorState->Selection.SinglePixelLocation.X, 
                       PixelBuffer->Width);

        EditorState->NavigationSlopFrames = NavigationPauseFrames;
    }

    if((Keyboard->H.EndedDown || 
        Keyboard->LeftArrow.EndedDown ||
        GameController->Left.EndedDown) &&
        EditorState->NavigationSlopFrames == 0)
    {
        DecrementPixel(&EditorState->Selection.SinglePixelLocation.X, 
                       PixelBuffer->Width);

        EditorState->NavigationSlopFrames = NavigationPauseFrames;
    }

    if((Keyboard->K.EndedDown || 
        Keyboard->UpArrow.EndedDown ||
        GameController->Up.EndedDown) &&
        EditorState->NavigationSlopFrames == 0)
    {
        IncrementPixel(&EditorState->Selection.SinglePixelLocation.Y, 
                       PixelBuffer->Height);

        EditorState->NavigationSlopFrames = NavigationPauseFrames;
    }

    if((Keyboard->J.EndedDown || 
        Keyboard->DownArrow.EndedDown ||
        GameController->Down.EndedDown) &&
        EditorState->NavigationSlopFrames == 0)
    {
        DecrementPixel(&EditorState->Selection.SinglePixelLocation.Y, 
                       PixelBuffer->Height);

        EditorState->NavigationSlopFrames = NavigationPauseFrames;
    }

    if ((GameController->RightShoulder2.EndedDown ||
        (Keyboard->N.EndedDown)) &&
         EditorState->ActionSlopFrames == 0)
    {
        if (EditorState->ColorPalette.Type == ColorPaletteTypeGameboyGreyscale)
        {
            EditorState->ColorPalette = GetColorPalette(ColorPaletteTypeZoneColors);
        } else if (EditorState->ColorPalette.Type == ColorPaletteTypeZoneColors)
        {
            EditorState->ColorPalette = GetColorPalette(ColorPaletteTypeGameboyGreyscale);
        }

        EditorState->ActionSlopFrames = 10;
    }

    if (Keyboard->F4.EndedDown &&
        EditorState->ActionSlopFrames == 0)
    {
        // TODO: (Ted)  Have some visual indication of being in this mode.
        if (EditorState->Selection.Mode == SelectionModeSinglePixel)
        {
            pixel_location Default = {};
            Default.X = 0;
            Default.Y = 0;

            EditorState->Selection.RectangleStart = Default;
            EditorState->Selection.RectangleEnd = Default;

            EditorState->Selection.State = SelectionStateStartPixel;
            EditorState->Selection.Mode = SelectionModeRectangle;

        } else if (EditorState->Selection.Mode == SelectionModeRectangle)
        {
            EditorState->Selection.Mode = SelectionModeSinglePixel;
        }

        EditorState->ActionSlopFrames = 10;
    }

    if (Keyboard->Return.EndedDown &&
        EditorState->ActionSlopFrames == 0)
    {
        if (EditorState->Selection.Mode == SelectionModeRectangle)
        {
            if (EditorState->Selection.State == SelectionStateStartPixel)
            {
                EditorState->Selection.RectangleStart = EditorState->Selection.SinglePixelLocation;
                EditorState->Selection.State = SelectionStateEndPixel;
            } else if (EditorState->Selection.State == SelectionStateEndPixel)
            {
                EditorState->Selection.RectangleEnd = EditorState->Selection.SinglePixelLocation;
                EditorState->Selection.State = SelectionStateSelected;
            } else if (EditorState->Selection.State == SelectionStateSelected)
            {
                pixel_location Default = {};
                Default.X = 0;
                Default.Y = 0;

                EditorState->Selection.RectangleStart = Default;
                EditorState->Selection.RectangleEnd = Default;
                EditorState->Selection.State = SelectionStateStartPixel;
            }
        }

        EditorState->ActionSlopFrames = 10;
    }

    // NOTE: (Ted)  Operation is meant to flip the sprite on the x-axis.
    if (Keyboard->F6.EndedDown &&
        EditorState->ActionSlopFrames == 0)
    {
        for (uint32 RowIndex = 0;
             RowIndex < EditorState->PixelBuffer.Height;
             RowIndex++)
        {
            uint32 *Row = (uint32 *)PixelBuffer->Pixels + RowIndex*PixelBuffer->Width;

            uint32 *LeftSide = Row;
            uint32 *RightSide = Row + (PixelBuffer->Width - 1);

            while (LeftSide < RightSide)
            {
                uint32 TempLeft = *LeftSide;
                *LeftSide = *RightSide;
                *RightSide = TempLeft;
                RightSide--;
                LeftSide++;
            }
        }
        
        EditorState->ActionSlopFrames = 10;
    }

    // NOTE: (Ted) This flips the sprite over the y-axis.
    if (Keyboard->F7.EndedDown &&
        EditorState->ActionSlopFrames == 0)
    {
        uint32 HalfHeight = EditorState->PixelBuffer.Height/2;
        
        for (uint32 RowIndex = 0; RowIndex < HalfHeight; RowIndex++)
        {
            uint32 *PixelTop = (uint32 *)PixelBuffer->Pixels + RowIndex*PixelBuffer->Width;
            uint32 *PixelBottom = (uint32 *)PixelBuffer->Pixels + PixelBuffer->Height*PixelBuffer->Width - RowIndex*PixelBuffer->Width - PixelBuffer->Width;

            for (uint32 XIndex = 0; XIndex < PixelBuffer->Width; XIndex++)
            {
                uint32 TempTop = *PixelTop;
                *PixelTop = *PixelBottom;
                *PixelBottom = TempTop;
                PixelTop++;
                PixelBottom++;
            }
        }

        EditorState->ActionSlopFrames = 10;
    }

    if ((Keyboard->P.EndedDown || 
         GameController->RightShoulder.EndedDown) &&
         EditorState->NavigationSlopFrames == 0)
    {
        EditorState->SelectedColorIndex++;

        if (EditorState->SelectedColorIndex == EditorState->ColorPalette.ColorCount)
        {
            EditorState->SelectedColorIndex = 0;
        }

        EditorState->NavigationSlopFrames = NavigationPauseFrames;
    }

    if ((Keyboard->Y.EndedDown || 
         GameController->LeftShoulder.EndedDown) &&
         EditorState->NavigationSlopFrames == 0)
    {
        EditorState->SelectedColorIndex--;

        if (EditorState->SelectedColorIndex == -1)
        {
            EditorState->SelectedColorIndex = EditorState->ColorPalette.ColorCount - 1;
        }

        EditorState->NavigationSlopFrames = NavigationPauseFrames;
    }

    // NOTE: (Ted)  Don't modify Selected Color After This Point.
    uint32 SelectedColor = EditorState->ColorPalette.Colors[EditorState->SelectedColorIndex];
   
    uint8 ActionPauseFrames = 10;

    // NOTE: (Ted)  Double button actions must take precedence over single button
    //              actions. These tend to be the more destructive things you don't
    //              want to do too often.
    if (((Keyboard->A.EndedDown && Keyboard->C.EndedDown) ||
         (GameController->Select.EndedDown && GameController->A.EndedDown)) &&
          EditorState->ActionSlopFrames == 0)
    {
        FillPixelBufferWithSingleColor(&EditorState->PixelBuffer, 
                                       SelectedColor);

        EditorState->ActionSlopFrames = ActionPauseFrames;
    }
    
    if ((Keyboard->F1.EndedDown || 
         GameController->Select.EndedDown) &&
         EditorState->ActionSlopFrames == 0)
    {
        PlatformOpenFileDialog(Memory->TransientStorage);
        EditorState->ActionSlopFrames = ActionPauseFrames;
    }

    read_file_result *FileDialogResult = (read_file_result *)Memory->TransientStorage;
   
    if (FileDialogResult->ContentsSize > 0)
    {
        char *Scan = FileDialogResult->Filename; 
        texture_file_type FileType = TextureFileTypeUnknown;

        while(Scan++)
        {
            if(*Scan == '.')
            {
                char First = *(Scan + 1);
                char Second = *(Scan + 2);
                char Third = *(Scan + 3);
                char Fourth = *(Scan + 4);

                if (First == 'p' && Second == 'g' && Third == 't' && Fourth == 'n')
                {
                    FileType = TextureFileTypeWidthHeight;
                } else if (First == 'p' && Second == 'g' && Third == 't' && Fourth == '\0')
                {
                    FileType = TextureFileTypeNoHeader;
                } 

                break;
            }
        }

        if (FileType == TextureFileTypeNoHeader)
        {
            uint32 *PixelBufferSource = (uint32 *)FileDialogResult->Contents;
            uint32 *PixelBufferDest = (uint32 *)EditorState->PixelBuffer.Pixels;

            uint32 PixelCount = RenderCommands->PreviewTextureWidth*RenderCommands->PreviewTextureHeight;

            for (uint32 Index = 0; Index < PixelCount; Index++)
            {
                *PixelBufferDest++ = *PixelBufferSource++;
            }

        } else if (FileType == TextureFileTypeWidthHeight)
        {
            texture_file_header *FileHeader = (texture_file_header *)FileDialogResult->Contents;
            uint8 *BaseTextureData = (uint8 *)FileDialogResult->Contents;

            InitializePixelBufferAndPreviewTexture(Memory, EditorState, RenderCommands,
                                                   FileHeader->TextureWidth, FileHeader->TextureHeight);

            uint32 *PixelBufferSource = (uint32 *)(BaseTextureData + FileHeader->PixelBufferOffset);
            uint32 *PixelBufferDest = (uint32 *)EditorState->PixelBuffer.Pixels;

            uint32 PixelCount = FileHeader->TextureWidth*FileHeader->TextureHeight;

            for (uint32 Index = 0; Index < PixelCount; Index++)
            {
                *PixelBufferDest++ = *PixelBufferSource++;
            }
        } 

        PlatformFreeFileMemory(FileDialogResult->Contents);
        PlatformFreeFileMemory(FileDialogResult->Filename);

        FileDialogResult->ContentsSize = 0;
    }

    if ((Keyboard->A.EndedDown || 
         GameController->A.EndedDown) && 
         EditorState->ActionSlopFrames == 0)
    {
        FillWithColorUsingSelection(&EditorState->Selection, PixelBuffer, SelectedColor);
        EditorState->ActionSlopFrames = ActionPauseFrames;
    }

    if ((Keyboard->F.EndedDown || 
         GameController->B.EndedDown) &&
         EditorState->ActionSlopFrames == 0)
    {
        FillWithColorUsingSelection(&EditorState->Selection, PixelBuffer, 0x00000000);
        EditorState->ActionSlopFrames = ActionPauseFrames;
    }

    if ((Keyboard->F2.EndedDown || 
         GameController->Start.EndedDown) &&
         EditorState->ActionSlopFrames == 0)
    {
        texture_file_header FileHeader = {};
        FileHeader.TextureWidth = PixelBuffer->Width;
        FileHeader.TextureHeight = PixelBuffer->Height;
        FileHeader.PixelBufferOffset = sizeof(texture_file_header);
        uint32 PixelBufferSize = PixelBuffer->Width*PixelBuffer->Height*sizeof(uint32); 
        FileHeader.FileSize = sizeof(texture_file_header) + PixelBufferSize;

        uint8 *StartOfFileData = EditorState->StartOfSaveFileSerializedData;

        texture_file_header *FileHeaderDest = (texture_file_header *)StartOfFileData;
        *FileHeaderDest = FileHeader;

        uint32 *PixelSource = (uint32 *)EditorState->PixelBuffer.Pixels;
        uint32 *PixelDest = (uint32 *)(StartOfFileData + FileHeader.PixelBufferOffset);
        uint32 PixelCount = FileHeader.TextureWidth*FileHeader.TextureHeight;

        for (uint32 Index = 0; Index < PixelCount; Index++)
        {
            *PixelDest++ = *PixelSource++;
        }

        bool32 Written = PlatformWriteEntireFile(FileHeader.FileSize, (void *)StartOfFileData);

        EditorState->ActionSlopFrames = ActionPauseFrames;
    }
   
    if (Keyboard->F5.EndedDown &&
        EditorState->ActionSlopFrames == 0)
    {
        uint32 WidthHeight = 0;

        if (EditorState->PixelBufferMode == PixelBufferModeSixteen)
        {
            WidthHeight = 24;
            EditorState->PixelBufferMode = PixelBufferModeTwentyFour;
        }  else if (EditorState->PixelBufferMode == PixelBufferModeTwentyFour)
        {
            WidthHeight = 32;
            EditorState->PixelBufferMode = PixelBufferModeThirtyTwo;
        }
        else if (EditorState->PixelBufferMode == PixelBufferModeThirtyTwo)
        {
            WidthHeight = 16;
            EditorState->PixelBufferMode = PixelBufferModeSixteen;
        } 

        InitializePixelBufferAndPreviewTexture(Memory, EditorState, RenderCommands,
                                               WidthHeight, WidthHeight);

        EditorState->ActionSlopFrames = ActionPauseFrames;
    }

    if (Keyboard->M.EndedDown &&
        EditorState->ActionSlopFrames == 0)
    {
        if (EditorState->PreviewMode == PreviewModeSprite)
        {
            EditorState->PreviewMode = PreviewModeTile;
        } else if (EditorState->PreviewMode == PreviewModeTile)
        {
            EditorState->PreviewMode = PreviewModeSprite;
        }

        EditorState->ActionSlopFrames = ActionPauseFrames;
    }

    if (EditorState->NavigationSlopFrames > 0)
    {
        EditorState->NavigationSlopFrames--;
    }

    if (EditorState->ActionSlopFrames > 0)
    {
        EditorState->ActionSlopFrames--;
    }

    texture_editor_color BackgroundColor = { .973f, 0.369f, 0.533f, 1.0f };
    v2 BackgroundMin = { 0.0f, 0.0f };
    v2 BackgroundMax = V2((real32)RenderCommands->ViewportWidth, 
                          (real32)RenderCommands->ViewportHeight);

    DrawRectangle(RenderCommands, BackgroundMin, BackgroundMax, BackgroundColor);

    real32 PixelWidth = 16;
    real32 YStartingPoint = 10.0f; 
    real32 XStartingPoint = 10.0f;

    uint32 *Row = PixelBuffer->Pixels;

    for (uint32 Y = 0; Y < PixelBuffer->Height; Y++)
    {
        uint32 *Pixel = Row;

        for (uint32 X = 0; X < PixelBuffer->Width; X++)
        {
            real32 MinX = XStartingPoint + (real32)X*PixelWidth;
            real32 MaxX = MinX + PixelWidth;
            real32 MinY = YStartingPoint + (real32)Y*PixelWidth;
            real32 MaxY = MinY + PixelWidth;

            texture_editor_color PixelColor = ConvertUint32ToEditorColor(*Pixel);

            v2 PixelMin = { MinX, MinY };
            v2 PixelMax = { MaxX, MaxY };

            DrawRectangle(RenderCommands, PixelMin, PixelMax, PixelColor);

            Pixel++;
        }

        Row += PixelBuffer->Width;
    }

    // NOTE: (Ted)  Draw the color palette
    real32 PaletteItemPadding = 10.0f;
    real32 PaletteItemWidth = 2*PixelWidth;

    uint32 PaletteCount = EditorState->ColorPalette.ColorCount;
    real32 PaletteXStart = (real32)RenderCommands->ViewportWidth - PaletteCount*PaletteItemWidth - 
                           PaletteCount*PaletteItemPadding;
    real32 PaletteYStart = 10.0f;

    texture_editor_color BorderColor = { .301f, 0.314f, 0.314f, 0.5f };

    for (uint32 ColorIndex = 0; ColorIndex < PaletteCount; ColorIndex++)
    {
        uint32 Color = EditorState->ColorPalette.Colors[ColorIndex];
        texture_editor_color PaletteColor = ConvertUint32ToEditorColor(Color);

        if (ColorIndex == EditorState->SelectedColorIndex)
        {
            real32 BorderMinX = PaletteXStart + (real32)ColorIndex*PaletteItemWidth + ColorIndex*PaletteItemPadding;
            v2 Min = { BorderMinX, PaletteYStart };

            DrawBorderedSquare(RenderCommands, Min, 4.0f, PaletteItemWidth, 
                               BorderColor, PaletteColor);

        } else
        {
            real32 MinX = PaletteXStart + (real32)ColorIndex*PaletteItemWidth + ColorIndex*PaletteItemPadding;
            real32 MaxX = MinX + PaletteItemWidth;
            real32 MinY = PaletteYStart;
            real32 MaxY = MinY + PaletteItemWidth;

            v2 PaletteItemMin = { MinX, MinY };
            v2 PaletteItemMax = { MaxX, MaxY };

            DrawRectangle(RenderCommands, PaletteItemMin, 
                          PaletteItemMax, PaletteColor);
        }
    }
   
    if (EditorState->Selection.Mode == SelectionModeSinglePixel ||
       (EditorState->Selection.Mode == SelectionModeRectangle && EditorState->Selection.State == SelectionStateStartPixel))
    {
        real32 BorderMinX = XStartingPoint + (real32)(EditorState->Selection.SinglePixelLocation.X*PixelWidth);
        real32 BorderMinY = YStartingPoint + (real32)(EditorState->Selection.SinglePixelLocation.Y*PixelWidth);

        v2 Min = { BorderMinX, BorderMinY };

        texture_editor_color CursorColor = ConvertUint32ToEditorColor(SelectedColor);

        if (EditorState->Selection.Mode == SelectionModeSinglePixel)
        {
            DrawBorderedSquare(RenderCommands, Min, 2.0f, PixelWidth, 
                               BorderColor, CursorColor);
        } else
        {
            v2 Max = { Min.X + PixelWidth, Min.Y + PixelWidth };
            DrawRectangle(RenderCommands, Min, 
                          Max, BorderColor);
        }

    } else if (EditorState->Selection.Mode == SelectionModeRectangle)
    {
        if (EditorState->Selection.State == SelectionStateEndPixel)
        {
            DrawSelectionBetweenPixelLocations(RenderCommands, PixelWidth, XStartingPoint, YStartingPoint,
                                               &EditorState->Selection.RectangleStart, &EditorState->Selection.SinglePixelLocation,
                                               BorderColor);

        } else if (EditorState->Selection.State == SelectionStateSelected)
        {
            DrawSelectionBetweenPixelLocations(RenderCommands, PixelWidth, XStartingPoint, YStartingPoint,
                                               &EditorState->Selection.RectangleStart, &EditorState->Selection.RectangleEnd,
                                               BorderColor);
        }
    }

    // NOTE: (Ted)  These textures draw differently from the pixels and the color palette.
    //              They draw from the yMin up, not from the yMin down.
    //
    //              This is to replicate how the textures will be drawn in the game world
    //              itself.
    real32 TextureWidth = 4*RenderCommands->PreviewTextureWidth;
    real32 TextureHeight = 4*RenderCommands->PreviewTextureHeight;

    real32 TextureXMin = 10.0f;
    real32 TextureXMax = TextureXMin + TextureWidth;
    real32 TextureYMin = (real32)RenderCommands->ViewportHeight - TextureHeight - 10.0f;
    real32 TextureYMax = TextureYMin + TextureHeight;

    v2 TextureWidthHeight = { TextureWidth, TextureHeight };
    v2 TextureMin = { TextureXMin, TextureYMin };
    v2 TextureMax = { TextureXMax, TextureYMax };

    // NOTE: (Ted)  The texture editor inverts the y-axis to make sure the texture
    //              preview exactly matches how the texture will be rendered in-game.
    DrawTexturedRectangleYAxisInverted(RenderCommands, TextureMin, TextureMax, 0);

    if (EditorState->PreviewMode == PreviewModeTile)
    {
        TextureMin.X += TextureWidth; 
        TextureMax.X += TextureWidth; 
        DrawTexturedRectangleYAxisInverted(RenderCommands, TextureMin, TextureMax, 0);

        TextureMin.X = 10.0f;
        TextureMax.X = TextureMin.X + TextureWidth;

        TextureMin.Y -= TextureHeight;
        TextureMax.Y = TextureMin.Y + TextureHeight;

        DrawTexturedRectangleYAxisInverted(RenderCommands, TextureMin, TextureMax, 0);

        TextureMin.X += TextureWidth; 
        TextureMax.X += TextureWidth; 

        DrawTexturedRectangleYAxisInverted(RenderCommands, TextureMin, TextureMax, 0);
    }
}
