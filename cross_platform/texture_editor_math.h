/*
    texture_editor_math.h
    Platformer Texture Editor

    2020 Ted Bendixson Mooselutions, LLC
    All Rights Reserved
 */

union v2
{
    struct
    {
        real32 X, Y;
    };
    real32 E[2];
};

inline v2 V2(real32 X, real32 Y)
{
    v2 Result;

    Result.X = X;
    Result.Y = Y;

    return (Result);
}
