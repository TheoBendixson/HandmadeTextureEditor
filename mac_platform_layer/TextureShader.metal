
#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

#import "CommonShaderTypes.h"
#import "TextureShaderTypes.h"

typedef struct
{
    // The [[position]] attribute qualifier of this member indicates this value is
    // the clip space position of the vertex when this structure is returned from
    // the vertex shader
    float4 position [[position]];

    // Since this member does not have a special attribute qualifier, the rasterizer
    // will interpolate its value with values of other vertices making up the triangle
    // and pass that interpolated value to the fragment shader for each fragment in
    // that triangle.
    float2 textureCoordinate;

    uint32_t textureID;

} RasterizerData;

vertex RasterizerData
vertexShader(uint vertexID [[ vertex_id ]],
             constant TextureShaderVertex *vertexArray [[ buffer(VertexInputIndexVertices) ]],
             constant vector_uint2 *viewportSizePointer  [[ buffer(VertexInputIndexViewportSize) ]])

{

    RasterizerData out;

    // Index into the array of positions to get the current vertex.
    //   Positions are specified in pixel dimensions (i.e. a value of 100 is 100 pixels from
    //   the origin)
    float2 pixelSpacePosition = vertexArray[vertexID].position.xy;

    // Get the viewport size and cast to float.
    float2 viewportSize = float2(*viewportSizePointer);

    // To convert from positions in pixel space to positions in clip-space,
    //  divide the pixel coordinates by half the size of the viewport.
    // Z is set to 0.0 and w to 1.0 because this is 2D sample.
    out.position = vector_float4(0.0, 0.0, 0.0, 1.0);

    // NOTE: (Ted)  This would be the Open GL equivalent of mapping the projection matrix to
    //              the game's screen coordinates. For now, this is a 2D game.
    out.position.xy = (pixelSpacePosition / (viewportSize / 2.0)) - 1;

    out.textureCoordinate = vertexArray[vertexID].textureCoordinate;
    out.textureID = vertexArray[vertexID].textureID;

    return out;
}

// Fragment function
fragment float4
fragmentShader(RasterizerData in [[stage_in]],
               texture2d<half> colorTexture [[ texture(TextureIndexBaseColor) ]])
{
    constexpr sampler textureSampler (mag_filter::nearest, 
                                      min_filter::nearest);

    // Sample the texture to obtain a color
    const half4 colorSample = colorTexture.sample(textureSampler, in.textureCoordinate);

    // return the color of the texture
    return float4(colorSample);
}
