// A simple 2D Color shader to put a single color to various quads that will be drawn
// to the screen.

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

// Include header shared between this Metal shader code and C code executing Metal API commands
#import "CommonShaderTypes.h"
#import "FlatColorShaderTypes.h"

typedef struct
{
    // The [[position]] attribute qualifier of this member indicates this value is
    // the clip space position of the vertex when this structure is returned from
    // the vertex shader
    float4 position [[position]];

    float4 color;

} RasterizerData;

// Vertex Function
vertex RasterizerData
vertexShader(uint vertexID [[ vertex_id ]],
             constant FlatColorShaderVertex *vertexArray [[ buffer(VertexInputIndexVertices) ]],
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

    out.color = vertexArray[vertexID].color;

    return out;
}

// Fragment function
fragment float4 fragmentShader(RasterizerData in [[stage_in]])
{
    // Return the interpolated color.
    return in.color;
}
