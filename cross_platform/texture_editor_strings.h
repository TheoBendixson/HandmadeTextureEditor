/*
    texture_editor_strings.h

    2020 Ted Bendixson, Mooselutions LLC
 
 */

void CatStrings(size_t SourceACount, char *SourceA,
                size_t SourceBCount, char *SourceB,
                size_t DestCount, char *Dest)
{
    // TODO: Dest bounds checking!
    for(int Index = 0;
        Index < SourceACount;
        ++Index)
    {
        *Dest++ = *SourceA++;
    }

    for(int Index = 0;
        Index < SourceBCount;
        ++Index)
    {
        *Dest++ = *SourceB++;
    }

    *Dest++ = 0;
}

internal int
StringLength(char *String)
{
    int Count = 0;
    while(*String++)
    {
        ++Count;
    }
    return(Count);
}
