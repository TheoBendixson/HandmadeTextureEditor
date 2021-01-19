
void
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
