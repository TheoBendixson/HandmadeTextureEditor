struct texture_editor_button_state 
{
    int HalfTransitionCount;
    bool32 EndedDown;
};

struct texture_editor_controller_input 
{
    bool32 IsAnalog;

    real32 StartX;
    real32 StartY;

    real32 MinX;
    real32 MinY;

    real32 MaxX;
    real32 MaxY;

    real32 EndX;
    real32 EndY;

    union 
    {
        texture_editor_button_state Buttons[14];
        struct 
        {
            texture_editor_button_state Up;
            texture_editor_button_state Down;
            texture_editor_button_state Left;
            texture_editor_button_state Right;
            texture_editor_button_state A;
            texture_editor_button_state B;
            texture_editor_button_state X;
            texture_editor_button_state Y;
            texture_editor_button_state LeftShoulder;
            texture_editor_button_state RightShoulder;
            texture_editor_button_state LeftShoulder2;
            texture_editor_button_state RightShoulder2;
            texture_editor_button_state Select;
            texture_editor_button_state Start;
        };
    };
};

struct texture_editor_key_state
{
    bool32 EndedDown;
};

struct texture_editor_keyboard_input
{
    union
    {
        texture_editor_key_state Keys[22];
        struct
        {
            texture_editor_key_state A;
            texture_editor_key_state C;
            texture_editor_key_state S;
            texture_editor_key_state F;
            texture_editor_key_state P;
            texture_editor_key_state Y;
            texture_editor_key_state H;
            texture_editor_key_state J;
            texture_editor_key_state K;
            texture_editor_key_state L;
            texture_editor_key_state M;
            texture_editor_key_state N;
            texture_editor_key_state LeftArrow;
            texture_editor_key_state RightArrow;
            texture_editor_key_state UpArrow;
            texture_editor_key_state DownArrow;
            texture_editor_key_state F1;
            texture_editor_key_state F2;
            texture_editor_key_state F4;
            texture_editor_key_state F5;
            texture_editor_key_state F6;
            texture_editor_key_state F7;
            texture_editor_key_state Return;
        };
    };
};

struct texture_editor_input 
{
    texture_editor_button_state MouseButtons[5];
    int32 MouseX, MouseY, MouseZ;
    real32 dtForFrame;
    texture_editor_controller_input GameController;
};
