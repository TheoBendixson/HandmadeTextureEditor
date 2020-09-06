#include <simd/simd.h>

// TODO: (Ted)  Rename this. There is no need to reference the Mac platform. It's implicit.
typedef struct
{
    vector_float2 position;
    vector_float2 textureCoordinate;
    uint32_t textureID;
} TextureShaderVertex;

typedef enum TextureIndex
{
    TextureIndexBaseColor = 0,
} TextureIndex;
