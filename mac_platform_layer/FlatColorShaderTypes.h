/*
    Platform Game Mac Platform Layer
    2020 Ted Bendixson
    Mooselutions LLC

    Describes an API between C and Metal to draw flat shaded blocks of color
*/

#ifndef FlatColorShaderTypes_h
#define FlatColorShaderTypes_h

#include <simd/simd.h>

typedef struct
{
    // Positions in pixel space. A value of 100 indicates 100 pixels from the origin/center.
    vector_float2 position;

    // Color 
    vector_float4 color;
} FlatColorShaderVertex;

#endif /* FlatColorShaderTypes_h */
