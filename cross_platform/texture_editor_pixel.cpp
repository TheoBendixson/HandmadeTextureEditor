
void
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
